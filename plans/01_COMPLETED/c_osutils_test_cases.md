# Test Cases for C_OscUtils Migration

## Overview
This document contains test cases for the C_OscUtils methods that will be migrated from C_SclString to QString during Phase 2-PREP.

## Test Cases by Method

### h_CheckValidCeName
- Test empty string
- Test valid C identifier (a-z, A-Z, 0-9, _)
- Test invalid characters
- Test string starting with digit
- Test string exceeding max length
- Test automatic adaptation mode

### h_IsFloat64NearlyEqual
- Test equal floats
- Test nearly equal floats
- Test significantly different floats
- Test with zero values

### h_IsFloat32NearlyEqual
- Test equal floats
- Test nearly equal floats
- Test significantly different floats
- Test with zero values

### h_CreateFolderRecursively
- Test creating existing folder
- Test creating nested folders
- Test with invalid path

### h_NiceifyStringForFileName
- Test special characters replacement
- Test "." and ".." handling
- Test blank string handling
- Test valid characters preserved

### h_NiceifyStringForCeComment
- Test non-printable character replacement
- Test "*" followed by "/" handling
- Test "\" at end handling
- Test valid characters preserved

### h_CheckValidFileName
- Test valid file name
- Test invalid file name (contains special characters that would be replaced)

### h_CheckValidFilePath
- Test valid file path
- Test invalid file path (contains invalid characters)
- Test path with slashes
- Test empty path

### h_IsScalingActive
- Test factor != 1.0
- Test offset != 0.0
- Test both factor = 1.0 and offset = 0.0

### h_GetValueScaled
- Test basic scaling
- Test with range adaptation

### h_GetValueUnscaled
- Test basic inverse scaling

### h_PosSerialNumberToString
- Test STW POS format (pre-2020)
- Test STW POS format (post-2020)

### h_FsnSerialNumberToString
- Test STW POS format
- Test invalid length
- Test valid format

### h_FileToString
- Test reading existing file
- Test reading non-existent file

### h_RangeCheckFloat
- Test value within range
- Test value exceeding range

### h_LoadString
- Test valid string index
- Test invalid string index

### h_CopyFile
- Test copying existing file
- Test copying to non-existent directory
- Test copying non-existent file

### h_GetCommandLineAsString
- Test command line with multiple arguments
- Test command line with single argument

### h_MakeIndependentOfDbProjectPath
- Test path with Data Block project variable
- Test relative path handling

### h_ResolvePlaceholderVariables
- Test path with project variables
- Test path with Data Block project variable

### h_ResolveProjIndependentPlaceholderVariables
- Test path with binary variable
- Test path with user name variable
- Test path with computer name variable

### h_ConcatPathIfNecessary
- Test relative path concatenation
- Test absolute path handling

### h_GetUniqueName
- Test name uniqueness
- Test name with number suffix

### h_GetNumberAtStringEnd
- Test string with number at end
- Test string without number at end

### h_GetSystemUserName
- Test getting user name from environment

### h_GetSystemMachineName
- Test getting machine name from environment

### h_HandleSystemMessages
- Test processing system messages

### h_ChangeFileExtension
- Test changing file extension
- Test adding extension when none exists
