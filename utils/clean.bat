@echo off

del /F /Q "auto"
rmdir /Q /S "auto"

del /F /Q _region_.*
rmdir /S /Q _region_.*

del /F /Q *.aux
del /F /Q *.log
del /F /Q *.out
del /F /Q *.pdf
del /F /Q *.rel
del /F /Q *.toc

del /F /Q *.bak

del /F /Q students_tasks.tex