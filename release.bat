set version=1

call build.bat
if errorlevel 1 goto :eof

md buildmon-%version%

copy *.bat buildmon-%version%
copy *.cpp buildmon-%version%
copy *.exe buildmon-%version%
copy *.md buildmon-%version%
copy *.py buildmon-%version%
copy LICENSE buildmon-%version%

del *.zip
7z a buildmon-%version%.zip buildmon-%version% -tzip

rd /q /s buildmon-%version%
