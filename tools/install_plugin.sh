#!/bin/sh
# Build the Release VST3 and install it for this user on macOS (no admin password, safe unattended):
#   tools/install_plugin.sh            build with the release preset, verify the signature, install
#   tools/install_plugin.sh --no-build install what build/ holds
# Then rescan in the host (Ableton Live: Preferences, Plug-Ins, Rescan). The release preset builds
# the universal binary, so Live 10.1 under Rosetta (x86_64) loads the same bundle.
set -eu
cd "$(dirname "$0")/.."
[ "$(uname)" = "Darwin" ] || { echo "macOS only (Windows: copy the .vst3 to C:\\Program Files\\Common Files\\VST3)"; exit 2; }
if [ "${1:-}" != "--no-build" ]; then
    cmake --preset release
    cmake --build --preset release
fi
bundle=$(ls -d build/*_artefacts/Release/VST3/*.vst3 2>/dev/null | head -n 1)
[ -n "$bundle" ] || { echo "no VST3 in build/ - build first"; exit 1; }
# macOS will not load a bundle whose signature is broken (Apple Silicon, and Rosetta): check it
codesign --verify --deep --strict "$bundle" || { echo "signature invalid: $bundle"; exit 1; }
lipo -archs "$bundle/Contents/MacOS/"* 2>/dev/null | sed 's/^/architectures: /'
dest="$HOME/Library/Audio/Plug-Ins/VST3"
mkdir -p "$dest"
rm -rf "$dest/$(basename "$bundle")"
cp -R "$bundle" "$dest/"
echo "installed: $dest/$(basename "$bundle")"
echo "now rescan in the host (Ableton Live: Preferences, Plug-Ins, Rescan)"
