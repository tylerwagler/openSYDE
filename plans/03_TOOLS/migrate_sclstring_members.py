#!/usr/bin/env python3
"""
C_SclString to QString Member Migration Script

This script migrates specific class members from C_SclString to QString.
It updates both the header file declarations and all usages in cpp files.

Usage:
    python migrate_sclstring_members.py --class C_OscTspApplication --dry-run
    python migrate_sclstring_members.py --class C_OscTspApplication --apply
    python migrate_sclstring_members.py --all-high-priority --dry-run
"""

import os
import re
import argparse
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Dict, Set, Tuple, Optional
from collections import defaultdict

# Root directory for opensyde_core
OPENSYDE_CORE_PATH = Path(__file__).parent.parent / "opensyde_tool" / "libs" / "opensyde_core"

# Directories to exclude from migration
EXCLUDE_DIRS = {'scl'}  # Don't modify the C_SclString class itself

@dataclass
class MemberMigration:
    """Represents a member to migrate."""
    class_name: str
    member_name: str
    header_file: Path
    category: str

@dataclass
class FileChange:
    """Represents a change to be made in a file."""
    file_path: Path
    line_number: int
    old_line: str
    new_line: str
    change_type: str  # 'declaration', 'usage', 'method_call'

# HIGH PRIORITY members to migrate (from analysis)
HIGH_PRIORITY_MEMBERS = [
    # PATH members
    MemberMigration("C_OscDataDealerNvmSafe", "mc_ParameterSetFilePath",
                    OPENSYDE_CORE_PATH / "data_dealer" / "C_OscDataDealerNvmSafe.hpp", "PATH"),
    MemberMigration("C_OscXceBase", "mhc_CERTIFICATES_FOLDER",
                    OPENSYDE_CORE_PATH / "exports" / "x_certificates_package_generation" / "C_OscXceBase.hpp", "PATH"),
    MemberMigration("C_OscXceBase", "mhc_UPDATE_PACKAGE_PARAMETERS_FOLDER",
                    OPENSYDE_CORE_PATH / "exports" / "x_certificates_package_generation" / "C_OscXceBase.hpp", "PATH"),
    MemberMigration("C_OscXceManifest", "c_CertificatesPath",
                    OPENSYDE_CORE_PATH / "exports" / "x_certificates_package_generation" / "C_OscXceManifest.hpp", "PATH"),
    MemberMigration("C_OscXceManifestFiler", "hc_FILE_NAME",
                    OPENSYDE_CORE_PATH / "exports" / "x_certificates_package_generation" / "C_OscXceManifestFiler.hpp", "PATH"),
    MemberMigration("C_OscXceUpdatePackageParameters", "c_AuthenticationKeyPath",
                    OPENSYDE_CORE_PATH / "exports" / "x_certificates_package_generation" / "C_OscXceUpdatePackageParameters.hpp", "PATH"),
    MemberMigration("C_OscXcoBase", "hc_XCFG_SYSDEF_FOLDER",
                    OPENSYDE_CORE_PATH / "exports" / "x_config_generation" / "C_OscXcoBase.hpp", "PATH"),
    MemberMigration("C_OscXcoBase", "hc_INI_DEV_FOLDER",
                    OPENSYDE_CORE_PATH / "exports" / "x_config_generation" / "C_OscXcoBase.hpp", "PATH"),
    MemberMigration("C_OscXcoManifestFiler", "hc_FILE_NAME",
                    OPENSYDE_CORE_PATH / "exports" / "x_config_generation" / "C_OscXcoManifestFiler.hpp", "PATH"),
    MemberMigration("C_OscCanOpenEdsFileInfoBlock", "c_FileName",
                    OPENSYDE_CORE_PATH / "imports" / "C_OscCanOpenEdsFileInfoBlock.hpp", "PATH"),
    MemberMigration("C_OscTargetSupportPackage", "c_TemplatePath",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackage.hpp", "PATH"),
    MemberMigration("C_OscTspApplication", "c_CodeGeneratorPath",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "PATH"),
    MemberMigration("C_OscTspApplication", "c_ProjectFolder",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "PATH"),
    MemberMigration("C_OscTspApplication", "c_GeneratePath",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "PATH"),
    MemberMigration("C_OscTspApplication", "c_ResultPaths",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "PATH"),
    MemberMigration("C_OscTargetSupportPackageV2", "c_TemplatePath",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "PATH"),
    MemberMigration("C_OscTargetSupportPackageV2", "c_HalcDefPath",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "PATH"),
    MemberMigration("C_OscComMessageLoggerFileBase", "mc_FilePath",
                    OPENSYDE_CORE_PATH / "protocol_drivers" / "communication" / "C_OscComMessageLoggerFileBase.hpp", "PATH"),
    MemberMigration("C_OscSuSequencesNodeFileStates", "c_FileName",
                    OPENSYDE_CORE_PATH / "protocol_drivers" / "system_update" / "C_OscSuSequencesNodeStates.hpp", "PATH"),

    # NAME members
    MemberMigration("C_OscParamSetInterpretedList", "c_Name",
                    OPENSYDE_CORE_PATH / "data_dealer" / "paramset" / "C_OscParamSetInterpretedList.hpp", "NAME"),
    MemberMigration("C_OscXcoManifest", "c_NodeName",
                    OPENSYDE_CORE_PATH / "exports" / "x_config_generation" / "C_OscXcoManifest.hpp", "NAME"),
    MemberMigration("C_OscCanOpenEdsDeviceInfoBlock", "c_VendorName",
                    OPENSYDE_CORE_PATH / "imports" / "C_OscCanOpenEdsDeviceInfoBlock.hpp", "NAME"),
    MemberMigration("C_OscCanOpenEdsDeviceInfoBlock", "c_ProductName",
                    OPENSYDE_CORE_PATH / "imports" / "C_OscCanOpenEdsDeviceInfoBlock.hpp", "NAME"),
    MemberMigration("C_OscCanOpenObjectData", "c_Name",
                    OPENSYDE_CORE_PATH / "imports" / "C_OscCanOpenObjectDictionary.hpp", "NAME"),
    MemberMigration("C_OscTargetSupportPackage", "c_DeviceName",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackage.hpp", "NAME"),
    MemberMigration("C_OscTspApplication", "c_Name",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "NAME"),
    MemberMigration("C_OscTspApplication", "c_IdeCall",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "NAME"),
    MemberMigration("C_OscTargetSupportPackageV2", "c_DeviceName",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "NAME"),
    MemberMigration("C_BroadcastGetDeviceInfoResults", "c_DeviceName",
                    OPENSYDE_CORE_PATH / "protocol_drivers" / "C_OscProtocolDriverOsyTpIp.hpp", "NAME"),
    MemberMigration("C_OscComMessageLoggerFileBase", "mc_ProtocolName",
                    OPENSYDE_CORE_PATH / "protocol_drivers" / "communication" / "C_OscComMessageLoggerFileBase.hpp", "NAME"),

    # COMMENT members
    MemberMigration("C_OscExportCanOpenConciseEntry", "c_Comment",
                    OPENSYDE_CORE_PATH / "exports" / "code_generation" / "C_OscExportCanOpenConfig.hpp", "COMMENT"),
    MemberMigration("C_OscCanOpenEdsFileInfoBlock", "c_Description",
                    OPENSYDE_CORE_PATH / "imports" / "C_OscCanOpenEdsFileInfoBlock.hpp", "COMMENT"),
    MemberMigration("C_OscTargetSupportPackage", "c_Comment",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackage.hpp", "COMMENT"),
    MemberMigration("C_OscTspApplication", "c_Comment",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "COMMENT"),
    MemberMigration("C_OscTargetSupportPackageV2", "c_Comment",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "COMMENT"),
    MemberMigration("C_OscTargetSupportPackageV2", "c_HalcComment",
                    OPENSYDE_CORE_PATH / "project" / "system" / "target_support_package" / "C_OscTargetSupportPackageV2.hpp", "COMMENT"),
]


def find_all_cpp_files(root_path: Path) -> List[Path]:
    """Find all C++ source files, excluding certain directories."""
    files = []
    for ext in ['*.cpp', '*.hpp', '*.h']:
        for f in root_path.rglob(ext):
            if not any(excl in f.parts for excl in EXCLUDE_DIRS):
                files.append(f)
    return sorted(files)


def migrate_declaration_in_header(header_path: Path, member_name: str, dry_run: bool = True) -> List[FileChange]:
    """Migrate a C_SclString declaration to QString in a header file."""
    changes = []

    if not header_path.exists():
        print(f"  Warning: Header not found: {header_path}")
        return changes

    try:
        with open(header_path, 'r', encoding='utf-8', errors='replace') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"  Error reading {header_path}: {e}")
        return changes

    modified = False
    new_lines = []

    for line_num, line in enumerate(lines, 1):
        new_line = line

        # Pattern 1: C_SclString member_name;
        pattern1 = rf'(\s*)(?:stw::scl::)?C_SclString\s+({re.escape(member_name)})\s*;'
        match1 = re.search(pattern1, line)
        if match1:
            indent = match1.group(1)
            new_line = f'{indent}QString {member_name};\n'
            changes.append(FileChange(header_path, line_num, line.rstrip(), new_line.rstrip(), 'declaration'))
            modified = True

        # Pattern 2: C_SclString member_name = "...";
        pattern2 = rf'(\s*)(?:stw::scl::)?C_SclString\s+({re.escape(member_name)})\s*=\s*"([^"]*)"'
        match2 = re.search(pattern2, line)
        if match2:
            indent = match2.group(1)
            value = match2.group(3)
            new_line = f'{indent}QString {member_name} = "{value}";\n'
            changes.append(FileChange(header_path, line_num, line.rstrip(), new_line.rstrip(), 'declaration'))
            modified = True

        # Pattern 3: static const C_SclString member_name;
        pattern3 = rf'(\s*)(static\s+)?(?:const\s+)?(?:stw::scl::)?C_SclString\s+({re.escape(member_name)})\s*;'
        match3 = re.search(pattern3, line)
        if match3 and not match1:  # Don't match if already matched pattern1
            indent = match3.group(1)
            static_kw = match3.group(2) or ''
            new_line = f'{indent}{static_kw}const QString {member_name};\n'
            changes.append(FileChange(header_path, line_num, line.rstrip(), new_line.rstrip(), 'declaration'))
            modified = True

        # Pattern 4: std::vector<C_SclString> member_name;
        pattern4 = rf'(\s*)std::vector\s*<\s*(?:stw::scl::)?C_SclString\s*>\s+({re.escape(member_name)})\s*;'
        match4 = re.search(pattern4, line)
        if match4:
            indent = match4.group(1)
            new_line = f'{indent}QStringList {member_name};\n'
            changes.append(FileChange(header_path, line_num, line.rstrip(), new_line.rstrip(), 'declaration'))
            modified = True

        new_lines.append(new_line)

    if modified and not dry_run:
        with open(header_path, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)

    return changes


def find_usages_and_migrate(member: MemberMigration, all_files: List[Path], dry_run: bool = True) -> List[FileChange]:
    """Find all usages of a member and prepare migrations."""
    changes = []

    # C_SclString method to QString method mappings
    method_mappings = {
        '.Length()': '.length()',
        '.IsEmpty()': '.isEmpty()',
        '.c_str()': '.toUtf8().constData()',
        '.LowerCase()': '.toLower()',
        '.UpperCase()': '.toUpper()',
        '.Trim()': '.trimmed()',
        '.TrimLeft()': '.trimmed()',
        '.TrimRight()': '.trimmed()',
        '.AnsiCompare(': '.compare(',
        '.Pos(': '.indexOf(',
        '.LastPos(': '.lastIndexOf(',
    }

    member_name = member.member_name

    for file_path in all_files:
        try:
            with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read()
                lines = content.split('\n')
        except Exception:
            continue

        # Skip if member not referenced in file
        if member_name not in content:
            continue

        modified = False
        new_lines = []

        for line_num, line in enumerate(lines, 1):
            new_line = line

            # Check for method calls on this member
            for old_method, new_method in method_mappings.items():
                # Pattern: member_name.OldMethod
                pattern = rf'({re.escape(member_name)}){re.escape(old_method)}'
                if re.search(pattern, line):
                    new_line = re.sub(pattern, rf'\1{new_method}', new_line)
                    if new_line != line:
                        changes.append(FileChange(file_path, line_num, line, new_line, 'method_call'))
                        modified = True

            # Handle .ToQString() - remove it since the member is now QString
            pattern_toqstring = rf'({re.escape(member_name)})\.ToQString\(\)'
            if re.search(pattern_toqstring, line):
                new_line = re.sub(pattern_toqstring, r'\1', new_line)
                if new_line != line:
                    changes.append(FileChange(file_path, line_num, line, new_line, 'method_call'))
                    modified = True

            # Handle FromQString - for assignments like member = C_SclString::FromQString(qstr)
            # After migration, this becomes member = qstr
            pattern_fromqstring = rf'{re.escape(member_name)}\s*=\s*C_SclString::FromQString\(([^)]+)\)'
            match = re.search(pattern_fromqstring, line)
            if match:
                qstr_value = match.group(1)
                new_line = re.sub(pattern_fromqstring, f'{member_name} = {qstr_value}', new_line)
                if new_line != line:
                    changes.append(FileChange(file_path, line_num, line, new_line, 'usage'))
                    modified = True

            new_lines.append(new_line)

        if modified and not dry_run:
            with open(file_path, 'w', encoding='utf-8', newline='') as f:
                f.write('\n'.join(new_lines))

    return changes


def migrate_member(member: MemberMigration, all_files: List[Path], dry_run: bool = True) -> List[FileChange]:
    """Migrate a single member from C_SclString to QString."""
    all_changes = []

    print(f"\n  Migrating {member.class_name}::{member.member_name} ({member.category})")

    # Step 1: Update the header declaration
    header_changes = migrate_declaration_in_header(member.header_file, member.member_name, dry_run)
    all_changes.extend(header_changes)
    if header_changes:
        print(f"    Header: {len(header_changes)} changes")

    # Step 2: Find and update all usages
    usage_changes = find_usages_and_migrate(member, all_files, dry_run)
    all_changes.extend(usage_changes)
    if usage_changes:
        print(f"    Usages: {len(usage_changes)} changes")

    return all_changes


def generate_migration_report(changes: List[FileChange]) -> str:
    """Generate a detailed migration report."""
    report = []
    report.append("=" * 80)
    report.append("C_SclString to QString Migration Report")
    report.append("=" * 80)
    report.append("")

    # Group by file
    by_file = defaultdict(list)
    for c in changes:
        by_file[c.file_path].append(c)

    report.append(f"Total changes: {len(changes)}")
    report.append(f"Files affected: {len(by_file)}")
    report.append("")

    # Count by type
    by_type = defaultdict(int)
    for c in changes:
        by_type[c.change_type] += 1

    report.append("Changes by type:")
    for change_type, count in sorted(by_type.items()):
        report.append(f"  {change_type}: {count}")
    report.append("")

    report.append("=" * 80)
    report.append("DETAILED CHANGES BY FILE:")
    report.append("=" * 80)

    for file_path, file_changes in sorted(by_file.items()):
        rel_path = file_path.relative_to(OPENSYDE_CORE_PATH) if file_path.is_relative_to(OPENSYDE_CORE_PATH) else file_path
        report.append(f"\n{rel_path}:")
        for c in sorted(file_changes, key=lambda x: x.line_number):
            report.append(f"  Line {c.line_number} [{c.change_type}]:")
            report.append(f"    - {c.old_line.strip()}")
            report.append(f"    + {c.new_line.strip()}")

    return "\n".join(report)


def main():
    parser = argparse.ArgumentParser(description="Migrate C_SclString members to QString")
    parser.add_argument('--dry-run', action='store_true', default=True,
                        help="Show what would change without making changes (default)")
    parser.add_argument('--apply', action='store_true',
                        help="Apply changes to files")
    parser.add_argument('--class', dest='class_name', type=str,
                        help="Migrate only members of specified class")
    parser.add_argument('--member', type=str,
                        help="Migrate only specified member (requires --class)")
    parser.add_argument('--all-high-priority', action='store_true',
                        help="Migrate all HIGH priority members (PATH, NAME, COMMENT)")
    parser.add_argument('--category', type=str, choices=['PATH', 'NAME', 'COMMENT', 'MESSAGE'],
                        help="Migrate only members of specified category")

    args = parser.parse_args()
    dry_run = not args.apply

    print(f"Scanning: {OPENSYDE_CORE_PATH}")
    print(f"Mode: {'DRY RUN' if dry_run else 'APPLYING CHANGES'}")
    print("")

    # Find all C++ files
    all_files = find_all_cpp_files(OPENSYDE_CORE_PATH)
    print(f"Found {len(all_files)} C++ files")

    # Determine which members to migrate
    members_to_migrate = []

    if args.class_name:
        members_to_migrate = [m for m in HIGH_PRIORITY_MEMBERS if m.class_name == args.class_name]
        if args.member:
            members_to_migrate = [m for m in members_to_migrate if m.member_name == args.member]
    elif args.category:
        members_to_migrate = [m for m in HIGH_PRIORITY_MEMBERS if m.category == args.category]
    elif args.all_high_priority:
        members_to_migrate = HIGH_PRIORITY_MEMBERS
    else:
        print("Please specify --class, --category, or --all-high-priority")
        print("\nAvailable classes with HIGH priority members:")
        classes = sorted(set(m.class_name for m in HIGH_PRIORITY_MEMBERS))
        for cls in classes:
            members = [m for m in HIGH_PRIORITY_MEMBERS if m.class_name == cls]
            print(f"  {cls}: {len(members)} members")
        return 1

    if not members_to_migrate:
        print("No members found matching criteria")
        return 1

    print(f"\nMigrating {len(members_to_migrate)} members...")

    all_changes = []
    for member in members_to_migrate:
        changes = migrate_member(member, all_files, dry_run)
        all_changes.extend(changes)

    # Generate report
    report = generate_migration_report(all_changes)

    report_path = Path(__file__).parent / "sclstring_migration_report.txt"
    with open(report_path, 'w', encoding='utf-8') as f:
        f.write(report)
    print(f"\nReport written to: {report_path}")

    # Print summary
    print(f"\n{'=' * 60}")
    print(f"Summary: {len(all_changes)} total changes")
    if dry_run:
        print("This was a DRY RUN. Use --apply to make changes.")
    else:
        print("Changes have been applied.")

    return 0


if __name__ == "__main__":
    exit(main())
