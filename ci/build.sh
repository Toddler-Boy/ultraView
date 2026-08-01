#!/bin/bash -e
handle_error() {
    echo "An error occurred on line $1"
    read -p "Press enter to continue"
    exit 1
}

trap 'handle_error $LINENO' ERR

OS_NAME=$(uname)

ROOT=$(cd "$(dirname "$0")/.."; pwd)
cd "$ROOT"

ver=$(<"$ROOT/VERSION")
ver=${ver%%*( )}
echo "Bulding installer for ultraView $ver"

BRANCH=${GITHUB_REF##*/}
echo "$BRANCH"

cd "$ROOT/ci"
rm -rf bin
mkdir -p bin

# Build mac version
if [ "$OS_NAME" = "Darwin" ]; then
  if [ -z "${TEAM_ID:-}" ] || [ -z "${DEV_APP_ID:-}" ]; then
    echo "Skipping signing — TEAM_ID / DEV_APP_ID secrets not set"
  fi

  #
  # Bootstrap a temp keychain from base64-encoded p12 secrets when running in CI.
  # APPLICATION = base64 of Developer ID Application .p12
  # INSTALLER   = base64 of Developer ID Installer .p12
  #
  if [ -n "${APPLICATION:-}" ] && [ "${RUNNER_ENVIRONMENT:-}" = "github-hosted" ]; then
    KC_PASS="$KEYCHAIN_PASSWORD"
    P12_PASS="$P12_PASSWORD"

    security create-keychain -p "$KC_PASS" Keys.keychain || true

    echo "$APPLICATION" | base64 -D -o /tmp/Application.p12
    security import /tmp/Application.p12 -t agg -k Keys.keychain -P "$P12_PASS" -A -T /usr/bin/codesign

    security list-keychains -s Keys.keychain
    security default-keychain -s Keys.keychain
    security unlock-keychain -p "$KC_PASS" Keys.keychain
    security set-keychain-settings -l -u -t 13600 Keys.keychain
    security set-key-partition-list -S apple-tool:,apple: -s -k "$KC_PASS" Keys.keychain
  fi

  cd "$ROOT"
  cmake --preset xcode
  cmake --build --preset xcode --config Release --parallel

  APP_PATH="$ROOT/Builds/xcode/ultraView_artefacts/Release/ultraView.app"

  # Generate entitlements for Hardened Runtime (Camera and Network Client)
  ENTITLEMENTS="$ROOT/ci/bin/entitlements.plist"
  cat > "$ENTITLEMENTS" <<ENTEOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <!-- Allows app to use the webcam -->
    <key>com.apple.security.device.camera</key>
    <true/>
    <!-- Allows app to send outgoing REST requests -->
    <key>com.apple.security.network.client</key>
    <true/>
</dict>
</plist>
ENTEOF

  # Codesign the app (the whole data set lives inside the bundle and gets
  # sealed with it)
  if [ -n "${APPLICATION:-}" ]; then
    codesign -s "$DEV_APP_ID" --options=runtime --timestamp --entitlements "$ENTITLEMENTS" --force --deep -v "$APP_PATH"
  else
    echo "Skipping codesign — APPLICATION secret not set"
  fi

  # Stage the drag-install image: the app plus an Applications link. Optional
  # Finder styling (background bitmap, icon layout) comes from a hand-arranged
  # window committed as Installer/macOS/dmg-style
  DMG_STAGE="$ROOT/ci/bin/dmg_root"
  rm -rf "$DMG_STAGE"
  mkdir -p "$DMG_STAGE"
  cp -R "$APP_PATH" "$DMG_STAGE/"
  ln -s /Applications "$DMG_STAGE/Applications"

  if [ -d "$ROOT/Installer/macOS/dmg-style" ]; then
    cp -R "$ROOT/Installer/macOS/dmg-style/." "$DMG_STAGE/"
  fi

  hdiutil create -volname "ultraView" -srcfolder "$DMG_STAGE" -ov -format UDZO "$ROOT/ci/bin/ultraView_$ver.dmg"
  rm -rf "$DMG_STAGE"

  # Sign the dmg
  if [ -n "${APPLICATION:-}" ]; then
    codesign -s "$DEV_APP_ID" --timestamp "$ROOT/ci/bin/ultraView_$ver.dmg"
  else
    echo "Skipping dmg signing — APPLICATION secret not set"
  fi

  # Notarize the dmg
  if [ -n "${APPLE_USER:-}" ] && [ -n "${APPLE_PASS:-}" ]; then
    # First submissions from a new team can sit in Apple's queue for well over
    # 30 minutes, so give notarytool a generous wait budget
    SUBMISSION_OUTPUT=$(xcrun notarytool submit --verbose --apple-id "$APPLE_USER" --password "$APPLE_PASS" --team-id "$TEAM_ID" --wait --timeout 100m "$ROOT/ci/bin/ultraView_$ver.dmg" 2>&1) || NOTARY_FAILED=1
    echo "$SUBMISSION_OUTPUT"
    SUBMISSION_ID=$(echo "$SUBMISSION_OUTPUT" | awk "/^  id:/ { if (!id) id = \$2 } END { print id }")
    if [ "${NOTARY_FAILED:-0}" = "1" ]; then
      # No submission id = rejected before upload (bad credentials/team id),
      # the notarytool output above is the whole story
      if [ -n "$SUBMISSION_ID" ]; then
        echo "Notarization failed — fetching log for $SUBMISSION_ID"
        xcrun notarytool log "$SUBMISSION_ID" --apple-id "$APPLE_USER" --password "$APPLE_PASS" --team-id "$TEAM_ID" || true
      fi
      exit 1
    fi
    xcrun stapler staple "$ROOT/ci/bin/ultraView_$ver.dmg"
  else
    echo "Skipping notarization — APPLE_USER / APPLE_PASS not set"
  fi
fi

# Build linux version
if [ "$OS_NAME" = "Linux" ]; then
  cd "$ROOT"
  cmake --preset ninja-clang
  cmake --build --preset ninja-clang --config Release --parallel

  cd "$ROOT/Builds/ninja-clang"
  cpack -G DEB -C Release

  cp "$ROOT/Builds/ninja-clang/"*.deb "$ROOT/ci/bin/"
fi

# Build Win version
if [[ "$OS_NAME" == MINGW* ]] || [[ "$OS_NAME" == MSYS* ]] || [[ "$OS_NAME" == CYGWIN* ]]; then
  cd "$ROOT"
  cmake --preset vs
  cmake --build --preset vs --config Release --parallel

  STAGE="$ROOT/Installer/win/stage"
  rm -rf "$STAGE"
  mkdir -p "$STAGE"
  cp "$ROOT/Builds/vs/ultraView_artefacts/Release/ultraView.exe" "$STAGE/"

  # Data.pak: images carry their own compression and get stored, the rest gets
  # the weakest deflate
  (
    cd "$ROOT/Data"
    find . -mindepth 1 \( -name '!src' -o -name '.*' \) -prune -o -type f ! -name Thumbs.db -print | sed 's|^\./||' > "$STAGE/_pakfiles.txt"
    grep -iE '\.(png|jpg)$' "$STAGE/_pakfiles.txt" > "$STAGE/_pakimages.txt"
    grep -ivE '\.(png|jpg)$' "$STAGE/_pakfiles.txt" > "$STAGE/_pakrest.txt"
    # -mcu=on: always UTF-8 entry names, the reader assumes them
    7z a -tzip -mx=0 -mcu=on "$STAGE/Data.pak" @"$STAGE/_pakimages.txt" > /dev/null
    7z a -tzip -mx=1 -mcu=on "$STAGE/Data.pak" @"$STAGE/_pakrest.txt" > /dev/null
    rm "$STAGE"/_pak*.txt
  )

  # The pak rides appended to the exe (zip is end-anchored, so the file stays
  # both a valid PE and a valid zip), making the exe fully self-contained.
  # Order matters: signing afterwards seals code and data under one signature
  cat "$STAGE/Data.pak" >> "$STAGE/ultraView.exe"
  rm "$STAGE/Data.pak"

  # Azure Trusted Signing
  uuid_re='^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$'
  WIN_SIGN=0
  if [ -n "${AZURE_TENANT_ID:-}" ] || [ -n "${AZURE_CLIENT_ID:-}" ] || [ -n "${AZURE_CLIENT_SECRET:-}" ]; then
    if   [[ ! "${AZURE_TENANT_ID:-}" =~ $uuid_re ]]; then
      echo "ERROR: AZURE_TENANT_ID is not a valid GUID — set the secret on the repo or unset all three to skip signing."; exit 1
    elif [[ ! "${AZURE_CLIENT_ID:-}" =~ $uuid_re ]]; then
      echo "ERROR: AZURE_CLIENT_ID is not a valid GUID — set the secret on the repo or unset all three to skip signing."; exit 1
    elif [ -z "${AZURE_CLIENT_SECRET:-}" ]; then
      echo "ERROR: AZURE_CLIENT_SECRET is empty — set the secret on the repo or unset all three to skip signing."; exit 1
    else
      WIN_SIGN=1
    fi
  fi

  if [ "$WIN_SIGN" = "1" ]; then
    SIGNTOOL=$(ls -1 "/c/Program Files (x86)/Windows Kits/10/bin/"*/x64/signtool.exe 2>/dev/null | sort | tail -1)
    if [ -z "$SIGNTOOL" ]; then
      echo "signtool.exe not found in Windows Kits"; exit 1
    fi

    TOOLS_DIR="$STAGE/_signingtools"
    mkdir -p "$TOOLS_DIR"
    nuget install Microsoft.Trusted.Signing.Client -Version 1.0.86 \
                  -OutputDirectory "$TOOLS_DIR" -ExcludeVersion -NonInteractive
    DLIB="$TOOLS_DIR/Microsoft.Trusted.Signing.Client/bin/x64/Azure.CodeSigning.Dlib.dll"
    METADATA="$STAGE/_metadata.json"
    cat > "$METADATA" <<METAEOF
{
  "Endpoint": "$AZURE_ENDPOINT",
  "CodeSigningAccountName": "$AZURE_ACCOUNT_NAME",
  "CertificateProfileName": "$AZURE_CERT_PROFILE"
}
METAEOF

    sign_file () {
      "$SIGNTOOL" sign -v -fd SHA256 \
        -tr "http://timestamp.acs.microsoft.com" -td SHA256 \
        -dlib "$DLIB" -dmdf "$METADATA" "$1"
    }

    sign_file "$STAGE/ultraView.exe"
  else
    echo "Skipping Windows binary signing — Azure secrets not set"
  fi

  # Build installer with InnoSetup
  cd "$ROOT/Installer/win"
  ISCC="/c/Program Files (x86)/Inno Setup 6/ISCC.exe"
  if [ ! -f "$ISCC" ]; then
    ISCC="/c/Program Files/Inno Setup 6/ISCC.exe"
  fi
  "$ISCC" "$ROOT/Installer/win/ultraView.iss"

  EXE_OUT="$ROOT/Installer/win/bin/ultraView_$ver.exe"

  # Sign the installer
  if [ "$WIN_SIGN" = "1" ]; then
    sign_file "$EXE_OUT"
  fi

  cp "$EXE_OUT" "$ROOT/ci/bin/"

  # Portable variant: the same signed self-contained exe in a store-only zip
  # (the zip is just transport — browsers dislike naked exe downloads)
  ZIP_ROOT="$ROOT/ci/bin/zip_root"
  rm -rf "$ZIP_ROOT"
  mkdir -p "$ZIP_ROOT/ultraView"
  cp "$STAGE/ultraView.exe" "$ZIP_ROOT/ultraView/"

  ZIP_OUT="$ROOT/ci/bin/ultraView_${ver}_portable.zip"
  if command -v 7z > /dev/null 2>&1; then
    (cd "$ZIP_ROOT" && 7z a -tzip -mx=0 "$ZIP_OUT" ultraView)
  else
    powershell.exe -NoProfile -Command "Compress-Archive -Path '$(cygpath -w "$ZIP_ROOT/ultraView")' -DestinationPath '$(cygpath -w "$ZIP_OUT")' -CompressionLevel NoCompression"
  fi
  rm -rf "$ZIP_ROOT"
fi
