@echo off
mkdir bin
mkdir dsm

for %%F in (src\*) do (
    call assembler.bat "%%F"
)
