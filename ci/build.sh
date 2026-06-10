#!/bin/bash -e

OS_NAME=$(uname)

ROOT=$(cd "$(dirname "$0")/.."; pwd)
cd "$ROOT"
echo "$ROOT"

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
  #
  if [ -n "${APPLICATION:-}" ] && [ "${RUNNER_ENVIRONMENT:-}" = "github-hosted" ]; then
    KC_PASS="${KEYCHAIN_PASSWORD:-nr4aGPyz}"
    P12_PASS="${P12_PASSWORD:-aym9PKWB}"

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

  # Codesign the app
  if [ -n "${APPLICATION:-}" ]; then
    codesign -s "$DEV_APP_ID" --options=runtime --timestamp --force --deep -v "$APP_PATH"
  else
    echo "Skipping codesign — APPLICATION secret not set"
  fi

  # Create DMG with app on left, Applications symlink on right, arrow background
  DMG_STAGE="$ROOT/ci/bin/dmg_contents"
  rm -rf "$DMG_STAGE"
  mkdir -p "$DMG_STAGE"
  cp -R "$APP_PATH" "$DMG_STAGE/"
  ln -s /Applications "$DMG_STAGE/Applications"

  create-dmg \
    --volname "ultraView" \
    --background "$ROOT/ci/dmg_background.png" \
    --window-pos 200 120 \
    --window-size 660 400 \
    --icon-size 80 \
    --icon "ultraView.app" 180 200 \
    --app-drop-link 480 200 \
    --no-internet-enable \
    "$ROOT/ci/bin/ultraView.dmg" \
    "$DMG_STAGE/" || true

  rm -rf "$DMG_STAGE"

  # Sign the DMG
  if [ -n "${APPLICATION:-}" ]; then
    codesign -s "$DEV_APP_ID" --options=runtime --timestamp --force -v "$ROOT/ci/bin/ultraView.dmg"
  fi

  # Notarize the DMG
  if [ -n "${APPLE_USER:-}" ] && [ -n "${APPLE_PASS:-}" ]; then
    SUBMISSION_OUTPUT=$(xcrun notarytool submit --verbose --apple-id "$APPLE_USER" --password "$APPLE_PASS" --team-id "$TEAM_ID" --wait --timeout 30m "$ROOT/ci/bin/ultraView.dmg" 2>&1) || NOTARY_FAILED=1
    echo "$SUBMISSION_OUTPUT"
    SUBMISSION_ID=$(echo "$SUBMISSION_OUTPUT" | awk "/^  id:/ { print \$2; exit }")
    if [ "${NOTARY_FAILED:-0}" = "1" ] && [ -n "$SUBMISSION_ID" ]; then
      echo "Notarization failed — fetching log for $SUBMISSION_ID"
      xcrun notarytool log "$SUBMISSION_ID" --apple-id "$APPLE_USER" --password "$APPLE_PASS" --team-id "$TEAM_ID" || true
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
  cmake --preset ninja-clang
  cmake --build --preset ninja-clang --config Release --parallel
  cp "$ROOT/Builds/ninja-clang/ultraView_artefacts/Release/ultraView" "$ROOT/ci/bin/"
fi

# Build Win version
if [[ "$OS_NAME" == MINGW* ]] || [[ "$OS_NAME" == MSYS* ]] || [[ "$OS_NAME" == CYGWIN* ]]; then
  cd "$ROOT"
  cmake --preset vs
  cmake --build --preset vs --config Release --parallel
  cp "$ROOT/Builds/vs/ultraView_artefacts/Release/ultraView.exe" "$ROOT/ci/bin/"
fi
