param(
    [string]$InputFile = "examples/sample.mama",
    [string]$GeneratedC = "build/generated.c",
    [string]$ExePath = "build/program.exe"
)

$ErrorActionPreference = "Stop"

# MSYS2 paths — adjust if your MSYS2 is installed elsewhere
$MSYS2 = "C:\msys64"
$MSYS2_USR_BIN = "$MSYS2\usr\bin"
$MSYS2_MINGW_BIN = "$MSYS2\mingw64\bin"

function Find-Tool($name, [string[]]$searchPaths) {
    # Check PATH first
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    # Check known MSYS2 locations
    foreach ($dir in $searchPaths) {
        $full = Join-Path $dir "$name.exe"
        if (Test-Path $full) { return $full }
    }
    throw "Could not find '$name'. Install MSYS2 from https://www.msys2.org/ and run: pacman -S base-devel mingw-w64-ucrt-x86_64-gcc flex bison make"
}

$GCC   = Find-Tool "gcc"       @($MSYS2_MINGW_BIN, $MSYS2_USR_BIN)
$BISON = Find-Tool "bison"     @($MSYS2_USR_BIN, $MSYS2_MINGW_BIN)
$FLEX  = Find-Tool "flex"      @($MSYS2_USR_BIN, $MSYS2_MINGW_BIN)

Write-Host "Using GCC:   $GCC"
Write-Host "Using Bison: $BISON"
Write-Host "Using Flex:  $FLEX"

if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Set PATH so bison can find m4
$env:PATH = "$MSYS2_USR_BIN;$MSYS2_MINGW_BIN;$env:PATH"

& $BISON -d -o build/parser.tab.c src/parser.y
if ($LASTEXITCODE -ne 0) { throw "bison failed" }

& $FLEX -o build/lex.yy.c src/lexer.l
if ($LASTEXITCODE -ne 0) { throw "flex failed" }

& $GCC -Wall -Wextra -O2 -Ibuild build/parser.tab.c build/lex.yy.c -o build/mamalangc.exe
if ($LASTEXITCODE -ne 0) { throw "compiler build failed" }

Write-Host "Compiler built successfully. Compiling $InputFile ..."

./build/mamalangc.exe $InputFile $GeneratedC
if ($LASTEXITCODE -ne 0) { throw "mamalang compilation failed" }

& $GCC -Wall -Wextra -O2 $GeneratedC -o $ExePath
if ($LASTEXITCODE -ne 0) { throw "generated C build failed" }

Write-Host "Build successful. Running generated program..."
Write-Host "-------------------------------------------"
& $ExePath
