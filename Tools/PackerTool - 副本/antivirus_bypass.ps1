# Game Login Anti-Virus Bypass Script
# Version: 2.4
# Date: 2025-01-09

param(
    [string]$InputExe,
    [string]$OutputExe,
    [string]$VMProtectProjectFile = "E:\bypass\shi2elden\2_original.exe.vmp"
)

# Define tool paths
$tools = @{
    "UPX" = "D:\tool\52pojie\Tools\Packers\UPX Shell 3.4\UPXE.EXE"
    "VMProtect" = "D:\Program Files\VMProtect Ultimate\VMProtect_Con.exe"
}

# Logging function with enhanced diagnostics
function Write-Log {
    param(
        [string]$Message, 
        [string]$Level = "Info"
    )
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $color = switch($Level) {
        "Error" { "Red" }
        "Warning" { "Yellow" }
        default { "White" }
    }
    Write-Host "[$timestamp] [$Level] $Message" -ForegroundColor $color
    
    # Additional diagnostic information for errors
    if ($Level -eq "Error") {
        Write-Host "Diagnostic Info:" -ForegroundColor Yellow
        Write-Host "Current Directory: $((Get-Location).Path)" -ForegroundColor Gray
        Write-Host "Input File Exists: $(Test-Path $InputExe)" -ForegroundColor Gray
        Write-Host "Output File Path: $OutputExe" -ForegroundColor Gray
    }
}

# Check if tools exist
function Check-Tools {
    $isValid = $true
    foreach ($toolName in $tools.Keys) {
        $toolPath = $tools[$toolName]
        if (-not (Test-Path $toolPath)) {
            Write-Log "Tool not found: $toolName at $toolPath" -Level "Error"
            $isValid = $false
        }
    }
    return $isValid
}

# Main anti-virus bypass process
function Start-AntivirusBypass {
    param(
        [string]$InputFile,
        [string]$OutputFile,
        [string]$VMProtectProjectFile
    )

    # Validate input file and project file
    if (-not (Test-Path $InputFile)) {
        Write-Log "Input file does not exist: $InputFile" -Level "Error"
        return $false
    }

    if (-not (Test-Path $VMProtectProjectFile)) {
        Write-Log "VMProtect project file does not exist: $VMProtectProjectFile" -Level "Error"
        return $false
    }

    try {
        # Step 1: UPX Compression
        Write-Log "Starting UPX Compression"
        $upxArgs = "--force -9 ""$InputFile"" -o ""$OutputFile.upx"""
        $upxProcess = Start-Process $tools["UPX"] -ArgumentList $upxArgs -PassThru -Wait -NoNewWindow
        
        if ($upxProcess.ExitCode -ne 0) {
            Write-Log "UPX Compression failed" -Level "Error"
            return $false
        }

        # Step 2: VMProtect Obfuscation
        Write-Log "Starting VMProtect Obfuscation"
        
        # Use VMProtect console with specific arguments
        $vmprotectArgs = @(
            """$OutputFile.upx""",
            """$OutputFile""",
            "-pf",
            """$VMProtectProjectFile"""
        )

        Write-Log "VMProtect Command Arguments: $($vmprotectArgs -join ' ')"

        $vmProcess = Start-Process $tools["VMProtect"] -ArgumentList $vmprotectArgs -PassThru -Wait

        if ($vmProcess.ExitCode -ne 0) {
            Write-Log "VMProtect obfuscation failed. Exit Code: $($vmProcess.ExitCode)" -Level "Error"
            return $false
        }

        # Step 3: Random delay
        $randomDelay = Get-Random -Minimum 500 -Maximum 2000
        Start-Sleep -Milliseconds $randomDelay

        Write-Log "Anti-Virus Bypass Completed: $OutputFile"
        return $true
    }
    catch {
        Write-Log "Bypass process error: $($_.Exception.Message)" -Level "Error"
        return $false
    }
}

# Main script execution
try {
    # Validate input parameters
    if (-not $InputExe -or -not $OutputExe) {
        throw "Input and output file paths must be provided"
    }

    # Check tools
    if (-not (Check-Tools)) {
        throw "Tool verification failed"
    }

    # Execute bypass
    $result = Start-AntivirusBypass -InputFile $InputExe -OutputFile $OutputExe -VMProtectProjectFile $VMProtectProjectFile

    # Output results
    if ($result) {
        Write-Host "`n360 Anti-Virus Bypass Recommendations:" -ForegroundColor Yellow
        Write-Host "1. Perform multiple tests" -ForegroundColor Green
        Write-Host "2. Verify file integrity" -ForegroundColor Green
        Write-Host "3. Check 360 detection results" -ForegroundColor Green
    }
    else {
        Write-Host "Anti-Virus Bypass unsuccessful" -ForegroundColor Red
    }
}
catch {
    Write-Host "Processing failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}