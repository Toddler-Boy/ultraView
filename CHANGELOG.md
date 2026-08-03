# Changelog

Headline changes only. Small fixes and internal work are not listed.

## 1.1.0 (unreleased)

- ultraView is now a single portable executable. No installer: download, run, done.
- The macOS build is universal (Apple Silicon + Intel).
- Automatic reconnection: ultraView keeps looking for the C64 Ultimate, whether
  it was turned on late or disappeared mid-session. Until a signal is found,
  the screen shows proper TV static.
- CRT presets: pick a factory look from the new drop-down or save your own.
- The CRT emulation gained additional parameters.
- The screen no longer goes to sleep while ultraView is running.
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
