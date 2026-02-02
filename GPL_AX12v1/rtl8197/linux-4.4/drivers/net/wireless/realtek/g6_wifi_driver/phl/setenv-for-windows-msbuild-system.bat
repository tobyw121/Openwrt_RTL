@echo off
powershell Set-ExecutionPolicy RemoteSigned
powershell "& "./setenv-for-windows-msbuild-system.ps1 %*""
