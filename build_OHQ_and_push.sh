#!/bin/bash

set -e  # Exit on error

# === Settings ===
PACKAGE_NAME="openhydroqual"
VERSION="2.0.6"
ARCH="amd64"
VERSIONED_DEB_FILE="${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"
FIXED_NAME="openhydroqual_amd64.deb"
RELEASE_TAG="v$VERSION"
GITHUB_REPO="ArashMassoudieh/OpenHydroQual"

PRO_FILE="./OpenHydroQual/OpenHydroQual.pro"
BUILD_DIR="./OpenHydroQual"
STAGING_DIR="./OpenHydroQual-deb"
BIN_PATH="${STAGING_DIR}/opt/OpenHydroQual/bin/Release"
EXECUTABLE="${BUILD_DIR}/OpenHydroQual"

# === Step 1: Build the Qt application ===
# qmake writes the Makefile into the current directory, so run it from inside
# the source tree rather than from the parent.
echo "🔧 Running qmake6..."
( cd "$BUILD_DIR" && qmake6 "$(basename "$PRO_FILE")" CONFIG+=release )

echo "⚙️ Running make..."
make -C "$BUILD_DIR" -j$(nproc)

# === Step 2: Prepare staging directory ===
echo "🧹 Cleaning old files from staging directory..."
rm -rf "${STAGING_DIR}/opt/OpenHydroQual/bin"
rm -rf "${STAGING_DIR}/opt/OpenHydroQual/resources"
rm -rf "${STAGING_DIR}/opt/resources"
rm -rf "${STAGING_DIR}/usr/local/openhydroqual"

echo "📦 Copying binary to package staging directory..."
mkdir -p "$BIN_PATH"
cp "$EXECUTABLE" "$BIN_PATH"

echo "📦 Copying resource files..."
RESOURCES_PATH="${STAGING_DIR}/opt/OpenHydroQual/resources"
cp -r "${BUILD_DIR}/resources" "${STAGING_DIR}/opt/OpenHydroQual/"

# === Step 2b: Update version in DEBIAN/control ===
echo "📝 Updating version in DEBIAN/control..."
sed -i "s/^Version:.*/Version: $VERSION/" "${STAGING_DIR}/DEBIAN/control"

# === Step 3: Build .deb package ===
echo "📦 Building .deb package..."
dpkg-deb --build "$STAGING_DIR" "$VERSIONED_DEB_FILE"

# === Step 4: Rename to fixed name for latest GitHub download compatibility ===
mv -f "$VERSIONED_DEB_FILE" "$FIXED_NAME"
echo "✅ Renamed to $FIXED_NAME for predictable downloading."

# === Step 5: Upload to GitHub release ===
echo "🚀 Uploading to GitHub release: $RELEASE_TAG..."

if ! gh release view "$RELEASE_TAG" --repo "$GITHUB_REPO" > /dev/null 2>&1; then
    echo "📢 Creating new release $RELEASE_TAG..."
    gh release create "$RELEASE_TAG" "$FIXED_NAME" \
        --repo "$GITHUB_REPO" \
        --title "OpenHydroQual $VERSION" \
        --notes "Release of OpenHydroQual version $VERSION"
else
    echo "📤 Uploading .deb to existing release..."
    gh release upload "$RELEASE_TAG" "$FIXED_NAME" \
        --repo "$GITHUB_REPO" \
        --clobber
fi

echo "🎉 Success: $FIXED_NAME uploaded to GitHub release $RELEASE_TAG!"
