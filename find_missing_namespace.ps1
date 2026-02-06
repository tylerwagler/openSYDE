$files = Get-ChildItem -Path "C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src" -Filter "*.cpp" -Recurse | Where-Object {
    $_.FullName -notmatch "can_monitor|syde_flash" -and 
    (Select-String -Path $_.FullName -Pattern "/\* -- Used Namespaces" -Quiet)
}

$missing = @()
foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw
    $nsIndex = $content.IndexOf("/* -- Used Namespaces")
    if ($nsIndex -ge 0) {
        $next500 = $content.Substring($nsIndex, [Math]::Min(500, $content.Length - $nsIndex))
        if ($next500 -notmatch "using namespace stw::opensyde_gui_logic") {
            $missing += $file.FullName
        }
    }
}

$missing | Out-File "C:\Users\tyler\Dev\repos\openSYDE\missing_namespace_files.txt" -Encoding utf8
Write-Host "Found $($missing.Count) files missing opensyde_gui_logic namespace"
