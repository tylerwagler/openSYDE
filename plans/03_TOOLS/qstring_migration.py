#!/usr/bin/env python3
"""
QString Migration Script for openSYDE
Automates conversion from C_SclString patterns to QString patterns.

Usage:
    python qstring_migration.py --dry-run    # Show what would change (default)
    python qstring_migration.py --apply      # Apply changes
    python qstring_migration.py --report     # Generate detailed report only
"""

import os
import re
import argparse
from pathlib import Path
from dataclasses import dataclass
from typing import List, Tuple, Dict
from collections import defaultdict

# Root directory for opensyde_core
OPENSYDE_CORE_PATH = Path(__file__).parent.parent / "opensyde_tool" / "libs" / "opensyde_core"

@dataclass
class Replacement:
    """Represents a single replacement to be made."""
    file_path: Path
    line_number: int
    old_text: str
    new_text: str
    pattern_name: str
    safe: bool = True  # Whether this is safe to auto-apply

@dataclass
class PatternDef:
    """Defines a search/replace pattern."""
    name: str
    search: str  # Regex pattern
    replace: str  # Replacement (can use \1, \2 for groups)
    safe: bool = True  # Safe to auto-apply without context checking
    description: str = ""

# =============================================================================
# SAFE PATTERNS - These can be applied without context checking
# =============================================================================
SAFE_PATTERNS = [
    PatternDef(
        name="AnsiCompare",
        search=r'\.AnsiCompare\(',
        replace='.compare(',
        description="C_SclString::AnsiCompare() -> QString::compare()"
    ),
    PatternDef(
        name="LowerCase",
        search=r'\.LowerCase\(\)',
        replace='.toLower()',
        description="C_SclString::LowerCase() -> QString::toLower()"
    ),
    PatternDef(
        name="UpperCase",
        search=r'\.UpperCase\(\)',
        replace='.toUpper()',
        description="C_SclString::UpperCase() -> QString::toUpper()"
    ),
    PatternDef(
        name="Trim",
        search=r'\.Trim\(\)',
        replace='.trimmed()',
        description="C_SclString::Trim() -> QString::trimmed()"
    ),
    PatternDef(
        name="TrimLeft",
        search=r'\.TrimLeft\(\)',
        replace='.trimmed()',  # QString doesn't have separate left/right trim
        description="C_SclString::TrimLeft() -> QString::trimmed()"
    ),
    PatternDef(
        name="TrimRight",
        search=r'\.TrimRight\(\)',
        replace='.trimmed()',
        description="C_SclString::TrimRight() -> QString::trimmed()"
    ),
    PatternDef(
        name="IsEmpty",
        search=r'\.IsEmpty\(\)',
        replace='.isEmpty()',
        description="C_SclString::IsEmpty() -> QString::isEmpty()"
    ),
    PatternDef(
        name="SubString",
        search=r'\.SubString\(',
        replace='.mid(',
        description="C_SclString::SubString() -> QString::mid() [NOTE: index is 0-based in QString, 1-based in C_SclString]",
        safe=False  # Index difference makes this unsafe for auto-apply
    ),
    PatternDef(
        name="Pos",
        search=r'\.Pos\(',
        replace='.indexOf(',
        description="C_SclString::Pos() -> QString::indexOf() [NOTE: return value differs]",
        safe=False
    ),
    PatternDef(
        name="Delete",
        search=r'\.Delete\(',
        replace='.remove(',
        description="C_SclString::Delete() -> QString::remove() [NOTE: index is 0-based in QString]",
        safe=False
    ),
    PatternDef(
        name="Insert",
        search=r'\.Insert\(',
        replace='.insert(',
        description="C_SclString::Insert() -> QString::insert() [NOTE: index is 0-based in QString]",
        safe=False
    ),
    PatternDef(
        name="LastPos",
        search=r'\.LastPos\(',
        replace='.lastIndexOf(',
        description="C_SclString::LastPos() -> QString::lastIndexOf()"
    ),
]

# =============================================================================
# CONTEXT-SENSITIVE PATTERNS - Need to check variable type
# =============================================================================
CONTEXT_PATTERNS = [
    PatternDef(
        name="Length",
        search=r'\.Length\(\)',
        replace='.length()',
        safe=False,
        description="C_SclString::Length() -> QString::length() [Only for QString variables]"
    ),
    PatternDef(
        name="c_str",
        search=r'\.c_str\(\)',
        replace='.toUtf8().constData()',
        safe=False,
        description="C_SclString::c_str() -> QString::toUtf8().constData() [Only for QString variables]"
    ),
    PatternDef(
        name="ToInt",
        search=r'\.ToInt\(\)',
        replace='.toInt()',
        safe=False,
        description="C_SclString::ToInt() -> QString::toInt() [Only for QString variables]"
    ),
    PatternDef(
        name="ToLong",
        search=r'\.ToLong\(\)',
        replace='.toLongLong()',
        safe=False,
        description="C_SclString::ToLong() -> QString::toLongLong() [Only for QString variables]"
    ),
]

