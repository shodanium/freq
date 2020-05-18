@echo off
echo === building everything...

setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" 1>nul

echo === freq03.cpp
cl.exe /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Gm- /O2 /sdl /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:column /Fo"bin\\" /Fe"bin\freq03cpp.exe" src\freq03.cpp src\ext\windows-mmap.c

echo === freq01.cpp
cl.exe /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Gm- /O2 /sdl /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:column /Fo"bin\\" /Fe"bin\freq01cpp.exe" src\freq01.cpp src\ext\windows-mmap.c

echo === freq02.cpp
cl.exe /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Gm- /O2 /sdl /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:column /Fo"bin\\" /Fe"bin\freq02cpp.exe" src\freq02.cpp src\ext\windows-mmap.c

echo === hack01.cpp
cl.exe /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Gm- /O2 /sdl /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:column /Fo"bin\\" /Fe"bin\hack01cpp.exe" src\hack01.cpp src\ext\windows-mmap.c

echo === freq01.go
call go build -o bin\freq01go.exe src\freq01.go

echo === freq01.rs
cd build\rust
call cargo build --release --color=never
cd ..\..
move "junk\rust\release\freq01.exe" "bin\freq01rs.exe"

echo === all done
endlocal
