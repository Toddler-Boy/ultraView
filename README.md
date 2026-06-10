<h1><img align="top" src="icons/macos15_big.png" height="45" /> ultraView</h1>
Open source C64 ultimate stream viewer deluxe.

## Features

- Best in class CRT simulation for both PAL and NTSC.
- Handles Video and Audio streams in perfect quality.
- Handles CRT & PRG uploads.
- Build in games browser.

![H.E.R.O. screenshot](/docs/screenshot_hero.jpg?raw=true "H.E.R.O. in action")

## Building

Requires CMake 4.3+ and a C++23 compiler. Run `./config_cmake.sh` then open the project in your IDE.

## CI Secrets

The following GitHub Actions secrets are required for code signing and notarization:

| Secret | Description | Where to find it |
|---|---|---|
| `TEAM_ID` | Apple Developer Team ID (10-character string) | [Apple Developer Account](https://developer.apple.com/account) > Membership details |
| `DEV_APP_ID` | Full signing identity, e.g. `Developer ID Application: Name (TEAM_ID)` | Run `security find-identity -v -p codesigning` on a Mac with the certificate installed |
| `APPLICATION` | Base64-encoded Developer ID Application `.p12` certificate | Export from Keychain Access > My Certificates, then `base64 -i Certificates.p12` |
| `P12_PASSWORD` | Password used when exporting the `.p12` file | Set during the Keychain Access export |
| `KEYCHAIN_PASSWORD` | Password for the temporary CI keychain | Any random string (only used during CI builds) |
| `APPLE_USER` | Apple ID email used for notarization | Your Apple Developer account email |
| `APPLE_PASS` | App-specific password for notarization | [Apple ID Account](https://appleid.apple.com) > Sign-In and Security > App-Specific Passwords |

**Windows (Azure Trusted Signing):**

| Secret | Description | Where to find it |
|---|---|---|
| `AZURE_TENANT_ID` | Azure AD tenant GUID | [Azure Portal](https://portal.azure.com) > Microsoft Entra ID > Overview |
| `AZURE_CLIENT_ID` | Service principal application GUID | Azure Portal > App registrations > Your app > Application (client) ID |
| `AZURE_CLIENT_SECRET` | Service principal client secret | Azure Portal > App registrations > Your app > Certificates & secrets |
| `AZURE_ENDPOINT` | Trusted Signing endpoint URL | Azure Portal > Trusted Signing account > Overview (e.g. `https://wus.codesigning.azure.net/`) |
| `AZURE_ACCOUNT_NAME` | Code Signing account name | Azure Portal > Trusted Signing account > Overview |
| `AZURE_CERT_PROFILE` | Certificate profile name | Azure Portal > Trusted Signing account > Certificate profiles |
