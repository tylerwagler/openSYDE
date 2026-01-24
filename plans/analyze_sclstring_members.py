#!/usr/bin/env python3
"""
Analyze C_SclString Member Declarations for QString Migration

This script scans all header files to identify C_SclString member variables
and categorizes them by their likely purpose (paths, names, comments, etc.)
to help prioritize QString migration.

Usage:
    python analyze_sclstring_members.py
"""

import os
import re
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Dict, Set
from collections import defaultdict

# Root directory for opensyde_core
OPENSYDE_CORE_PATH = Path(__file__).parent.parent / "opensyde_tool" / "libs" / "opensyde_core"

@dataclass
class MemberDeclaration:
    """Represents a C_SclString member variable declaration."""
    file_path: Path
    line_number: int
    class_name: str
    member_name: str
    full_line: str
    category: str = ""  # Will be set based on naming patterns

@dataclass
class ClassInfo:
    """Information about a class with C_SclString members."""
    name: str
    file_path: Path
    members: List[MemberDeclaration] = field(default_factory=list)


def categorize_member(member_name: str) -> str:
    """Categorize a member based on its name."""
    name_lower = member_name.lower()

    # Path-related
    if any(x in name_lower for x in ['path', 'file', 'folder', 'dir', 'location', 'url']):
        return "PATH"

    # Name-related
    if any(x in name_lower for x in ['name', 'title', 'label', 'id', 'identifier']):
        return "NAME"

    # Comment/Description
    if any(x in name_lower for x in ['comment', 'description', 'desc', 'info', 'detail', 'note']):
        return "COMMENT"

    # Unit/Format
    if any(x in name_lower for x in ['unit', 'format', 'suffix', 'prefix']):
        return "FORMAT"

    # Version/Date
    if any(x in name_lower for x in ['version', 'date', 'time', 'timestamp']):
        return "VERSION"

    # Error/Message
    if any(x in name_lower for x in ['error', 'message', 'msg', 'text', 'string']):
        return "MESSAGE"

    # Value/Content
    if any(x in name_lower for x in ['value', 'content', 'data']):
        return "VALUE"

    return "OTHER"


def find_header_files(root_path: Path) -> List[Path]:
    """Find all header files."""
    files = []
    for ext in ['*.hpp', '*.h']:
        files.extend(root_path.rglob(ext))
    return sorted(files)


def extract_class_name(lines: List[str], line_idx: int) -> str:
    """Try to find the class name that contains this member."""
    # Look backwards for class/struct declaration
    for i in range(line_idx - 1, max(0, line_idx - 100), -1):
        line = lines[i].strip()
        # Match class or struct declaration
        match = re.search(r'(?:class|struct)\s+(?:\w+\s+)?(\w+)', line)
        if match:
            return match.group(1)
    return "UNKNOWN"


def scan_file_for_members(file_path: Path) -> List[MemberDeclaration]:
    """Scan a header file for C_SclString member declarations."""
    members = []

    try:
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()
            lines = content.split('\n')
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return members

    # Pattern to match C_SclString member declarations
    # Matches: C_SclString c_MemberName; or C_SclString c_MemberName = ...;
    # Also matches with stw::scl:: prefix
    patterns = [
        # Standard member declaration
        r'(?:stw::scl::)?C_SclString\s+(\w+)\s*[;=]',
        # std::vector of C_SclString
        r'std::vector\s*<\s*(?:stw::scl::)?C_SclString\s*>\s+(\w+)\s*[;=]',
        # QList of C_SclString
        r'QList\s*<\s*(?:stw::scl::)?C_SclString\s*>\s+(\w+)\s*[;=]',
    ]

    for line_num, line in enumerate(lines, 1):
        stripped = line.strip()

        # Skip comments
        if stripped.startswith('//') or stripped.startswith('/*'):
            continue

        # Skip function parameters and local variables (look for member-like context)
        # Members typically have visibility modifiers or are in class scope

        for pattern in patterns:
            matches = re.finditer(pattern, line)
            for match in matches:
                member_name = match.group(1)

                # Skip if it looks like a function parameter (has parentheses nearby)
                if '(' in line and ')' in line:
                    # Check if the match is between parentheses
                    paren_start = line.find('(')
                    paren_end = line.rfind(')')
                    match_pos = match.start()
                    if paren_start < match_pos < paren_end:
                        continue

                class_name = extract_class_name(lines, line_num - 1)
                category = categorize_member(member_name)

                members.append(MemberDeclaration(
                    file_path=file_path,
                    line_number=line_num,
                    class_name=class_name,
                    member_name=member_name,
                    full_line=stripped,
                    category=category
                ))

    return members


