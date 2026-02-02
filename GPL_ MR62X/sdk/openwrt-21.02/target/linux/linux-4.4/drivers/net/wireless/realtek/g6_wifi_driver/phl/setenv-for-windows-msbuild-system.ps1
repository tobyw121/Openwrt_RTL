# -h [8.0|8.1]
# -t [32|64|Arm]
# -os [6|7|8|8.1]
# -d [chk|fre]

param(
        [string]$h,
        [string]$t, 
        [string]$os, 
        [string]$d
     )

$global:setenv           = ""
$global:toolsetCmd       = ""
$global:targetMachineCmd = ""
$global:targetOSCmd      = ""
$global:buildEnvCmd      = ""

function PromptForChoiceToSetEnv
{
    Write-Host "`n--------------------------------------------------------------------------" -foregroundcolor Green
    $question    = "Windows Kernel Mode Driver Toolset Version?"
    $act3 = "set MSBUILD_PLATFORM_TOOLSET=WindowsKernelModeDriver10.0"
    $opt3 = New-Object System.Management.Automation.Host.ChoiceDescription "10.&0 (WDK-????)", $act3
	$options = [System.Management.Automation.Host.ChoiceDescription[]]($opt3)
    $result = $host.ui.PromptForChoice($question, $description, $options, 0) 
    
    switch ($result)
    {
        0 { "10.0"; $global:toolsetCmd = $act3; }
    }
    
    Write-Host "`n--------------------------------------------------------------------------" -foregroundcolor Green
    $question    = "Target OS?"
    $act2 = "set DDK_TARGET_OS=Win7"
    $opt2 = New-Object System.Management.Automation.Host.ChoiceDescription "Win&7", $act2
	$act5 = "set DDK_TARGET_OS=Win10"
    $opt5 = New-Object System.Management.Automation.Host.ChoiceDescription "Win&10", $act5
    $options = [System.Management.Automation.Host.ChoiceDescription[]]($opt2, $opt5)
    $result = $host.ui.PromptForChoice($question, $description, $options, 1) 
    
    switch ($result)
    {
        0 { "Win7";    $global:targetOSCmd = $act2; }
		1 { "Win10";   $global:targetOSCmd = $act5; }
    }
    
    Write-Host "`n--------------------------------------------------------------------------" -foregroundcolor Green
    $question    = "Target Machine?"
    $act1 = "set BUILD_DEFAULT_TARGETS=-i386"
    $opt1 = New-Object System.Management.Automation.Host.ChoiceDescription "i&386", $act1
    $act2 = "set BUILD_DEFAULT_TARGETS=-amd64"
    $opt2 = New-Object System.Management.Automation.Host.ChoiceDescription "amd&64", $act2
    $act3 = "set BUILD_DEFAULT_TARGETS=-Arm"
    $opt3 = New-Object System.Management.Automation.Host.ChoiceDescription "&Arm", $act2
    $options = [System.Management.Automation.Host.ChoiceDescription[]]($opt1, $opt2, $opt3)
    $result = $host.ui.PromptForChoice($question, $description, $options, 0) 
    
    switch ($result)
    {
        0 { "i386";  $global:targetMachineCmd = $act1; }
        1 { "amd64"; $global:targetMachineCmd = $act2; }
        2 { "Arm";   $global:targetMachineCmd = $act3; }
    }
    
    Write-Host "`n--------------------------------------------------------------------------" -foregroundcolor Green
    $question    = "Checked or Free Build?"
    $act1 = "set DDKBUILDENV=chk"
    $opt1 = New-Object System.Management.Automation.Host.ChoiceDescription "&chk", $act1
    $act2 = "set DDKBUILDENV=fre"
    $opt2 = New-Object System.Management.Automation.Host.ChoiceDescription "&fre", $act2
    $options = [System.Management.Automation.Host.ChoiceDescription[]]($opt1, $opt2)
    $result = $host.ui.PromptForChoice($question, $description, $options, 0) 
    
    switch ($result)
    {
        0 { "chk"; $global:buildEnvCmd = $act1; }
        1 { "fre"; $global:buildEnvCmd = $act2; }
    }
    $global:setenv = "$global:targetMachineCmd&&$global:targetOSCmd&&$global:buildEnvCmd&&$global:toolsetCmd&&set USE_MSBUILD_SYSTEM=true"
}

