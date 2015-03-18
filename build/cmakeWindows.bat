@echo off

@rd /s /q CMakeFiles
@del cmake_install.cmake
@del CMakeCache.txt
@del Makefile

cmake -G "MSYS Makefiles" -DVTK_DIR:PATH=D:\ComputerGraphics\VTK62 ..