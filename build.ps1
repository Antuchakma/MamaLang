param(
    [string]$InputFile = "examples/sample.mama",
    [string]$GeneratedC = "build/generated.c",
    [string]$ExePath = "build/program.exe"
)

$ErrorActionPreference = "Stop"

function Require-Command($name) {
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) {
        throw "Required command '$name' was not found in PATH."
    }
}

function Resolve-AnyCommand([string[]]$names) {
    foreach ($n in $names) {
        $cmd = Get-Command $n -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Name }
    }
    throw "None of these commands were found in PATH: $($names -join ', ')"
}

Require-Command gcc

$BisonCmd = Resolve-AnyCommand @("bison", "win_bison")
$FlexCmd = Resolve-AnyCommand @("flex", "win_flex")

if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

& $BisonCmd -d -o build/parser.tab.c src/parser.y
if ($LASTEXITCODE -ne 0) { throw "bison failed" }

& $FlexCmd -o build/lex.yy.c src/lexer.l
if ($LASTEXITCODE -ne 0) { throw "flex failed" }

gcc -Wall -Wextra -O2 -Ibuild build/parser.tab.c build/lex.yy.c -o build/mamalangc.exe
if ($LASTEXITCODE -ne 0) { throw "compiler build failed" }

./build/mamalangc.exe $InputFile $GeneratedC
if ($LASTEXITCODE -ne 0) { throw "mamalang compilation failed" }

gcc -Wall -Wextra -O2 $GeneratedC -o $ExePath
if ($LASTEXITCODE -ne 0) { throw "generated C build failed" }

Write-Host "Build successful. Running generated program..."
& $ExePath