# =============================================================================
# SPECIAL PATTERNS - Require more complex transformations
# =============================================================================
SPECIAL_PATTERNS = [
    PatternDef(
        name="PrintFormatted_ErrorText",
        search=r'c_ErrorText\.PrintFormatted\(',
        replace='c_ErrorText = QString::asprintf(',
        safe=True,
        description="c_ErrorText.PrintFormatted() -> c_ErrorText = QString::asprintf()"
    ),
    PatternDef(
        name="PrintFormatted_Text",
        search=r'c_Text\.PrintFormatted\(',
        replace='c_Text = QString::asprintf(',
        safe=True,
        description="c_Text.PrintFormatted() -> c_Text = QString::asprintf()"
    ),
    PatternDef(
        name="PrintFormatted_Error",
        search=r'c_Error\.PrintFormatted\(',
        replace='c_Error = QString::asprintf(',
        safe=True,
        description="c_Error.PrintFormatted() -> c_Error = QString::asprintf()"
    ),
    PatternDef(
        name="PrintFormatted_Line",
        search=r'c_Line\.PrintFormatted\(',
        replace='c_Line = QString::asprintf(',
        safe=True,
        description="c_Line.PrintFormatted() -> c_Line = QString::asprintf()"
    ),
    PatternDef(
        name="PrintFormatted_Generic",
        search=r'(\w+)\.PrintFormatted\(',
        replace=r'\1 = QString::asprintf(',
        safe=False,  # Generic pattern needs review
        description="var.PrintFormatted() -> var = QString::asprintf()"
    ),
    PatternDef(
        name="IntToStr",
        search=r'C_SclString::IntToStr\(([^)]+)\)',
        replace=r'QString::number(\1)',
        safe=True,
        description="C_SclString::IntToStr(n) -> QString::number(n)"
    ),
    PatternDef(
        name="IntToHex",
        search=r'C_SclString::IntToHex\(([^,]+),\s*(\d+)\)',
        replace=r'QString::number(\1, 16).rightJustified(\2, \'0\')',
        safe=False,  # Complex transformation, needs review
        description="C_SclString::IntToHex(n, width) -> QString::number(n, 16).rightJustified(width, '0')"
    ),
]

# C_SclStringList patterns (when the list itself has been changed to use QString internally)
STRINGLIST_PATTERNS = [
    PatternDef(
        name="SclStringList_Append_lower",
        search=r'(\w+)\.append\(',  # QStringList uses lowercase
        replace=r'\1.Add(',  # But if it's C_SclStringList, use Add
        safe=False,
        description="Check if using C_SclStringList.Add() vs QStringList.append()"
    ),
    PatternDef(
        name="SclStringList_GetCount",
        search=r'\.GetCount\(\)',
        replace='.count()',
        safe=False,
        description="C_SclStringList::GetCount() -> QStringList::count() [Only if converted to QStringList]"
    ),
]


def find_cpp_files(root_path: Path) -> List[Path]:
    """Find all .cpp and .hpp files in the directory tree, excluding scl/ directory."""
    files = []
    # Directories to exclude - these contain the C_SclString class definition itself
    exclude_dirs = {'scl'}

    for ext in ['*.cpp', '*.hpp', '*.h']:
        for f in root_path.rglob(ext):
            # Check if any parent directory is in the exclude list
            if not any(excl in f.parts for excl in exclude_dirs):
                files.append(f)
    return sorted(files)


def scan_file_for_patterns(file_path: Path, patterns: List[PatternDef]) -> List[Replacement]:
    """Scan a single file for all patterns and return list of replacements."""
    replacements = []

    try:
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return replacements

    for line_num, line in enumerate(lines, 1):
        for pattern in patterns:
            matches = list(re.finditer(pattern.search, line))
            for match in matches:
                old_text = match.group(0)
                # Apply replacement
                new_text = re.sub(pattern.search, pattern.replace, old_text)

                replacements.append(Replacement(
                    file_path=file_path,
                    line_number=line_num,
                    old_text=old_text,
                    new_text=new_text,
                    pattern_name=pattern.name,
                    safe=pattern.safe
                ))

    return replacements