if ($PSBoundParameters.count -le 0)
{
    PromptForChoiceToSetEnv
}
else
{
    if     ($h -ieq "8.0")  { $global:toolsetCmd = "set MSBUILD_PLATFORM_TOOLSET=WindowsKernelModeDriver8.0"}
    elseif ($h -ieq "8.1")  { $global:toolsetCmd = "set MSBUILD_PLATFORM_TOOLSET=WindowsKernelModeDriver8.1"}
    elseif ($h -ine "")     { Write-Host "-h [8.0|8.1]"; $global:toolsetCmd = "" }
    else                    { $global:toolsetCmd = "set MSBUILD_PLATFORM_TOOLSET="+$toolset
                              $toolset = ([string](Invoke-Expression("cmd /c set MSBUILD_PLATFORM_TOOLSET"))).split("=")[1]
                            }

    if     ($t -ieq "32")   { $global:targetMachineCmd = "set BUILD_DEFAULT_TARGETS=-i386"}
    elseif ($t -ieq "64")   { $global:targetMachineCmd = "set BUILD_DEFAULT_TARGETS=-amd64"}
    elseif ($t -ieq "Arm")  { $global:targetMachineCmd = "set BUILD_DEFAULT_TARGETS=-Arm"}
    elseif ($t -ine "")     { Write-Host "-t [32|64|Arm]"; $global:targetMachineCmd = ""; }
    else                    { $global:targetMachineCmd = "set BUILD_DEFAULT_TARGETS="+$targetMachine
                              $targetMachine = ([string](Invoke-Expression("cmd /c set BUILD_DEFAULT_TARGETS"))).split("=")[1]
                            }

    if     ($os -ieq "6")   { $global:targetOSCmd = "set DDK_TARGET_OS=Vista"}
    elseif ($os -ieq "7")   { $global:targetOSCmd = "set DDK_TARGET_OS=Win7"}
    elseif ($os -ieq "8")   { $global:targetOSCmd = "set DDK_TARGET_OS=Win8"}
    elseif ($os -ieq "8.1") { $global:targetOSCmd = "set DDK_TARGET_OS=WinBlue"}
    elseif ($os -ine "")    { Write-Host "-os [6|7|8|8.1]"; $global:targetOSCmd = ""; }
    else                    { $global:targetOSCmd = "set DDK_TARGET_OS="+$targetOS
                              $targetOS = ([string](Invoke-Expression("cmd /c set DDK_TARGET_OS"))).split("=")[1]
                            }

    if     ($d -ieq "chk")  { $global:buildEnvCmd = "set DDKBUILDENV=chk"}
    elseif ($d -ieq "fre")  { $global:buildEnvCmd = "set DDKBUILDENV=fre"}
    elseif ($d -ine "")     { Write-Host "-d [chk|fre]"; $global:buildEnvCmd = ""; }
    else                    { $global:buildEnvCmd = "set DDKBUILDENV="+$buildEnv
                              $buildEnv = ([string](Invoke-Expression("cmd /c set DDKBUILDENV"))).split("=")[1]
                            }

    if ($global:toolsetCmd -ine "")       { $global:setenv += "$global:toolsetCmd&&" }
    if ($global:targetMachineCmd -ine "") { $global:setenv += "$global:targetMachineCmd&&" }
    if ($global:targetOSCmd -ine "")      { $global:setenv += "$global:targetOSCmd&&" }
    if ($global:buildEnvCmd -ine "")      { $global:setenv += "$global:buildEnvCmd&&" }

    $global:setenv += "set USE_MSBUILD_SYSTEM=true"
}

$global:setenv += "&&set MSBUILD_PLATFORM_TOOLSET"
$global:setenv += "&&set DDK_TARGET_OS"
$global:setenv += "&&set BUILD_DEFAULT_TARGETS"
$global:setenv += "&&set DDKBUILDENV"

