# Fix QByteArray pointer access patterns
# Replace &array[0] with reinterpret_cast<uint8_t*>(array.data())

$ErrorActionPreference = "Stop"

Write-Host "Fixing QByteArray pointer access patterns..." -ForegroundColor Cyan

# Find files that might need fixing
$files = Get-ChildItem -Path "opensyde_core" -Recurse -Include @("*.cpp") | Where-Object {
    (Select-String -Path $_.FullName -Pattern "&.*\[0\]" -Quiet)
}

Write-Host "Found $($files.Count) files with potential [0] patterns" -ForegroundColor Yellow

$processedCount = 0
foreach ($file in $files) {
    $processedCount++
    $relativePath = $file.FullName.Replace((Get-Location).Path, ".")
    Write-Host "[$processedCount/$($files.Count)] Checking: $relativePath" -ForegroundColor Gray

    # Read content
    $content = Get-Content -Path $file.FullName -Raw
    $originalContent = $content

    # Pattern 1: &this->memberArray[0] -> reinterpret_cast<uint8_t*>(this->memberArray.data())
    $content = $content -replace '&(this->(?:mc|aau8)_\w+)\[0\]', 'reinterpret_cast<uint8_t*>($1.data())'

    # Pattern 2: &localVar[0] -> reinterpret_cast<uint8_t*>(localVar.data())
    $content = $content -replace '&(c_\w+)\[0\]', 'reinterpret_cast<uint8_t*>($1.data())'

    # Pattern 3: &this->memberArray[index] -> reinterpret_cast<uint8_t*>(&this->memberArray.data()[index])
    $content = $content -replace '&(this->(?:mc|aau8)_\w+)\[(\w+)\]', 'reinterpret_cast<uint8_t*>(&$1.data()[$2])'

    # Pattern 4: &localVar[index] -> reinterpret_cast<uint8_t*>(&localVar.data()[index])
    $content = $content -replace '&(c_\w+)\[(\w+)\]', 'reinterpret_cast<uint8_t*>(&$1.data()[$2])'

    # Only write if changed
    if ($content -ne $originalContent) {
        Set-Content -Path $file.FullName -Value $content -NoNewline
        Write-Host "  -> Fixed" -ForegroundColor Green
    }
}

Write-Host "`nProcessing complete! Checked $processedCount files." -ForegroundColor Green
