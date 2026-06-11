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
  # INSTALLER   = base64 of Developer ID Installer .p12
  #
  if [ -n "${APPLICATION:-}" ] && [ "${RUNNER_ENVIRONMENT:-}" = "github-hosted" ]; then
    KC_PASS="$KEYCHAIN_PASSWORD"
    P12_PASS="$P12_PASSWORD"

    security create-keychain -p "$KC_PASS" Keys.keychain || true

    echo "$APPLICATION" | base64 -D -o /tmp/Application.p12
    security import /tmp/Application.p12 -t agg -k Keys.keychain -P "$P12_PASS" -A -T /usr/bin/codesign

    echo "$INSTALLER" | base64 -D -o /tmp/Installer.p12
    security import /tmp/Installer.p12 -t agg -k Keys.keychain -P "$P12_PASS" -A -T /usr/bin/productsign

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

  # Stage Data for pkg
  PKG_STAGE="$ROOT/ci/bin/pkg_root"
  rm -rf "$PKG_STAGE"
  mkdir -p "$PKG_STAGE/Applications"
  mkdir -p "$PKG_STAGE/Library/Application Support/ultraView"
  cp -R "$APP_PATH" "$PKG_STAGE/Applications/"
  rsync -a --exclude='!src' "$ROOT/Data/" "$PKG_STAGE/Library/Application Support/ultraView/"

  # Build component pkg
  pkgbuild \
    --root "$PKG_STAGE" \
    --identifier "com.refx.ultraView" \
    --version "1.0" \
    "$ROOT/ci/bin/ultraView-component.pkg"

  # Build product pkg with fixed title
  cat > "$ROOT/ci/bin/distribution.xml" <<DISTEOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>ultraView</title>
    <pkg-ref id="com.refx.ultraView"/>
    <options customize="never" require-scripts="false"/>
    <choices-outline>
        <line choice="default">
            <line choice="com.refx.ultraView"/>
        </line>
    </choices-outline>
    <choice id="default"/>
    <choice id="com.refx.ultraView" visible="false">
        <pkg-ref id="com.refx.ultraView"/>
    </choice>
    <pkg-ref id="com.refx.ultraView" version="1.0">ultraView-component.pkg</pkg-ref>
</installer-gui-script>
DISTEOF

  productbuild \
    --distribution "$ROOT/ci/bin/distribution.xml" \
    --package-path "$ROOT/ci/bin" \
    "$ROOT/ci/bin/ultraView-unsigned.pkg"

  rm "$ROOT/ci/bin/ultraView-component.pkg" "$ROOT/ci/bin/distribution.xml"

  # Sign the pkg
  if [ -n "${INSTALLER:-}" ]; then
    productsign --sign "$DEV_INST_ID" "$ROOT/ci/bin/ultraView-unsigned.pkg" "$ROOT/ci/bin/ultraView.pkg"
    rm "$ROOT/ci/bin/ultraView-unsigned.pkg"
  else
    mv "$ROOT/ci/bin/ultraView-unsigned.pkg" "$ROOT/ci/bin/ultraView.pkg"
    echo "Skipping pkg signing — INSTALLER secret not set"
  fi

  rm -rf "$PKG_STAGE"

  # Notarize the pkg
  if [ -n "${APPLE_USER:-}" ] && [ -n "${APPLE_PASS:-}" ]; then
    SUBMISSION_OUTPUT=$(xcrun notarytool submit --verbose --apple-id "$APPLE_USER" --password "$APPLE_PASS" --team-id "$TEAM_ID" --wait --timeout 30m "$ROOT/ci/bin/ultraView.pkg" 2>&1) || NOTARY_FAILED=1
    echo "$SUBMISSION_OUTPUT"
    SUBMISSION_ID=$(echo "$SUBMISSION_OUTPUT" | awk "/^  id:/ { print \$2; exit }")
    if [ "${NOTARY_FAILED:-0}" = "1" ] && [ -n "$SUBMISSION_ID" ]; then
      echo "Notarization failed — fetching log for $SUBMISSION_ID"
      xcrun notarytool log "$SUBMISSION_ID" --apple-id "$APPLE_USER" --password "$APPLE_PASS" --team-id "$TEAM_ID" || true
      exit 1
    fi
    xcrun stapler staple "$ROOT/ci/bin/ultraView.pkg"
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
  cp -R "$ROOT/Data" "$STAGE/Data"

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

  EXE_OUT="$ROOT/Installer/win/bin/ultraView.exe"

  # Sign the installer
  if [ "$WIN_SIGN" = "1" ]; then
    sign_file "$EXE_OUT"
  fi

  cp "$EXE_OUT" "$ROOT/ci/bin/"
fi
