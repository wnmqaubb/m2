# File merge script - Append additional data
# Version: 1.3

param(
    [string]$ModifiedExe,
    [string]$AdditionalDataFile,
    [string]$OutputExe
)

# Log function
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
}

# Check if file exists
function Test-FileExists {
    param([string]$FilePath)
    if (-not (Test-Path $FilePath)) {
        Write-Log "File does not exist: $FilePath" -Level "Error"
        return $false
    }
    return $true
}

# Append additional data to executable
function Append-AdditionalData {
    param(
        [string]$ModifiedFile,
        [string]$AdditionalDataFile,
        [string]$OutputFile
    )

    # Validate input files
    if (-not (Test-FileExists $ModifiedFile) -or 
        -not (Test-FileExists $AdditionalDataFile)) {
        return $false
    }

    try {
        # Read modified executable bytes
        $modifiedBytes = [System.IO.File]::ReadAllBytes($ModifiedFile)
        # Read additional data bytes
        $additionalDataBytes = [System.IO.File]::ReadAllBytes($AdditionalDataFile)

        # Combine modified executable with additional data
        $finalBytes = $modifiedBytes + $additionalDataBytes

        # Write new file
        [System.IO.File]::WriteAllBytes($OutputFile, $finalBytes)

        Write-Log "File append successful: $OutputFile" -Level "Info"
        Write-Log "Modified Exe Size: $($modifiedBytes.Length) bytes" -Level "Info"
        Write-Log "Additional Data Size: $($additionalDataBytes.Length) bytes" -Level "Info"
        return $true
    }
    catch {
        Write-Log "Append failed: $($_.Exception.Message)" -Level "Error"
        return $false
    }
}

# Main execution logic
try {
    $result = Append-AdditionalData -ModifiedFile $ModifiedExe `
                                    -AdditionalDataFile $AdditionalDataFile `
                                    -OutputFile $OutputExe

    if ($result) {
        Write-Host "`nAppend recommendations:" -ForegroundColor Yellow
        Write-Host "1. Verify output file integrity" -ForegroundColor Green
        Write-Host "2. Test program functionality" -ForegroundColor Green
        Write-Host "3. Perform antivirus scan" -ForegroundColor Green
    }
    else {
        Write-Host "File append failed" -ForegroundColor Red
    }
}
catch {
    Write-Host "Processing error: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
