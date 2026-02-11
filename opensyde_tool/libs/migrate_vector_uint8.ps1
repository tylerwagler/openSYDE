# Batch migration script: std::vector<uint8_t> -> QByteArray
# For opensyde_core library

$ErrorActionPreference = "Stop"

Write-Host "Migrating std::vector<uint8_t> to QByteArray in opensyde_core..." -ForegroundColor Cyan

# Find all .cpp and .hpp files containing vector.*uint8_t
$files = Get-ChildItem -Path "." -Recurse -Include @("*.cpp", "*.hpp") | Where-Object {
    (Select-String -Path $_.FullName -Pattern "vector.*uint8_t" -Quiet)
}

Write-Host "Found $($files.Count) files to process" -ForegroundColor Yellow

$processedCount = 0
foreach ($file in $files) {
    $processedCount++
    $relativePath = $file.FullName.Replace((Get-Location).Path, ".")
    Write-Host "[$processedCount/$($files.Count)] Processing: $relativePath" -ForegroundColor Gray

    # Read content
    $content = Get-Content -Path $file.FullName -Raw

    # Replace std::vector<uint8_t> with QByteArray
    $newContent = $content -replace 'std::vector<uint8_t>', 'QByteArray'

    # Also handle cases with spaces
    $newContent = $newContent -replace 'std::vector\s*<\s*uint8_t\s*>', 'QByteArray'
    $newContent = $newContent -replace 'vector\s*<\s*uint8_t\s*>', 'QByteArray'

    # Write back
    Set-Content -Path $file.FullName -Value $newContent -NoNewline
}

Write-Host "`nMigration complete! Processed $processedCount files." -ForegroundColor Green
Write-Host "Note: You may need to add QByteArray includes and fix casting issues." -ForegroundColor Yellow
