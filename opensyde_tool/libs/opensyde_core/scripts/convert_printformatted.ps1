# convert_printformatted.ps1
# Converts C_SclString PrintFormatted() calls to QString::asprintf()
#
# Usage:
#   .\convert_printformatted.ps1 -FilePath <path-to-cpp-file> [-DryRun]
#   .\convert_printformatted.ps1 -FilePath <path-to-cpp-file> -Apply
#
# Example:
#   .\convert_printformatted.ps1 -FilePath C:\path\to\file.cpp -DryRun
#   .\convert_printformatted.ps1 -FilePath C:\path\to\file.cpp -Apply

param(
    [Parameter(Mandatory=$true)]
    [string]$FilePath,

    [Parameter(Mandatory=$false)]
    [switch]$Apply,

    [Parameter(Mandatory=$false)]
    [switch]$DryRun = $true
)

# Validate file exists
if (-not (Test-Path $FilePath)) {
    Write-Error "File not found: $FilePath"
    exit 1
}

# Read file content
$content = Get-Content -Path $FilePath -Raw

# Store original content for comparison
$originalContent = $content

# Counter for changes
$changeCount = 0

# Pattern explanation:
# This pattern matches:
# 1. Variable name (alphanumeric, underscore, dots for member access)
# 2. .PrintFormatted(
# 3. Everything until the matching closing parenthesis
#
# Replace with:
# 1. Variable name
# 2. = QString::asprintf(
# 3. Arguments
#
# Example transformations:
# Before: c_Text.PrintFormatted("Value: %d", s32_Value);
# After:  c_Text = QString::asprintf("Value: %d", s32_Value);
#
# Before: this->c_Message.PrintFormatted("Error %d: %s", s32_Code, pc_Text);
# After:  this->c_Message = QString::asprintf("Error %d: %s", s32_Code, pc_Text);

# Use regex to find and replace PrintFormatted calls
# This handles simple cases - complex nested calls may need manual review
$pattern = '(\b[a-zA-Z_][a-zA-Z0-9_]*(?:->[a-zA-Z_][a-zA-Z0-9_]*|\.[a-zA-Z_][a-zA-Z0-9_]*)*)\.PrintFormatted\s*\('

$matches = [regex]::Matches($content, $pattern)

if ($matches.Count -gt 0) {
    Write-Host "`nFound $($matches.Count) PrintFormatted() calls in $FilePath" -ForegroundColor Cyan
    Write-Host "=" * 80

    # Process matches from end to beginning to preserve positions
    for ($i = $matches.Count - 1; $i -ge 0; $i--) {
        $match = $matches[$i]
        $varName = $match.Groups[1].Value
        $startPos = $match.Index

        # Find the matching closing parenthesis
        $openParens = 1
        $pos = $match.Index + $match.Length
        $argStart = $pos

        while ($openParens -gt 0 -and $pos -lt $content.Length) {
            $char = $content[$pos]
            if ($char -eq '(') { $openParens++ }
            if ($char -eq ')') { $openParens-- }
            $pos++
        }

        if ($openParens -eq 0) {
            # Extract the arguments (everything between the parentheses)
            $args = $content.Substring($argStart, $pos - $argStart - 1)

            # Build the replacement
            $oldText = $content.Substring($startPos, $pos - $startPos)
            $newText = "$varName = QString::asprintf($args)"

            # Show the change
            Write-Host "`nChange $($matches.Count - $i):" -ForegroundColor Yellow
            Write-Host "  Before: " -NoNewline
            Write-Host $oldText -ForegroundColor Red
            Write-Host "  After:  " -NoNewline
            Write-Host $newText -ForegroundColor Green

            # Replace in content
            $content = $content.Remove($startPos, $pos - $startPos).Insert($startPos, $newText)
            $changeCount++
        }
    }

    Write-Host "`n" + ("=" * 80)
    Write-Host "Total changes: $changeCount" -ForegroundColor Cyan

    if ($Apply -and $changeCount -gt 0) {
        # Backup original file
        $backupPath = $FilePath + ".bak"
        Copy-Item -Path $FilePath -Destination $backupPath -Force
        Write-Host "`nBackup created: $backupPath" -ForegroundColor Yellow

        # Write modified content
        Set-Content -Path $FilePath -Value $content -NoNewline
        Write-Host "File updated: $FilePath" -ForegroundColor Green
        Write-Host "`nIMPORTANT: Review the changes and compile to verify correctness!" -ForegroundColor Magenta
    } elseif ($changeCount -gt 0) {
        Write-Host "`nDRY RUN - No changes applied. Use -Apply to modify the file." -ForegroundColor Yellow
    }
} else {
    Write-Host "No PrintFormatted() calls found in $FilePath" -ForegroundColor Green
}

# Show summary
Write-Host "`n" + ("=" * 80)
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  File: $FilePath"
Write-Host "  PrintFormatted() calls found: $($matches.Count)"
Write-Host "  Changes prepared: $changeCount"
Write-Host "  Mode: $(if ($Apply) { 'APPLY' } else { 'DRY RUN' })"
Write-Host "=" * 80