def generate_report(all_members: List[MemberDeclaration]) -> str:
    """Generate analysis report."""
    report = []
    report.append("=" * 80)
    report.append("C_SclString Member Analysis Report")
    report.append("=" * 80)
    report.append("")

    # Summary by category
    by_category = defaultdict(list)
    for m in all_members:
        by_category[m.category].append(m)

    report.append("SUMMARY BY CATEGORY:")
    report.append("-" * 40)
    report.append("")

    category_priority = {
        "PATH": "HIGH - File paths should use QString for Qt file operations",
        "NAME": "HIGH - Names displayed in UI should use QString",
        "COMMENT": "MEDIUM - Comments/descriptions for UI display",
        "MESSAGE": "MEDIUM - Error messages and text for logging",
        "FORMAT": "LOW - Units and format strings",
        "VERSION": "LOW - Version strings",
        "VALUE": "LOW - Generic values",
        "OTHER": "REVIEW - Needs manual review",
    }

    for category in ["PATH", "NAME", "COMMENT", "MESSAGE", "FORMAT", "VERSION", "VALUE", "OTHER"]:
        members = by_category.get(category, [])
        if members:
            priority = category_priority.get(category, "UNKNOWN")
            report.append(f"  {category}: {len(members)} members - {priority}")

    report.append("")
    report.append(f"  TOTAL: {len(all_members)} C_SclString members found")
    report.append("")

    # Summary by class
    by_class = defaultdict(list)
    for m in all_members:
        by_class[m.class_name].append(m)

    report.append("=" * 80)
    report.append("SUMMARY BY CLASS (sorted by member count):")
    report.append("=" * 80)
    report.append("")

    for class_name, members in sorted(by_class.items(), key=lambda x: -len(x[1])):
        categories = set(m.category for m in members)
        cat_str = ", ".join(sorted(categories))
        report.append(f"  {class_name}: {len(members)} members ({cat_str})")

    report.append("")

    # Detailed listing by category (HIGH priority first)
    for category in ["PATH", "NAME", "COMMENT", "MESSAGE", "FORMAT", "VERSION", "VALUE", "OTHER"]:
        members = by_category.get(category, [])
        if members:
            report.append("=" * 80)
            report.append(f"CATEGORY: {category} ({len(members)} members)")
            report.append("=" * 80)
            report.append("")

            # Group by file
            by_file = defaultdict(list)
            for m in members:
                by_file[m.file_path].append(m)

            for file_path, file_members in sorted(by_file.items()):
                rel_path = file_path.relative_to(OPENSYDE_CORE_PATH) if file_path.is_relative_to(OPENSYDE_CORE_PATH) else file_path
                report.append(f"  {rel_path}:")
                for m in file_members:
                    report.append(f"    Line {m.line_number}: {m.class_name}::{m.member_name}")
                report.append("")

    # Migration recommendations
    report.append("=" * 80)
    report.append("MIGRATION RECOMMENDATIONS:")
    report.append("=" * 80)
    report.append("")
    report.append("1. HIGH PRIORITY - Migrate these first:")
    report.append("   - PATH members: Used with QFile, QDir, QFileInfo")
    report.append("   - NAME members: Often displayed in Qt UI")
    report.append("")
    report.append("2. MEDIUM PRIORITY:")
    report.append("   - COMMENT/MESSAGE: Used in logging and UI")
    report.append("")
    report.append("3. LOW PRIORITY:")
    report.append("   - FORMAT/VERSION/VALUE: Less frequently used with Qt APIs")
    report.append("")
    report.append("4. Classes with many members should be migrated together")
    report.append("   to avoid mixed C_SclString/QString usage in the same class.")
    report.append("")

    return "\n".join(report)


def main():
    print(f"Scanning: {OPENSYDE_CORE_PATH}")
    print("")

    # Find all header files
    files = find_header_files(OPENSYDE_CORE_PATH)
    print(f"Found {len(files)} header files")
    print("")

    # Scan for C_SclString members
    all_members = []
    for file_path in files:
        members = scan_file_for_members(file_path)
        all_members.extend(members)

    print(f"Found {len(all_members)} C_SclString member declarations")
    print("")

    # Generate report
    report = generate_report(all_members)

    # Write report to file
    report_path = Path(__file__).parent / "sclstring_member_analysis.txt"
    with open(report_path, 'w', encoding='utf-8') as f:
        f.write(report)
    print(f"Report written to: {report_path}")
    print("")

    # Print summary to console
    print(report[:4000])
    if len(report) > 4000:
        print(f"\n... (report truncated, see full report in file)")

    return 0


if __name__ == "__main__":
    exit(main())
