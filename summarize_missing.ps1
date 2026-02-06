$lines = Get-Content "C:\Users\tyler\Dev\repos\openSYDE\missing_namespace_files.txt"

$dirs = $lines | ForEach-Object {
    $path = $_ -replace [regex]::Escape("C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\"), ""
    $dir = Split-Path $path -Parent
    if ([string]::IsNullOrEmpty($dir)) { "." } else { $dir }
} | Group-Object | Sort-Object Count -Descending

Write-Host ""
Write-Host "Summary by Directory:"
Write-Host "===================="
$dirs | ForEach-Object {
    Write-Host ("{0,4} files in {1}" -f $_.Count, $_.Name)
}

Write-Host ""
Write-Host "Total: $($lines.Count) files"
