# Changelog

Headline changes only. Small fixes and internal work are not listed.

## 1.1.3 (2026-08-06)

- Self-healing streams: when the C64 Ultimate is set to stream to a multicast
  address or another machine, ultraView notices the silence and redirects the
  streams to itself.

## 1.1.2 (2026-08-05)

- New password field above the browser, for password-protected C64 Ultimates.
  The title bar tells you when a password is required.
- Fixed the window opening behind other applications on Windows.

## 1.1.1 (2026-08-04)

- The macOS requirement dropped from macOS 14 to macOS 11 (Big Sur).
- Numbers now use your region's digit grouping on macOS (61,165 instead of 61165).

## 1.1.0 (2026-08-03)

- ultraView is now a single portable executable. No installer: download, run, done.
- The macOS build is universal (Apple Silicon + Intel) and requires macOS 14 or newer.
- Automatic reconnection: ultraView keeps looking for the C64 Ultimate, whether
  it was turned on late or disappeared mid-session. Until a signal is found,
  the screen shows proper TV static.
- CRT presets: pick a factory look from the new drop-down or save your own.
- The CRT emulation gained additional parameters.
- The screen no longer goes to sleep while ultraView is running.
- Fixed the window position and size not being restored on macOS.
- Settings moved to a new format and start fresh. When upgrading from 1.0.x,
  you can delete the old folders:
  - Windows: `C:\ProgramData\ultraView` and `%APPDATA%\ultraView`
  - macOS: `/Library/Application Support/ultraView` and `~/Library/ultraView`

## 1.0.3

- Fixed the macOS camera and network permissions.

## 1.0.2

- Fixed the macOS overlays.

## 1.0.1

- Reboot the C64 if the last loaded cartridge was an EasyFlash.
- Lowered the Windows requirement to Windows 10 (1607).
- Switched the macOS installer from dmg to pkg.

## 1.0.0

- Initial release.
