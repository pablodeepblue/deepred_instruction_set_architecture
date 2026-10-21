@echo off
if "%1"=="" (
    echo Error: Please provide an argument.
    exit /b 1
)

mkdir bin
mkdir asm
mkdir dsm

set file=%1
for %%F in ("%file%") do set filename_no_ext=%%~nF

if exist "dsm\%filename_no_ext%.txt" (
    del "dsm\%filename_no_ext%.txt"
)

if exist "asm\%filename_no_ext%.txt" (
    del "asm\%filename_no_ext%.txt"
)

python ppu_assembler.py -a -i "%file%" -o "bin\%filename_no_ext%.bin"
python ppu_assembler.py -p -i "%file%" -o "asm\%filename_no_ext%.asm"
python ppu_assembler.py -d -i "bin\%filename_no_ext%.bin" >> "dsm\%filename_no_ext%.txt"