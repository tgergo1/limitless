# SPDX-License-Identifier: GPL-3.0-only
param(
  [string]$Mode = "default",
  [string]$Compiler = "cl"
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
Set-Location $root
python tools/check_reserved_filenames.py
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

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
  param([string]$OutExe, [string[]]$Sources, [string[]]$ExtraFlags = @())
  & $Compiler @cxxFlags @defs @ExtraFlags @Sources "/Fe:$OutExe"
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

Invoke-CompileC (Join-Path $build "test_limitless_parse_edges.exe") @("tests/test_limitless_parse_edges.c")
& (Join-Path $build "test_limitless_parse_edges.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "test_limitless_conversion_edges.exe") @("tests/test_limitless_conversion_edges.c")
& (Join-Path $build "test_limitless_conversion_edges.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "test_limitless_invariants.exe") @("tests/test_limitless_invariants.c")
& (Join-Path $build "test_limitless_invariants.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileC (Join-Path $build "test_limitless_allocator_contract.exe") @("tests/test_limitless_allocator_contract.c")
& (Join-Path $build "test_limitless_allocator_contract.exe")
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

Invoke-CompileCpp (Join-Path $build "test_multi_cpp.exe") @("tests/multi_cpp_impl.cpp", "tests/multi_cpp_a.cpp", "tests/multi_cpp_b.cpp")
& (Join-Path $build "test_multi_cpp.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_limitless_cpp_basic.exe") @("tests/test_limitless_cpp.cpp")
& (Join-Path $build "test_limitless_cpp_basic.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_limitless_cpp_generated.exe") @("tests/test_limitless_cpp_generated.cpp")
& (Join-Path $build "test_limitless_cpp_generated.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_cpp_namespace_strict.exe") @("tests/test_cpp_namespace_strict.cpp")
& (Join-Path $build "test_cpp_namespace_strict.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_cpp_legacy_bridge.exe") @("tests/test_cpp_legacy_bridge.cpp") @("/wd4996")
& (Join-Path $build "test_cpp_legacy_bridge.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_limitless_threads.exe") @("tests/test_limitless_threads.cpp")
& (Join-Path $build "test_limitless_threads.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_limitless_threads_c_api.exe") @("tests/test_limitless_threads_c_api.cpp")
& (Join-Path $build "test_limitless_threads_c_api.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Invoke-CompileCpp (Join-Path $build "test_limitless_cpp_cross_thread_status.exe") @("tests/test_limitless_cpp_cross_thread_status.cpp")
& (Join-Path $build "test_limitless_cpp_cross_thread_status.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Compiler @cxxFlags @defs "/I$root" "tests/test_include_repo_root.cpp" "/Fe:$(Join-Path $build "test_include_repo_root.exe")"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $build "test_include_repo_root.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$negativeObj = Join-Path $build "strict_legacy_symbol_fail.obj"
$negativeLog = Join-Path $build "strict_legacy_symbol_fail.log"
& $Compiler @cxxFlags @defs "/c" "tests/negative/strict_legacy_symbol_fail.cpp" "/Fo:$negativeObj" *> $negativeLog
$negativeExit = $LASTEXITCODE
if ($negativeExit -eq 0) {
  Write-Error "negative compile unexpectedly succeeded"
  Get-Content $negativeLog
  exit 1
}
$negativeContent = Get-Content -Raw $negativeLog
if ($negativeContent -notmatch "limitless_number|limitless_cpp_set_default_ctx|undeclared|not declared|identifier") {
  Write-Error "negative compile failed without expected symbol diagnostics"
  Get-Content $negativeLog
  exit 1
}

Invoke-CompileC (Join-Path $build "limitless_cli.exe") @("tests/ci/limitless_cli.c")
$iters = 3000
if ($Mode -eq "noexceptions") { $iters = 2000 }
python tests/ci/diff_reference.py --cli (Join-Path $build "limitless_cli.exe") --iters $iters
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
