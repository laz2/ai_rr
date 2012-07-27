
@echo off

del /F /Q "Debug"
rmdir /Q /S "Debug"

del /F /Q "Release"
rmdir /Q /S "Release"

del /F /Q "*.user"
del /F /Q "*.ncb"
del /F /Q /A:H "*.suo"
del /F /Q "*.vsp"