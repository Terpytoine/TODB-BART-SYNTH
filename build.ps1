param([string]$Zig = 'zig')
$ErrorActionPreference = 'Stop'
$buildRoot = $PSScriptRoot
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $buildRoot '.zig-cache'
Push-Location $buildRoot
try {
    & $Zig rc /fo bart-icon.res bart-icon.rc
    if ($LASTEXITCODE) { throw 'Icon resource build failed' }
    & $Zig c++ plugin.cpp -std=c++17 -O2 -shared -fms-extensions -I. -o 'BART SYNTH.vst3' -luser32 -lole32 -lcomdlg32 -luuid
    if ($LASTEXITCODE) { throw 'VST3 build failed' }
    & $Zig c++ setup.cpp bart-icon.res -std=c++17 -O2 '-Wl,/subsystem:windows' -o setup-base.exe -luser32 -lshell32 -lole32 -luuid -ldwmapi -lgdi32 -ladvapi32
    if ($LASTEXITCODE) { throw 'Installer build failed' }
    & $Zig c++ host-test.cpp -std=c++17 -O2 -I. -o host-test.exe
    if ($LASTEXITCODE) { throw 'Host test build failed' }
    & $Zig c++ bank-test-v2.cpp -std=c++17 -O2 -I. -o bank-test.exe
    if ($LASTEXITCODE) { throw 'Bank test build failed' }
    & $Zig c++ chord-test.cpp -std=c++17 -O2 -I. -o chord-test.exe
    if ($LASTEXITCODE) { throw 'Chord test build failed' }
    & $Zig c++ uninstall.cpp bart-icon.res -std=c++17 -O2 '-Wl,/subsystem:windows' -o uninstall.exe -luser32 -lshell32 -lole32 -ladvapi32
    if ($LASTEXITCODE) { throw 'Uninstaller build failed' }
    node package.mjs '../build/TODB BART SYNTH'
    .\host-test.exe '..\build\TODB BART SYNTH\TODB BART SYNTH.vst3\Contents\x86_64-win\TODB BART SYNTH.vst3'
    if ($LASTEXITCODE) { throw 'Host tests failed' }
    .\bank-test.exe
    if ($LASTEXITCODE) { throw 'Bank tests failed' }
    .\chord-test.exe
    if ($LASTEXITCODE) { throw 'Chord tests failed' }
} finally { Pop-Location }

