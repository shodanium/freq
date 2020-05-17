@echo off
echo === building everything...
setlocal

echo === freq01.cpp
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" 1>nul
cl.exe /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Gm- /O2 /sdl /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:column /Fo"bin\\" /Fe"bin\freq01cpp.exe" freq01.cpp windows-mmap.c

echo === freq01.go
call go build -o bin\freq01go.exe freq01.go

echo === main.rs
cd rust
call cargo build --release --color=never
cd ..
move "rust\target\release\freq.exe" "bin\freq01rs.exe"

echo === all done
endlocal