def apply_replacements_to_file(file_path: Path, replacements: List[Replacement]) -> bool:
    """Apply all replacements to a single file."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()

        # Group replacements by pattern to apply all at once
        for pattern in SAFE_PATTERNS + SPECIAL_PATTERNS:
            if pattern.safe:
                content = re.sub(pattern.search, pattern.replace, content)

        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)

        return True
    except Exception as e:
        print(f"Error applying to {file_path}: {e}")
        return False


def generate_report(all_replacements: List[Replacement]) -> str:
    """Generate a detailed report of all findings."""
    report = []
    report.append("=" * 80)
    report.append("QString Migration Report")
    report.append("=" * 80)
    report.append("")

    # Summary by pattern
    by_pattern = defaultdict(list)
    for r in all_replacements:
        by_pattern[r.pattern_name].append(r)

    report.append("SUMMARY BY PATTERN:")
    report.append("-" * 40)
    safe_count = 0
    unsafe_count = 0
    for pattern_name, reps in sorted(by_pattern.items(), key=lambda x: -len(x[1])):
        safe_marker = "[SAFE]" if reps[0].safe else "[REVIEW]"
        report.append(f"  {pattern_name}: {len(reps)} occurrences {safe_marker}")
        if reps[0].safe:
            safe_count += len(reps)
        else:
            unsafe_count += len(reps)

    report.append("")
    report.append(f"Total SAFE replacements (can auto-apply): {safe_count}")
    report.append(f"Total REVIEW replacements (need manual check): {unsafe_count}")
    report.append("")

    # Summary by file
    by_file = defaultdict(list)
    for r in all_replacements:
        by_file[r.file_path].append(r)

    report.append("SUMMARY BY FILE:")
    report.append("-" * 40)
    for file_path, reps in sorted(by_file.items(), key=lambda x: -len(x[1])):
        safe_in_file = sum(1 for r in reps if r.safe)
        unsafe_in_file = len(reps) - safe_in_file
        rel_path = file_path.relative_to(OPENSYDE_CORE_PATH) if file_path.is_relative_to(OPENSYDE_CORE_PATH) else file_path
        report.append(f"  {rel_path}: {len(reps)} ({safe_in_file} safe, {unsafe_in_file} review)")

    report.append("")
    report.append("=" * 80)
    report.append("DETAILED CHANGES (Safe patterns only):")
    report.append("=" * 80)

    for file_path, reps in sorted(by_file.items()):
        safe_reps = [r for r in reps if r.safe]
        if safe_reps:
            rel_path = file_path.relative_to(OPENSYDE_CORE_PATH) if file_path.is_relative_to(OPENSYDE_CORE_PATH) else file_path
            report.append(f"\n{rel_path}:")
            for r in safe_reps:
                report.append(f"  Line {r.line_number}: {r.old_text} -> {r.new_text}")

    report.append("")
    report.append("=" * 80)
    report.append("ITEMS REQUIRING REVIEW (Unsafe patterns):")
    report.append("=" * 80)

    for file_path, reps in sorted(by_file.items()):
        unsafe_reps = [r for r in reps if not r.safe]
        if unsafe_reps:
            rel_path = file_path.relative_to(OPENSYDE_CORE_PATH) if file_path.is_relative_to(OPENSYDE_CORE_PATH) else file_path
            report.append(f"\n{rel_path}:")
            for r in unsafe_reps:
                report.append(f"  Line {r.line_number} [{r.pattern_name}]: {r.old_text} -> {r.new_text}")

    return "\n".join(report)


def main():
    parser = argparse.ArgumentParser(description="QString Migration Script for openSYDE")
    parser.add_argument('--dry-run', action='store_true', default=True,
                        help="Show what would change without making changes (default)")
    parser.add_argument('--apply', action='store_true',
                        help="Apply safe changes to files")
    parser.add_argument('--report', action='store_true',
                        help="Generate detailed report to file")
    parser.add_argument('--path', type=str, default=None,
                        help="Override path to scan (default: opensyde_core)")

    args = parser.parse_args()

    # Determine scan path
    scan_path = Path(args.path) if args.path else OPENSYDE_CORE_PATH

    if not scan_path.exists():
        print(f"Error: Path does not exist: {scan_path}")
        return 1

    print(f"Scanning: {scan_path}")
    print("")

    # Find all files
    files = find_cpp_files(scan_path)
    print(f"Found {len(files)} C++ files")
    print("")

    # Scan for all patterns
    all_patterns = SAFE_PATTERNS + SPECIAL_PATTERNS + CONTEXT_PATTERNS
    all_replacements = []

    for file_path in files:
        replacements = scan_file_for_patterns(file_path, all_patterns)
        all_replacements.extend(replacements)

    print(f"Found {len(all_replacements)} total replacements")
    safe_replacements = [r for r in all_replacements if r.safe]
    print(f"  - {len(safe_replacements)} safe (can auto-apply)")
    print(f"  - {len(all_replacements) - len(safe_replacements)} need review")
    print("")

    # Generate report
    report = generate_report(all_replacements)

    if args.report or args.dry_run:
        report_path = Path(__file__).parent / "qstring_migration_report.txt"
        with open(report_path, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"Report written to: {report_path}")
        print("")

    # Print summary to console
    print(report[:3000])  # Print first part of report
    if len(report) > 3000:
        print(f"\n... (report truncated, see full report in file)")

    if args.apply:
        print("")
        print("=" * 80)
        print("APPLYING SAFE CHANGES...")
        print("=" * 80)

        # Group by file
        by_file = defaultdict(list)
        for r in safe_replacements:
            by_file[r.file_path].append(r)

        modified_count = 0
        for file_path, reps in by_file.items():
            if apply_replacements_to_file(file_path, reps):
                modified_count += 1
                rel_path = file_path.relative_to(OPENSYDE_CORE_PATH) if file_path.is_relative_to(OPENSYDE_CORE_PATH) else file_path
                print(f"  Modified: {rel_path} ({len(reps)} changes)")

        print("")
        print(f"Modified {modified_count} files")
    else:
        print("")
        print("Dry run complete. Use --apply to make changes.")

    return 0


if __name__ == "__main__":
    exit(main())
