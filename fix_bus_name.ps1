# Script to fix multiple types of C_SclString to QString migration issues
$srcPath = 'c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src'

Get-ChildItem -Path $srcPath -Recurse -Filter *.cpp | ForEach-Object {
    $file = $_.FullName
    $content = Get-Content $file -Raw
    $modified = $false
    $originalContent = $content
    
    # Fix 1: Remove .c_str() from pc_Bus->c_Name (already QString)
    if ($content -match 'pc_Bus->c_Name\.c_str\(\)') {
        $content = $content -replace 'pc_Bus->c_Name\.c_str\(\)', 'pc_Bus->c_Name'
        $modified = $true
    }
    
    # Fix 2: Remove .c_str() from pc_OscBus->c_Name (already QString) 
    if ($content -match '->c_Name\.c_str\(\)') {
        # This is more general - be careful with patterns that might affect C_SclString members
        # Only fix patterns related to bus names which are now QString
    }
    
    # Fix 3: Replace .IsEmpty() with .isEmpty() for QString members (like c_Comment on bus)
    if ($content -match 'pc_Bus->c_Comment\.IsEmpty\(\)') {
        $content = $content -replace 'pc_Bus->c_Comment\.IsEmpty\(\)', 'pc_Bus->c_Comment.isEmpty()'
        $modified = $true
    }
    
    # Fix 4: Remove .c_str() from pc_Bus->c_Comment (already QString)
    if ($content -match 'pc_Bus->c_Comment\.c_str\(\)') {
        $content = $content -replace 'pc_Bus->c_Comment\.c_str\(\)', 'pc_Bus->c_Comment'
        $modified = $true
    }
    
    # Fix 5: Remove .toStdString().c_str() when passing to h_CheckValidFilePath (use QString directly)
    if ($content -match '\.toStdString\(\)\.c_str\(\)') {
        $content = $content -replace '\.toStdString\(\)\.c_str\(\)', ''
        $modified = $true
    }
    
    if ($modified) {
        Set-Content $file -Value $content -NoNewline
        Write-Host "Fixed: $file"
    }
}

Write-Host "Done!"
