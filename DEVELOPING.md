# Developing {{Product}}

For the friends' side (install, update, feedback) see README.md. This file is the developer's:
build commands, the harness, the tests, the validators, the release.

## Requirements

- CMake 3.25+, Ninja (`brew install ninja`), Xcode Command Line Tools (macOS) or Visual Studio
  2022 (Windows). JUCE is fetched by CMake at the pinned commit; nothing to install.
- Python 3.9+ with `numpy` and `scipy` for the test suite and the measurement scripts
  (`pip3 install --user numpy scipy`). Without them CMake registers no tests.
- The machine file `~/.config/oberli/tools.toml` (`python3 scripts/dsp-kit/toolchain.py init`,
  then fill in the paths): Homebrew LLVM for the RTSan build, Faust, pluginval, the hosts.

## Build

```bash
cmake --preset release && cmake --build --preset release   # universal Release: VST3, Standalone, harness
cmake --preset debug && cmake --build --preset debug       # a debugger only (about 6x the audio-thread time)
cmake --preset asan && cmake --build --preset asan         # ASan + UBSan (release gate)
eval "$(python3 scripts/dsp-kit/toolchain.py env)" && cmake --preset rtsan && cmake --build --preset rtsan   # RealtimeSanitizer
```

Bundles: `build/{{ProductId}}_artefacts/Release/VST3/{{Product}}.vst3`, Standalone beside it, each
with its dSYM. Install for a local session on the Mac: `tools/install_plugin.sh` (builds the release
preset, verifies the signature, copies the `.vst3` to `~/Library/Audio/Plug-Ins/VST3/`, per-user, no
password; `--no-build` installs what `build/` holds), then rescan in Ableton Live. Windows: copy the
`.vst3` to `C:\Program Files\Common Files\VST3\`. `ctest` checks on macOS that every release bundle's
signature verifies and its dSYM symbolicates every slice (`release_bundles`).

## The harness

```bash
H=./build/{{ProductId}}Harness_artefacts/Release/{{ProductId}}Harness
$H --help
$H --seconds=2 --profile {{HarnessSource}} --out=/tmp/render.wav
$H --list-params; $H --list-presets
python3 scripts/dsp-kit/dspkit.py smoke $H                  # the contract's minimum, in one go
PRODUCT_MALLOC_TRAP_SELFTEST=1 $H {{HarnessSource}}          # must abort: the trap is live (rule 28)
```

The harness implements `scripts/dsp-kit/harness-contract.md` (v3). Presets come from `presets/`
(`FACTORY_PRESETS` overrides the folder, `PRESETS_CSV` the list). Product-specific flags and
metrics are the two hooks at the end of `tools/harness/Main.cpp`.

## Tests

```bash
ctest --preset fast                       # every build: contract, rate x block matrix, health, state (< 60 s)
ctest --preset all                        # plus goldens and the lints
HARNESS=$H python3 tools/tests/test_plugin.py --label fast   # the same suite by hand
python3 tools/tests/check_table.py --harness $H --kind {{kind}}   # parameters.csv vs the plugin and the manual
python3 scripts/dsp-kit/presetlint.py bank presets.csv --tier {{Tier}} --harness $H
python3 scripts/dsp-kit/presetlint.py render presets.csv --tier {{Tier}} --harness $H --md docs/presets.md
python3 scripts/dsp-kit/perf.py run --harness $H --arch x86_64   # the CPU row of the release gate
```

The shared code (`Source/oberli/`, README.md there) has its own tests in the fast label:
`oberli_loudness` (EBU Tech 3341 / 3342), `oberli_shared` (Analysis, References, RenderSafety),
`bounce_null` (a bounce renders what playback does: `tools/tests/check_bounce.py`) and
`oberli_copy` (the folder is the template's copy). Refresh it from the template:

```bash
python3 tools/refresh_oberli.py           # a shallow clone of Oberli-Audio/plugin-template, then the copy
python3 tools/refresh_oberli.py --check   # what oberli_copy runs
```

## Validators (G6, the release gate)

```bash
python3 scripts/dsp-kit/validate_plugin.py --profile quick --plugin "build/{{ProductId}}_artefacts/Release/VST3/{{Product}}.vst3"
python3 scripts/dsp-kit/validate_plugin.py --profile full  --plugin "build/{{ProductId}}_artefacts/Release/VST3/{{Product}}.vst3"
cmake --preset asan && cmake --build --preset asan && \
  python3 scripts/dsp-kit/validate_plugin.py --profile full --sanitizer asan \
    --plugin "build-asan/{{ProductId}}_artefacts/RelWithDebInfo/VST3/{{Product}}.vst3"   # pluginval from tools.toml pluginval.asan
```

pluginval is taken from `$PLUGINVAL`, then `tools.toml`, then `/Applications`; the Steinberg
validator is built from the pinned SDK commit into `build-validators/` on first use. The
complete list is the "Release gate" section of `scripts/dsp-kit/../joint-plugin-workflow.md`
in sound-design-bob; `validate_plugin.py` runs its unattended part.

## Release

ship-bob's six steps (`scripts/ship/release.py dry-run` prints them for this repo): bump
`VERSION`, the changelog section, `notices.py generate .`, `shipcheck.py known-issues . --write`,
`shipcheck.py docs .` green, one commit, the tag `v<VERSION>`; the workflow builds, signs,
zips and drafts the release; smoke-test the downloaded assets; publish; tell the friends.
