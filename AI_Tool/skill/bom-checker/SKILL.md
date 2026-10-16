---
name: "bom-checker"
description: "Scans C/H source files for UTF-8 BOM characters that break compilers and strips them byte-safely. Invoke when a build fails with 'unexpected character <U+FEFF>' or when checking modules for encoding issues."
---

# BOM Checker

Detects and removes UTF-8 Byte Order Mark (BOM, U+FEFF) characters at the beginning of C/H source files. GCC/Hightec etc. report this as `error: unexpected character <U+FEFF>` at line 1, and it silently corrupts the first `#include`/`#define` if unnoticed.

## When To Use

- A build error like `unexpected character <U+FEFF>` or `stray '\357'`/'ï»¿' appears at line 1 of a `.c`/`.h` file.
- User asks to scan a module/plugin directory for BOM-related encoding problems.
- After editing/saving files with an editor that wrote UTF-8 with BOM, before compiling.

## Detection Method (byte-level, not text-level)

Do NOT rely on text readers — BOM is often invisible. Always inspect raw bytes.

Scan all `.c`/`.h` files under a plugin directory (PowerShell):

```powershell
Get-ChildItem -Path '<plugin_dir>' -Recurse -Include *.c,*.h | ForEach-Object {
    $b = [System.IO.File]::ReadAllBytes($_.FullName)
    if ($b.Length -ge 3 -and $b[0] -eq 0xEF -and $b[1] -eq 0xBB -and $b[2] -eq 0xBF) {
        Write-Output ('BOM: ' + $_.FullName)
    }
}
```

A single UTF-8 BOM is bytes `EF BB BF`. Some files carry a DOUBLE BOM (`EF BB BF EF BB BF`), which appears as two invisible characters before the first `/`.

## Fix Method (byte-safe strip)

```powershell
$path = '<file_path>'
$b = [System.IO.File]::ReadAllBytes($path)
$n = 0
while ($b.Length -ge ($n+3) -and $b[$n] -eq 0xEF -and $b[$n+1] -eq 0xBB -and $b[$n+2] -eq 0xBF) { $n += 3 }
if ($n -gt 0) {
    [System.IO.File]::WriteAllBytes($path, $b[$n..($b.Length-1)])
    Write-Output ("Fixed ($n bytes): " + $path)
}
```

The `while` loop strips single OR double BOM(s) without touching the rest of the file.

## Verification

1. Re-check the first bytes start with `2F` (`/`) for C files, or `23` (`#`) for headers.
2. Re-run the directory scan — output should be empty.
3. Confirm file size shrank by exactly 3 bytes per BOM.
4. IDE text-read of line 1 must show `/*****...` or `#ifndef` without leading invisible chars.

## Notes

- BOM affects only the very start of a file; a BOM anywhere else is illegal UTF-8 and may be data corruption — report it.
- Recommend saving C/H sources as **UTF-8 without BOM** (or GBK) in editors to prevent recurrence.
- Always edit files with the dedicated tools (Read/Write/Edit) after byte-level repair so future edits keep the clean encoding.
- Do not use text-based tools (sed/Get-Content) to "remove" the BOM — they may re-introduce it or mangle encoding. Byte-level repair is mandatory.