if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
cl /O2 *.cpp advapi32.lib /link /manifest:embed /manifestuac:level='requireAdministrator'
