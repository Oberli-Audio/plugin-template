#!/bin/sh
# Build the Release VST3 and install it to the folder for all users on macOS, where the release zip
# goes too (ship-bob locations.md); asks for the password only when that folder is not writable:
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
name=$(basename "$bundle")
dest="/Library/Audio/Plug-Ins/VST3"
sudo_=""
[ -w "$dest" ] || sudo_="sudo"
$sudo_ mkdir -p "$dest"
$sudo_ rm -rf "$dest/$name"
$sudo_ cp -R "$bundle" "$dest/"
echo "installed: $dest/$name"
# one copy only: a per-user copy would leave the host to pick one
if [ -e "$HOME/Library/Audio/Plug-Ins/VST3/$name" ]; then
    rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/$name"
    echo "removed the per-user copy: ~/Library/Audio/Plug-Ins/VST3/$name"
fi
echo "now rescan in the host (Ableton Live: Preferences, Plug-Ins, Rescan)"
