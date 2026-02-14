# SPDX-License-Identifier: GPL-3.0-only
param(
  [string]$Mode = "default",
  [string]$Compiler = "cl"
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
Set-Location $root

$build = Join-Path $root "build/ci/windows-$Compiler-$Mode"
New-Item -ItemType Directory -Path $build -Force | Out-Null

$cFlags = @("/nologo", "/W4", "/WX", "/std:c11")
$cxxFlags = @("/nologo", "/W4", "/WX", "/std:c++14", "/permissive-")
$defs = @()

if ($Mode -eq "limb64") {
  $defs += "/DLIMITLESS_LIMB_BITS=64"
}
if ($Mode -eq "extended-stress") {
  $defs += "/DLIMITLESS_EXTENDED_STRESS"
}
if ($Mode -eq "noexceptions") {
  if ($Compiler -eq "clang-cl") {
    $cxxFlags += "/clang:-fno-exceptions"
  } else {
    $cxxFlags += "/D_HAS_EXCEPTIONS=0"
  }
}

function Invoke-CompileC {
  param([string]$OutExe, [string[]]$Sources)
  & $Compiler @cFlags @defs @Sources "/Fe:$OutExe"
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

function Invoke-CompileCpp {
  param([string]$OutExe, [string[]]$Sources)
  & $Compiler @cxxFlags @defs @Sources "/Fe:$OutExe"
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if ($Mode -eq "extended-stress") {
  Invoke-CompileC (Join-Path $build "test_limitless_generated_stress.exe") @("tests/test_limitless_generated.c")
  & (Join-Path $build "test_limitless_generated_stress.exe")
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

  Invoke-CompileCpp (Join-Path $build "test_limitless_cpp_generated_stress.exe") @("tests/test_limitless_cpp_generated.cpp")
  & (Join-Path $build "test_limitless_cpp_generated_stress.exe")
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  exit 0
}

Invoke-CompileC (Join-Path $build "test_limitless_c_basic.exe") @("tests/test_limitless.c")
& (Join-Path $build "test_limitless_c_basic.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "test_limitless_api.exe") @("tests/test_limitless_api.c")
& (Join-Path $build "test_limitless_api.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "test_limitless_parser_fuzz.exe") @("tests/test_limitless_parser_fuzz.c")
& (Join-Path $build "test_limitless_parser_fuzz.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "test_limitless_c_generated.exe") @("tests/test_limitless_generated.c")
& (Join-Path $build "test_limitless_c_generated.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "test_default_allocator_override.exe") @("tests/test_default_allocator_override.c")
& (Join-Path $build "test_default_allocator_override.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "test_multi.exe") @("tests/multi_impl.c", "tests/multi_a.c", "tests/multi_b.c")
& (Join-Path $build "test_multi.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_limitless_cpp_basic.exe") @("tests/test_limitless_cpp.cpp")
& (Join-Path $build "test_limitless_cpp_basic.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_limitless_cpp_generated.exe") @("tests/test_limitless_cpp_generated.cpp")
& (Join-Path $build "test_limitless_cpp_generated.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_limitless_threads.exe") @("tests/test_limitless_threads.cpp")
& (Join-Path $build "test_limitless_threads.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "limitless_cli.exe") @("tests/ci/limitless_cli.c")
$iters = 2000
if ($Mode -eq "noexceptions") { $iters = 1500 }
python tests/ci/diff_reference.py --cli (Join-Path $build "limitless_cli.exe") --iters $iters
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
