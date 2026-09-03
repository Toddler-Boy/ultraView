#!/bin/bash
source "$(dirname "$0")/../Source/ultra-shared/scripts/preamble.sh"

OS_NAME=$(uname)
ROOT=$(pwd)

ver=$(tr -d ' \r\n' < "$ROOT/VERSION")
echo "Building ultraView $ver"

BRANCH=${GITHUB_REF##*/}
echo "$BRANCH"

cd "$ROOT/ci"
rm -rf bin
mkdir -p bin

# Pre-seeded configure-check results harvested from a previous CMakeCache.txt.
# Skips the slow try_compile probes (mainly libarchive's) on the fresh CI
# Builds folder; a missing seed file just means a full probe run
seed_args() {
  SEED_ARGS=()
  if [ -f "$ROOT/Tools/configure-seed-$1.cmake" ]; then
    SEED_ARGS=(-C "$ROOT/Tools/configure-seed-$1.cmake")
  fi
}

# Build mac version
if [ "$OS_NAME" = "Darwin" ]; then
  seed_args xcode

  # Branch-push CI runs are compile checks only: arm64, unsigned, no dmg.
  # RELEASE=1 (tag and manual workflow runs) does the full packaging
  if [ "${RELEASE:-}" != "1" ]; then
    cd "$ROOT"
    cmake --preset xcode "${SEED_ARGS[@]}"
    cmake --build --preset xcode --config Release --parallel
    exit 0
  fi

  if [ -z "${TEAM_ID:-}" ] || [ -z "${DEV_APP_ID:-}" ]; then
    echo "Skipping signing — TEAM_ID / DEV_APP_ID secrets not set"
  fi

  #
  # Bootstrap a temp keychain from base64-encoded p12 secrets when running in CI.
  # APPLICATION = base64 of Developer ID Application .p12
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
  # Releases are universal; compile checks above stay arm-only for speed
  cmake --preset xcode "${SEED_ARGS[@]}" -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
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

  # Stage the drag-install image: the app plus an Applications link, with the
  # committed background picture tucked into a hidden .background folder
  # (created here at build time only — hidden folders stay out of the repo)
  DMG_STAGE="$ROOT/ci/bin/dmg_root"
  rm -rf "$DMG_STAGE"
  mkdir -p "$DMG_STAGE"
  cp -R "$APP_PATH" "$DMG_STAGE/"
  ln -s /Applications "$DMG_STAGE/Applications"

  mkdir "$DMG_STAGE/.background"
  cp "$ROOT/Installer/macOS/background@2x.png" "$DMG_STAGE/.background/background.png"
  # The bitmap is 1200x800 and must cover the whole 600x400pt window — any
  # uncovered area takes Finder's default background, which goes dark gray in
  # dark mode. 144 dpi is what tells Finder it's a @2x image, otherwise it
  # renders at double size
  sips -s dpiWidth 144 -s dpiHeight 144 "$DMG_STAGE/.background/background.png" > /dev/null

  # Finder layout (background, icon positions) lives in the volume's .DS_Store,
  # which only Finder itself can write — so build read-write first, style the
  # mounted volume via AppleScript, then compress. HFS+ because APFS images
  # don't mount on older macOS
  RW_DMG="$ROOT/ci/bin/ultraView_rw.dmg"
  hdiutil create -volname "ultraView" -srcfolder "$DMG_STAGE" -ov -format UDRW -fs HFS+ "$RW_DMG"
  MOUNT_DEV=$(hdiutil attach -readwrite -noverify -noautoopen "$RW_DMG" | awk 'NR==1{print $1}')

  # Finder can be flaky right after mount, especially on CI runners
  STYLED=0
  for attempt in 1 2 3; do
    if osascript <<'OSAEOF'
tell application "Finder"
    tell disk "ultraView"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        -- 428 = 400pt of content under the 28pt title bar (bounds include it)
        set the bounds of container window to {200, 120, 800, 548}
        set viewOptions to the icon view options of container window
        set arrangement of viewOptions to not arranged
        set icon size of viewOptions to 100
        set background picture of viewOptions to file ".background:background.png"
        set position of item "ultraView.app" of container window to {150, 200}
        set position of item "Applications" of container window to {450, 200}
        close
        open
        update without registering applications
        delay 2
        close
    end tell
end tell
OSAEOF
    then STYLED=1; break; fi
    echo "Finder styling attempt $attempt failed — retrying"
    sleep 5
  done
  if [ "$STYLED" != "1" ]; then
    echo "Could not style the DMG Finder window"
    exit 1
  fi

  sync
  for attempt in 1 2 3 4 5; do
    hdiutil detach "$MOUNT_DEV" && break || sleep 2
  done

  hdiutil convert "$RW_DMG" -format UDZO -o "$ROOT/ci/bin/ultraView.dmg" -ov
  rm -f "$RW_DMG"
  rm -rf "$DMG_STAGE"

  # Sign the dmg
  if [ -n "${APPLICATION:-}" ]; then
    codesign -s "$DEV_APP_ID" --timestamp "$ROOT/ci/bin/ultraView.dmg"
  else
    echo "Skipping dmg signing — APPLICATION secret not set"
  fi

  # Notarize the dmg — releases only, so every dev push doesn't cost an Apple
  # submission (NOTARIZE comes from the workflow on v* tags)
  if [ "${NOTARIZE:-}" != "1" ]; then
    echo "Skipping notarization — not a release tag"
  elif [ -n "${APPLE_USER:-}" ] && [ -n "${APPLE_PASS:-}" ]; then
    # First submissions from a new team can sit in Apple's queue for well over
    # 30 minutes, so give notarytool a generous wait budget
    SUBMISSION_OUTPUT=$(xcrun notarytool submit --verbose --apple-id "$APPLE_USER" --password "$APPLE_PASS" --team-id "$TEAM_ID" --wait --timeout 100m "$ROOT/ci/bin/ultraView.dmg" 2>&1) || NOTARY_FAILED=1
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
    xcrun stapler staple "$ROOT/ci/bin/ultraView.dmg"
  else
    echo "Skipping notarization — APPLE_USER / APPLE_PASS not set"
  fi
fi

# Build linux version
if [ "$OS_NAME" = "Linux" ]; then
  cd "$ROOT"
  seed_args ninja-clang
  cmake --preset ninja-clang "${SEED_ARGS[@]}"
  cmake --build --preset ninja-clang --config Release --parallel

  cd "$ROOT/Builds/ninja-clang"
  cpack -G DEB -C Release

  cp "$ROOT/Builds/ninja-clang/"*.deb "$ROOT/ci/bin/"
fi

# Build Win version
if [[ "$OS_NAME" == MINGW* ]] || [[ "$OS_NAME" == MSYS* ]] || [[ "$OS_NAME" == CYGWIN* ]]; then
  cd "$ROOT"
  seed_args vs
  cmake --preset vs "${SEED_ARGS[@]}"
  cmake --build --preset vs --config Release --parallel

  STAGE="$ROOT/ci/bin/stage"
  rm -rf "$STAGE"
  mkdir -p "$STAGE"
  cp "$ROOT/Builds/vs/ultraView_artefacts/Release/ultraView.exe" "$STAGE/"

  # Data.pak: the ultra-shared Data tree packs first, so app entries would win
  # a name clash (matching the naked-mode precedence); it is tiny and
  # text-only, one weakest-deflate pass
  (
    cd "$ROOT/Source/ultra-shared/Data"
    find . -mindepth 1 \( -name '!src' -o -name '.*' \) -prune -o -type f ! -name Thumbs.db -print | sed 's|^\./||' > "$STAGE/_pakshared.txt"
    7z a -tzip -mx=1 -mcu=on "$STAGE/Data.pak" @"$STAGE/_pakshared.txt" > /dev/null
  )

  # App tree: images carry their own compression and get stored, the rest gets
  # the weakest deflate
  (
    cd "$ROOT/Data"
    find . -mindepth 1 \( -name '!src' -o -name '.*' \) -prune -o -type f ! -name Thumbs.db -print | sed 's|^\./||' > "$STAGE/_pakfiles.txt"
    grep -iE '\.(png|jpg)$' "$STAGE/_pakfiles.txt" > "$STAGE/_pakimages.txt"
    grep -ivE '\.(png|jpg)$' "$STAGE/_pakfiles.txt" > "$STAGE/_pakrest.txt"
    # -mcu=on: always UTF-8 entry names, the reader assumes them
    7z a -tzip -mx=0 -mcu=on "$STAGE/Data.pak" @"$STAGE/_pakimages.txt" > /dev/null
    7z a -tzip -mx=1 -mcu=on "$STAGE/Data.pak" @"$STAGE/_pakrest.txt" > /dev/null
  )

  # Completeness: every listed file made it into the pak. A name clash between
  # the two trees would collapse into one entry and trip the count
  expected=$(cat "$STAGE/_pakshared.txt" "$STAGE/_pakfiles.txt" | wc -l)
  actual=$(7z l -ba "$STAGE/Data.pak" | wc -l)
  if [ "$expected" -ne "$actual" ]; then
    echo "Data.pak entry count mismatch: $actual packed, $expected on disk"
    exit 1
  fi
  rm "$STAGE"/_pak*.txt

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

  # The signed self-contained exe is the whole deliverable; no version in the
  # name — downloads and the self-updater always see a plain ultraView.exe
  cp "$STAGE/ultraView.exe" "$ROOT/ci/bin/"
fi
