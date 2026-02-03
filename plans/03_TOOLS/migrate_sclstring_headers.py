#!/usr/bin/env python3
"""
C_SclString to QString Header Migration Script

This script migrates C_SclString member declarations to QString in header files.
After running this, build the project and fix any compiler errors.

Usage:
    python migrate_sclstring_headers.py --file <header_path> --dry-run
    python migrate_sclstring_headers.py --file <header_path> --apply
    python migrate_sclstring_headers.py --class C_OscTspApplication --dry-run
"""

import os
import re
import argparse
from pathlib import Path
from dataclasses import dataclass
from typing import List, Optional

# Root directory for opensyde_core
OPENSYDE_CORE_PATH = Path(__file__).parent.parent / "opensyde_tool" / "libs" / "opensyde_core"

@dataclass
class HeaderChange:
    """Represents a change to be made in a header file."""
    line_number: int
    old_line: str
    new_line: str
    member_name: str


def migrate_header_file(header_path: Path, dry_run: bool = True) -> List[HeaderChange]:
    """Migrate all C_SclString declarations in a header file to QString."""
    changes = []

    if not header_path.exists():
        print(f"Error: File not found: {header_path}")
        return changes

    try:
        with open(header_path, 'r', encoding='utf-8', errors='replace') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading {header_path}: {e}")
        return changes

    new_lines = []
    needs_qstring_include = False
    has_qstring_include = False

    for line_num, line in enumerate(lines, 1):
        new_line = line

        # Check for existing QString include
        if '#include <QString>' in line or '#include <QtCore/QString>' in line:
            has_qstring_include = True

        # Pattern 1: Simple member declaration
        # C_SclString member_name;
        pattern1 = r'^(\s*)(?:stw::scl::)?C_SclString\s+(\w+)\s*;(.*)$'
        match1 = re.match(pattern1, line)
        if match1:
            indent = match1.group(1)
            member = match1.group(2)
            comment = match1.group(3)
            new_line = f'{indent}QString {member};{comment}\n'
            changes.append(HeaderChange(line_num, line.rstrip(), new_line.rstrip(), member))
            needs_qstring_include = True

        # Pattern 2: Member with initialization
        # C_SclString member_name = "value";
        pattern2 = r'^(\s*)(?:stw::scl::)?C_SclString\s+(\w+)\s*=\s*"([^"]*)"(.*)$'
        match2 = re.match(pattern2, line)
        if match2 and not match1:
            indent = match2.group(1)
            member = match2.group(2)
            value = match2.group(3)
            rest = match2.group(4)
            new_line = f'{indent}QString {member} = "{value}"{rest}\n'
            changes.append(HeaderChange(line_num, line.rstrip(), new_line.rstrip(), member))
            needs_qstring_include = True

        # Pattern 3: Static const member
        # static const C_SclString member_name;
        pattern3 = r'^(\s*)(static\s+)?(const\s+)?(?:stw::scl::)?C_SclString\s+(\w+)\s*;(.*)$'
        match3 = re.match(pattern3, line)
        if match3 and not match1 and not match2:
            indent = match3.group(1)
            static_kw = match3.group(2) or ''
            const_kw = match3.group(3) or ''
            member = match3.group(4)
            comment = match3.group(5)
            new_line = f'{indent}{static_kw}{const_kw}QString {member};{comment}\n'
            changes.append(HeaderChange(line_num, line.rstrip(), new_line.rstrip(), member))
            needs_qstring_include = True

        # Pattern 4: Vector of C_SclString -> QStringList
        # std::vector<C_SclString> member_name;
        pattern4 = r'^(\s*)std::vector\s*<\s*(?:stw::scl::)?C_SclString\s*>\s+(\w+)\s*;(.*)$'
        match4 = re.match(pattern4, line)
        if match4:
            indent = match4.group(1)
            member = match4.group(2)
            comment = match4.group(3)
            new_line = f'{indent}QStringList {member};{comment}\n'
            changes.append(HeaderChange(line_num, line.rstrip(), new_line.rstrip(), member))
            needs_qstring_include = True

        # Pattern 5: Function parameter (leave as-is but note it)
        # This pattern is intentionally NOT replacing function parameters

        new_lines.append(new_line)

    if changes and not dry_run:
        # Add QString include if needed
        if needs_qstring_include and not has_qstring_include:
            # Find a good place to add the include
            insert_pos = 0
            for i, line in enumerate(new_lines):
                if '#include' in line:
                    insert_pos = i + 1
                elif line.strip() and not line.startswith('//') and not line.startswith('/*') and insert_pos > 0:
                    break

            if insert_pos > 0:
                new_lines.insert(insert_pos, '#include <QString>\n')
                print(f"  Added #include <QString> at line {insert_pos + 1}")

        with open(header_path, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)

    return changes


def find_header_for_class(class_name: str) -> Optional[Path]:
    """Find the header file that defines a class."""
    # Common patterns for class definition
    patterns = [
        rf'class\s+(?:\w+\s+)?{class_name}\s*[:\{{]',
        rf'struct\s+(?:\w+\s+)?{class_name}\s*[:\{{]',
    ]

    for ext in ['*.hpp', '*.h']:
        for header in OPENSYDE_CORE_PATH.rglob(ext):
            try:
                with open(header, 'r', encoding='utf-8', errors='replace') as f:
                    content = f.read()
                for pattern in patterns:
                    if re.search(pattern, content):
                        return header
            except Exception:
                continue

    return None


def main():
    parser = argparse.ArgumentParser(description="Migrate C_SclString to QString in header files")
    parser.add_argument('--dry-run', action='store_true', default=True,
                        help="Show what would change without making changes (default)")
    parser.add_argument('--apply', action='store_true',
                        help="Apply changes to files")
    parser.add_argument('--file', type=str,
                        help="Header file to migrate")
    parser.add_argument('--class', dest='class_name', type=str,
                        help="Find and migrate header for specified class")

    args = parser.parse_args()
    dry_run = not args.apply

    # Determine which header to migrate
    header_path = None
    if args.file:
        header_path = Path(args.file)
        if not header_path.is_absolute():
            header_path = OPENSYDE_CORE_PATH / args.file
    elif args.class_name:
        header_path = find_header_for_class(args.class_name)
        if not header_path:
            print(f"Could not find header file for class: {args.class_name}")
            return 1
    else:
        print("Please specify --file or --class")
        return 1

    print(f"Header file: {header_path}")
    print(f"Mode: {'DRY RUN' if dry_run else 'APPLYING CHANGES'}")
    print("")

    # Migrate the header
    changes = migrate_header_file(header_path, dry_run)

    if not changes:
        print("No C_SclString members found to migrate.")
        return 0

    print(f"Found {len(changes)} members to migrate:\n")
    for c in changes:
        print(f"  Line {c.line_number}: {c.member_name}")
        print(f"    - {c.old_line}")
        print(f"    + {c.new_line}")
        print("")

    if dry_run:
        print("This was a DRY RUN. Use --apply to make changes.")
    else:
        print("Changes applied. Now build the project and fix any compiler errors.")

    return 0


if __name__ == "__main__":
    exit(main())
