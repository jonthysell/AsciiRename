param([string]$TargetOutputPackageName, [string]$TargetOutputDirectory)

Write-Host "Create package..."

Compress-Archive -Path "$TargetOutputDirectory" -DestinationPath "$TargetOutputPackageName"