$toolset        = $global:toolsetCmd.split("=")[1]
$targetMachine  = $global:targetMachineCmd.split("=")[1]
$targetOS       = $global:targetOSCmd.split("=")[1]
$buildEnv       = $global:buildEnvCmd.split("=")[1]
$global:setenv += "&&title $toolset - ($targetOS, $targetMachine, $buildEnv)"

Write-Host "`n------------------------- MSBuild Final Settings -------------------------" -foregroundcolor Yellow

$c = $global:setenv # + "&& buildrtwe && exit"
cmd /k $c

# SIG # Begin signature block
# MIIThQYJKoZIhvcNAQcCoIITdjCCE3ICAQExCzAJBgUrDgMCGgUAMGkGCisGAQQB
# gjcCAQSgWzBZMDQGCisGAQQBgjcCAR4wJgIDAQAABBAfzDtgWUsITrck0sYpfvNR
# AgEAAgEAAgEAAgEAAgEAMCEwCQYFKw4DAhoFAAQUizA6jJO/b6ZyY+82+4UQFj86
# PFCggg98MIICOTCCAaagAwIBAgIQBHm9ayresZpKygIl3MQHVTAJBgUrDgMCHQUA
# MCwxKjAoBgNVBAMTIVBvd2VyU2hlbGwgTG9jYWwgQ2VydGlmaWNhdGUgUm9vdDAe
# Fw0xMzA3MTkxMzAzMTNaFw0zOTEyMzEyMzU5NTlaMBoxGDAWBgNVBAMTD1Bvd2Vy
# U2hlbGwgVXNlcjCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA6C2+IsrjookO
# G90yAxrccI+dBPeNBC4kyTiVujax6s/44MijZAGeR07M+MNsvPjCwVT0d5ulsw5v
# Gf54e2iL1H2hChcQiX4ed1MXMvOvE7E1BWhD5khc/sqA2PlXdxs2eEWVCE2BsleK
# V0NMa6bti7n3tfZqX8lBq/WZPIR1CE0CAwEAAaN2MHQwEwYDVR0lBAwwCgYIKwYB
# BQUHAwMwXQYDVR0BBFYwVIAQ975/eRF3jqmfV0QJHiV8sqEuMCwxKjAoBgNVBAMT
# IVBvd2VyU2hlbGwgTG9jYWwgQ2VydGlmaWNhdGUgUm9vdIIQxcHP7/9EzJJF9Loz
# Y6VfFjAJBgUrDgMCHQUAA4GBACAQUZdjMoIDBfKv4KPfQfnJwAigrwkX6dBbfivE
# WzivFIyXtMXcw5wcJ0LUo8fd9aUJWF2JaZNUYrRVGrEIZNm1kQ9jjJJ1sK+vaWPV
# HZh6qIkSe5jGJ97NI5X4F20GT1TXnDiA7rYazxhxTzCoiaxKysL6cUkAi61pdlXZ
# ch4tMIIGajCCBVKgAwIBAgIQA5/t7ct5W43tMgyJGfA2iTANBgkqhkiG9w0BAQUF
# ADBiMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQL
# ExB3d3cuZGlnaWNlcnQuY29tMSEwHwYDVQQDExhEaWdpQ2VydCBBc3N1cmVkIElE
# IENBLTEwHhcNMTMwNTIxMDAwMDAwWhcNMTQwNjA0MDAwMDAwWjBHMQswCQYDVQQG
# EwJVUzERMA8GA1UEChMIRGlnaUNlcnQxJTAjBgNVBAMTHERpZ2lDZXJ0IFRpbWVz
# dGFtcCBSZXNwb25kZXIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6
# aUqBTW+lFBaqis1nvku/xmmPWBzgeegenVgmmNpc1Hyj+dsrjBI2w/z5ZAaxu8Ko
# mAoXDeGV60C065ZtmL+mj3nPvIqSe22cGAZR2KUYUzIBJxlh6IRB38bw6Mr+d61f
# 2J57jGBvhVxGvWvnD4DO5wPDfDHPt2VVxvvgmQjkc1r7l9rQTL60tsYPfyaSqbj8
# OO605DqkSNBM6qlGJ1vPkhGTnBan/tKtHyLFHqzBce+8StsBCUTfmBwtZ7qoigMz
# yVG19wJNCaRN/oBexddFw30IqgEzzDPYTzAW5P8iMi7rfjvw+R4y65Ul0vL+bVSE
# utXl1NHdG6+9WXuUhTABAgMBAAGjggM1MIIDMTAOBgNVHQ8BAf8EBAMCB4AwDAYD
# VR0TAQH/BAIwADAWBgNVHSUBAf8EDDAKBggrBgEFBQcDCDCCAb8GA1UdIASCAbYw
# ggGyMIIBoQYJYIZIAYb9bAcBMIIBkjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cu
# ZGlnaWNlcnQuY29tL0NQUzCCAWQGCCsGAQUFBwICMIIBVh6CAVIAQQBuAHkAIAB1
# AHMAZQAgAG8AZgAgAHQAaABpAHMAIABDAGUAcgB0AGkAZgBpAGMAYQB0AGUAIABj
# AG8AbgBzAHQAaQB0AHUAdABlAHMAIABhAGMAYwBlAHAAdABhAG4AYwBlACAAbwBm
# ACAAdABoAGUAIABEAGkAZwBpAEMAZQByAHQAIABDAFAALwBDAFAAUwAgAGEAbgBk
# ACAAdABoAGUAIABSAGUAbAB5AGkAbgBnACAAUABhAHIAdAB5ACAAQQBnAHIAZQBl
# AG0AZQBuAHQAIAB3AGgAaQBjAGgAIABsAGkAbQBpAHQAIABsAGkAYQBiAGkAbABp
# AHQAeQAgAGEAbgBkACAAYQByAGUAIABpAG4AYwBvAHIAcABvAHIAYQB0AGUAZAAg
# AGgAZQByAGUAaQBuACAAYgB5ACAAcgBlAGYAZQByAGUAbgBjAGUALjALBglghkgB
# hv1sAxUwHwYDVR0jBBgwFoAUFQASKxOYspkH7R7for5XDStnAs0wHQYDVR0OBBYE
# FGMvyd95knu1I8q74aTuM37j4p36MH0GA1UdHwR2MHQwOKA2oDSGMmh0dHA6Ly9j
# cmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEFzc3VyZWRJRENBLTEuY3JsMDigNqA0
# hjJodHRwOi8vY3JsNC5kaWdpY2VydC5jb20vRGlnaUNlcnRBc3N1cmVkSURDQS0x
# LmNybDB3BggrBgEFBQcBAQRrMGkwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRp
# Z2ljZXJ0LmNvbTBBBggrBgEFBQcwAoY1aHR0cDovL2NhY2VydHMuZGlnaWNlcnQu
# Y29tL0RpZ2lDZXJ0QXNzdXJlZElEQ0EtMS5jcnQwDQYJKoZIhvcNAQEFBQADggEB
# AKt0vUAATHYVJVc90xwD/31FyEUSZucoZWDY3zuz+g3BrDOP9IG5YfGd+5hV195H
# Q7qAPfFIzD9nMFYfzvTQTIS9h6SexeEPqAZd0C9uXtwZ6PCH6uBOrz1sII5zb37W
# hxjghtOa/J7qjHLpQQ+4cbU4LPgpstUcop0b7F8quNw3IOHLu/DQbGyls8ufSvZU
# 4yY0PS64wSsct/bDPf7RLR5Q9JTI+P3uc9tJtRv09f+lkME5FBvY7XEbapj7+kCa
# RKkpDlVeeLi3pIPDcAHwZkDlrnk04StNA6Et5ttUYhjt1QmLoqrWDMhPGr6ZJXhp
# mYnUWYne34jw02dedKWdpkQwggbNMIIFtaADAgECAhAG/fkDlgOt6gAK6z8nu7ob
# MA0GCSqGSIb3DQEBBQUAMGUxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2Vy
# dCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5jb20xJDAiBgNVBAMTG0RpZ2lD
# ZXJ0IEFzc3VyZWQgSUQgUm9vdCBDQTAeFw0wNjExMTAwMDAwMDBaFw0yMTExMTAw
# MDAwMDBaMGIxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAX
# BgNVBAsTEHd3dy5kaWdpY2VydC5jb20xITAfBgNVBAMTGERpZ2lDZXJ0IEFzc3Vy
# ZWQgSUQgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOiCLZn5
# ysJClaWAc0Bw0p5WVFypxNJBBo/JM/xNRZFcgZ/tLJz4FlnfnrUkFcKYubR3SdyJ
# xArar8tea+2tsHEx6886QAxGTZPsi3o2CAOrDDT+GEmC/sfHMUiAfB6iD5IOUMnG
# h+s2P9gww/+m9/uizW9zI/6sVgWQ8DIhFonGcIj5BZd9o8dD3QLoOz3tsUGj7T++
# 25VIxO4es/K8DCuZ0MZdEkKB4YNugnM/JksUkK5ZZgrEjb7SzgaurYRvSISbT0C5
# 8Uzyr5j79s5AXVz2qPEvr+yJIvJrGGWxwXOt1/HYzx4KdFxCuGh+t9V3CidWfA9i
# pD8yFGCV/QcEogkCAwEAAaOCA3owggN2MA4GA1UdDwEB/wQEAwIBhjA7BgNVHSUE
# NDAyBggrBgEFBQcDAQYIKwYBBQUHAwIGCCsGAQUFBwMDBggrBgEFBQcDBAYIKwYB
# BQUHAwgwggHSBgNVHSAEggHJMIIBxTCCAbQGCmCGSAGG/WwAAQQwggGkMDoGCCsG
# AQUFBwIBFi5odHRwOi8vd3d3LmRpZ2ljZXJ0LmNvbS9zc2wtY3BzLXJlcG9zaXRv
# cnkuaHRtMIIBZAYIKwYBBQUHAgIwggFWHoIBUgBBAG4AeQAgAHUAcwBlACAAbwBm
# ACAAdABoAGkAcwAgAEMAZQByAHQAaQBmAGkAYwBhAHQAZQAgAGMAbwBuAHMAdABp
# AHQAdQB0AGUAcwAgAGEAYwBjAGUAcAB0AGEAbgBjAGUAIABvAGYAIAB0AGgAZQAg
# AEQAaQBnAGkAQwBlAHIAdAAgAEMAUAAvAEMAUABTACAAYQBuAGQAIAB0AGgAZQAg
# AFIAZQBsAHkAaQBuAGcAIABQAGEAcgB0AHkAIABBAGcAcgBlAGUAbQBlAG4AdAAg
# AHcAaABpAGMAaAAgAGwAaQBtAGkAdAAgAGwAaQBhAGIAaQBsAGkAdAB5ACAAYQBu
# AGQAIABhAHIAZQAgAGkAbgBjAG8AcgBwAG8AcgBhAHQAZQBkACAAaABlAHIAZQBp
# AG4AIABiAHkAIAByAGUAZgBlAHIAZQBuAGMAZQAuMAsGCWCGSAGG/WwDFTASBgNV
# HRMBAf8ECDAGAQH/AgEAMHkGCCsGAQUFBwEBBG0wazAkBggrBgEFBQcwAYYYaHR0
# cDovL29jc3AuZGlnaWNlcnQuY29tMEMGCCsGAQUFBzAChjdodHRwOi8vY2FjZXJ0
# cy5kaWdpY2VydC5jb20vRGlnaUNlcnRBc3N1cmVkSURSb290Q0EuY3J0MIGBBgNV
# HR8EejB4MDqgOKA2hjRodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vRGlnaUNlcnRB
# c3N1cmVkSURSb290Q0EuY3JsMDqgOKA2hjRodHRwOi8vY3JsNC5kaWdpY2VydC5j
# b20vRGlnaUNlcnRBc3N1cmVkSURSb290Q0EuY3JsMB0GA1UdDgQWBBQVABIrE5iy
# mQftHt+ivlcNK2cCzTAfBgNVHSMEGDAWgBRF66Kv9JLLgjEtUYunpyGd823IDzAN
# BgkqhkiG9w0BAQUFAAOCAQEARlA+ybcoJKc4HbZbKa9Sz1LpMUerVlx71Q0LQbPv
# 7HUfdDjyslxhopyVw1Dkgrkj0bo6hnKtOHisdV0XFzRyR4WUVtHruzaEd8wkpfME
# GVWp5+Pnq2LN+4stkMLA0rWUvV5PsQXSDj0aqRRbpoYxYqioM+SbOafE9c4deHaU
# JXPkKqvPnHZL7V/CSxbkS3BMAIke/MV5vEwSV/5f4R68Al2o/vsHOE8Nxl2RuQ9n
# Rc3Wg+3nkg2NsWmMT/tZ4CMP0qquAHzunEIOz5HXJ7cW7g/DvXwKoO4sCFWFIrjr
# GBpN/CohrUkxg0eVd3HcsRtLSxwQnHcUwZ1PL1qVCCkQJjGCA3MwggNvAgEBMEAw
# LDEqMCgGA1UEAxMhUG93ZXJTaGVsbCBMb2NhbCBDZXJ0aWZpY2F0ZSBSb290AhAE
# eb1rKt6xmkrKAiXcxAdVMAkGBSsOAwIaBQCgeDAYBgorBgEEAYI3AgEMMQowCKAC
# gAChAoAAMBkGCSqGSIb3DQEJAzEMBgorBgEEAYI3AgEEMBwGCisGAQQBgjcCAQsx
# DjAMBgorBgEEAYI3AgEVMCMGCSqGSIb3DQEJBDEWBBRN3tSgzRDGRUENt8y9tNsT
# 5Q7eozANBgkqhkiG9w0BAQEFAASBgFhSceUjqbrmRE5nawFHAviVOAlwtaK8rQRA
# +f2wn0wSlJwIIm5XTC0m16IV4i/zUc1h6M97yfS6d6BA1CSkgmBS8oCdNtixxhL+
# YAvAvwRMhxPZhzFDL/5Pk2Od0qrjGSH3FLWcR7Ti6kSEU+uGa4dJWrXlM0vw6zJR
# rzO4pNVyoYICDzCCAgsGCSqGSIb3DQEJBjGCAfwwggH4AgEBMHYwYjELMAkGA1UE
# BhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3LmRpZ2lj
# ZXJ0LmNvbTEhMB8GA1UEAxMYRGlnaUNlcnQgQXNzdXJlZCBJRCBDQS0xAhADn+3t
# y3lbje0yDIkZ8DaJMAkGBSsOAwIaBQCgXTAYBgkqhkiG9w0BCQMxCwYJKoZIhvcN
# AQcBMBwGCSqGSIb3DQEJBTEPFw0xMzA5MTAwMjU0MjFaMCMGCSqGSIb3DQEJBDEW
# BBRBViJdsUdT220GizXVUIeUSOHbKTANBgkqhkiG9w0BAQEFAASCAQAxavXiUyZW
# Pn/ZW09mP0BUQv8OsiBblmP6TVSUIGZrzJb5xkVqDjqx8OYEEXzUj/2mmNp7uawh
# 8gjRGMea5gl9OfQcmzvKyxztdYsZaN8yBwq1MVl4QT4HBRHheAWja+bVz+BFGHCI
# eMQlk36wF/NzCv/bA/4rvzimC81+aP9wVfcmG1BjGgOs2ASfvilCG53ZlgtA+1zN
# UTAb/imY244WDUB9aiUjvWlBvQXMPaGEPwS+f3hT/nLVjTtkvgN8eGZWdDHBEaJl
# HQKGYypNg5OPK5XmZ349IYH1mWUGso+h55w2SYv+LyVcEciiD9U1FknIqBzlWbvK
# z8Oi1Q847x8O
# SIG # End signature block
