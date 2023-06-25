set TARGET_PATH=D:\Unzip\SAC\bedrock-server-1.20.1.02

if exist %TARGET_PATH%\bedrock_server_mod.exe goto process

@echo Failed to find BDS path, will not run BDS

goto end

:process
xcopy Release\WorldEdit.dll %TARGET_PATH%\plugins /Y
xcopy Release\WorldEdit.pdb %TARGET_PATH%\plugins /Y

wt %TARGET_PATH%\bedrock_server_mod.exe

:end