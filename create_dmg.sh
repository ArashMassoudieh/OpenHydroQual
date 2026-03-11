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

echo "🧹 Cleaning previous builds..."
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

echo "💽 Creating DMG at ${DMG_PATH}..."
hdiutil create -volname "${APP_NAME}" \
  -srcfolder "${STAGING_DIR}" \
  -ov -format UDZO "${DMG_PATH}"

echo "✅ DMG created at: ${DMG_PATH}"

echo "🔏 Signing DMG..."
codesign --force --deep --options runtime \
  --sign "Developer ID Application: Arash Massoudieh (H3BRP928NG)" \
  "${DMG_PATH}"

echo "📤 Submitting DMG for notarization..."
xcrun notarytool submit "${DMG_PATH}" \
  --apple-id "arashmassoudieh@gmail.com" \
  --team-id "H3BRP928NG" \
  --password "wzma-hwfk-cexx-mpdh" \
  --wait

echo "📎 Stapling notarization ticket..."
xcrun stapler staple "${DMG_PATH}"

echo "✅ DMG signed, notarized, and stapled!"

