# C_OscUtils Migration Plan

## Overview
This document outlines the migration plan for the C_OscUtils utility class from C_SclString to QString. This is the first step in Phase 2-PREP of the Qt Native Replacement initiative.

## Migration Approach
We'll implement a dual API approach where both C_SclString and QString versions coexist during the transition period:
1. Add QString overloads for all public methods (dual API)
2. Mark C_SclString versions as deprecated
3. Convert internal implementations to use QString
4. Update callers incrementally over time

## Key Considerations
1. **Index Differences**: C_SclString uses 1-based indexing, QString uses 0-based indexing
2. **String Conversion**: Need to handle conversions properly between C_SclString and QString
3. **Template Specialization**: Handle template declarations carefully
4. **Header Inclusions**: Update all #include statements appropriately

## Migration Steps

### Step 1: Update Header File (C_OscUtils.hpp)
- Add QString overloads for all public methods
- Mark C_SclString versions as deprecated
- Update includes to include Qt headers

### Step 2: Update Implementation File (C_OscUtils.cpp)
- Convert internal implementations to use QString
- Update all string operations to Qt equivalents
- Handle indexing differences properly

### Step 3: Create Test Cases
- Create unit tests for each migrated method
- Test boundary conditions
- Test special character handling
- Test string index conversions

### Step 4: Prepare Automated Tools
- Create scripts to identify and convert patterns
- Test the conversion scripts

## Methods to Migrate

### String Operations
| C_SclString Method | QString Equivalent |
|--------------------|-----------------|
| `.c_str()` | `.toStdString().c_str()` or `.toLatin1().constData()` |
| `.AsStdString()` | `.toStdString()` |
| `.Length()` | `.length()` or `.size()` |
| `.SubString(pos, len)` | `.mid(pos-1, len)` (note: 0-based vs 1-based) |
| `.Pos(search)` | `.indexOf(search) + 1` (note: 0-based vs 1-based) |
| `.UpperCase()` | `.toUpper()` |
| `.LowerCase()` | `.toLower()` |
| `.Trim()` | `.trimmed()` |
| `.StringPrintFormatted()` | `QString::asprintf()` or `QString::arg()` |
| `.IntToStr()` | `QString::number()` |
| `.StringToInt()` | `.toInt()` |
| `.Delete(pos, len)` | `.remove(pos-1, len)` |
| `.Insert(str, pos)` | `.insert(pos-1, str)` |

## Testing Strategy
1. **Unit Tests**: Create for each migrated method
2. **String Index Tests**: Specific tests for 1-based → 0-based conversion
3. **Special Character Tests**: Verify special characters handled correctly
4. **Performance Tests**: Ensure no regression in critical paths

## Risk Mitigation
1. **Index Off-by-One Errors**: Comprehensive unit tests, manual audit of SubString/Pos calls
2. **Unicode/Encoding Issues**: Test with special characters, international text
3. **API Breaking Changes**: Update all consumers in same changeset
