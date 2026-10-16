#!/bin/bash
set -e

# App metadata
APP_NAME="OpenHydroQual"
APP_BUNDLE="${APP_NAME}.app"
APP_EXEC="Contents/MacOS/${APP_NAME}"
FRAMEWORKS_DIR="${APP_BUNDLE}/Contents/Frameworks"
STAGING_DIR=~/OpenHydroQual_pkg
DMG_NAME="${APP_NAME}.dmg"
DMG_PATH=~/Desktop/${DMG_NAME}

# Path to your Qt installation (adjust version if needed)
QT_PATH=~/Qt/6.10.1/macos/bin

# notarytool keychain profile holding the Apple ID + app-specific password.
# Create it once with:
#   xcrun notarytool store-credentials "OHQ-notary" \
#     --apple-id "arashmassoudieh@gmail.com" --team-id "H3BRP928NG"
NOTARY_PROFILE="${NOTARY_PROFILE:-OHQ-notary}"

echo "🧹 Cleaning previous builds..."

# A previous DMG left mounted would survive the rm below as an orphaned volume,
# so detach anything still backed by the file we are about to replace.
for dev in $(hdiutil info 2>/dev/null | awk -v p="${DMG_PATH}" '
        index($0, "image-path") && index($0, p) { found = 1; next }
        found && /^\/dev\/disk[0-9]+[[:space:]]/ { print $1; found = 0 }'); do
    echo "   detaching stale mount ${dev}"
    hdiutil detach "${dev}" -quiet || hdiutil detach "${dev}" -force || true
done

rm -rf "${STAGING_DIR}"
rm -f "${DMG_PATH}"

echo "📂 Preparing Frameworks folder..."
mkdir -p "${FRAMEWORKS_DIR}"

echo "📦 Copying non-Qt libraries..."
# Core libs
cp /opt/homebrew/opt/llvm/lib/libomp.dylib "${FRAMEWORKS_DIR}/" || true
cp /opt/homebrew/opt/lapack/lib/liblapack.3.dylib "${FRAMEWORKS_DIR}/" || true
cp /opt/homebrew/opt/lapack/lib/libblas.3.dylib "${FRAMEWORKS_DIR}/" || true
cp /opt/homebrew/opt/gsl/lib/libgsl.28.dylib "${FRAMEWORKS_DIR}/" || true
cp /opt/homebrew/opt/armadillo/lib/libarmadillo.11.dylib "${FRAMEWORKS_DIR}/" || true

# GCC runtime libs
cp /opt/homebrew/opt/gcc/lib/gcc/current/libgfortran.5.dylib "${FRAMEWORKS_DIR}/" || true
cp /opt/homebrew/opt/gcc/lib/gcc/current/libquadmath.0.dylib "${FRAMEWORKS_DIR}/" || true
cp /opt/homebrew/Cellar/gcc/12.2.0/lib/gcc/12/libgcc_s.1.1.dylib "${FRAMEWORKS_DIR}/" || true

# Homebrew ships some dylibs read-only (mode 444) and cp preserves that, which
# makes macdeployqt fail when it strips and relocates them. Make the copies
# writable so it can.
chmod u+w "${FRAMEWORKS_DIR}"/*.dylib

echo "🔧 Rewriting binary linkage (non-Qt)..."
install_name_tool -change /opt/homebrew/opt/llvm/lib/libomp.dylib \
  @executable_path/../Frameworks/libomp.dylib \
  "${APP_BUNDLE}/${APP_EXEC}" || true

install_name_tool -change /opt/homebrew/opt/lapack/lib/liblapack.3.dylib \
  @executable_path/../Frameworks/liblapack.3.dylib \
  "${APP_BUNDLE}/${APP_EXEC}" || true

install_name_tool -change /opt/homebrew/opt/lapack/lib/libblas.3.dylib \
  @executable_path/../Frameworks/libblas.3.dylib \
  "${APP_BUNDLE}/${APP_EXEC}" || true

install_name_tool -change /opt/homebrew/opt/gsl/lib/libgsl.28.dylib \
  @executable_path/../Frameworks/libgsl.28.dylib \
  "${APP_BUNDLE}/${APP_EXEC}" || true

install_name_tool -change /opt/homebrew/opt/armadillo/lib/libarmadillo.11.dylib \
  @executable_path/../Frameworks/libarmadillo.11.dylib \
  "${APP_BUNDLE}/${APP_EXEC}" || true

# Patch libgfortran to use bundled libgcc_s
if [ -f "${FRAMEWORKS_DIR}/libgfortran.5.dylib" ]; then
  install_name_tool -change \
    /opt/homebrew/Cellar/gcc/12.2.0/lib/gcc/12/libgcc_s.1.1.dylib \
    @executable_path/../Frameworks/libgcc_s.1.1.dylib \
    "${FRAMEWORKS_DIR}/libgfortran.5.dylib" || true
fi

echo "📦 Running macdeployqt..."
"${QT_PATH}/macdeployqt" "${APP_BUNDLE}" -verbose=2

echo "📂 Creating staging folder at ${STAGING_DIR}..."
mkdir -p "${STAGING_DIR}"

echo "📦 Copying ${APP_BUNDLE} into staging folder..."
cp -R "./${APP_BUNDLE}" "${STAGING_DIR}/"

echo "🔏 Signing app..."
codesign --force --deep --options runtime \
  --sign "Developer ID Application: Arash Massoudieh (H3BRP928NG)" \
  "${STAGING_DIR}/${APP_BUNDLE}"

# `hdiutil create -srcfolder` mounts its scratch volume at /Volumes/<volname>,
# and macOS refuses to write there once an app has been launched from that path
# ("could not access /Volumes/.../X.app - Operation not permitted"), which made
# this step fail intermittently. Build a read-write image, attach it at a
# mountpoint we pick, copy with ditto (which preserves xattrs and the code
# signature), then compress. Same volume name, no dependency on /Volumes.
echo "💽 Creating DMG at ${DMG_PATH}..."
DMG_TMP_DIR="$(mktemp -d)"
RW_DMG="${DMG_TMP_DIR}/rw.dmg"
DMG_MNT="${DMG_TMP_DIR}/mnt"
mkdir -p "${DMG_MNT}"

# Size from the staging folder, plus headroom for filesystem overhead.
DMG_SIZE_MB=$(( $(du -sm "${STAGING_DIR}" | awk '{print $1}') + 120 ))

hdiutil create -size "${DMG_SIZE_MB}m" -fs HFS+ -volname "${APP_NAME}" \
  -type UDIF -ov "${RW_DMG}"
hdiutil attach "${RW_DMG}" -mountpoint "${DMG_MNT}" -nobrowse
ditto "${STAGING_DIR}" "${DMG_MNT}"
hdiutil detach "${DMG_MNT}" -quiet || hdiutil detach "${DMG_MNT}" -force
hdiutil convert "${RW_DMG}" -format UDZO -o "${DMG_PATH}"
rm -rf "${DMG_TMP_DIR}"

echo "✅ DMG created at: ${DMG_PATH}"

echo "🔏 Signing DMG..."
codesign --force --deep --options runtime \
  --sign "Developer ID Application: Arash Massoudieh (H3BRP928NG)" \
  "${DMG_PATH}"

echo "📤 Submitting DMG for notarization..."
xcrun notarytool submit "${DMG_PATH}" \
  --keychain-profile "${NOTARY_PROFILE}" \
  --wait

echo "📎 Stapling notarization ticket..."
xcrun stapler staple "${DMG_PATH}"

# Gatekeeper only checks notarization on quarantined (downloaded) files, so a
# DMG that failed to notarize still opens fine locally and looks shippable.
# Verify here instead of finding out from users after it is on the website.
echo "🔍 Verifying notarization..."
xcrun stapler validate "${DMG_PATH}"
spctl -a -t open --context context:primary-signature -v "${DMG_PATH}"

echo "✅ DMG signed, notarized, stapled, and verified!"

