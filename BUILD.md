# Ancient Synth — Build Guide
## macOS · Xcode · Apple Developer

---

## Prerequisites (one-time setup)

### 1. Xcode
Install from the Mac App Store, then accept the licence:
```
sudo xcodebuild -license accept
```
Install command-line tools if prompted:
```
xcode-select --install
```

### 2. Homebrew (if not already installed)
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. CMake
```
brew install cmake
```
Verify: `cmake --version` — should be 3.22 or newer.

### 4. Git
```
brew install git
```

---

## Option A — CMake (Recommended, no Projucer needed)

This is the fastest route. CMake downloads JUCE automatically.

### Step 1 — Unzip the project
Unzip `AncientSynth.zip` anywhere, for example your Desktop:
```
cd ~/Desktop
unzip AncientSynth.zip
cd AncientSynth
```

### Step 2 — Configure (generates an Xcode project)
```
cmake -B build -G Xcode -DCMAKE_BUILD_TYPE=Release
```
This downloads JUCE 7.0.9 from GitHub (~5 minutes on first run, cached after that).

### Step 3 — Open in Xcode (optional — for browsing/debugging)
```
open build/AncientSynth.xcodeproj
```
Select the **AncientSynth_AU** or **AncientSynth_VST3** scheme from the scheme picker, set the destination to **My Mac**, then Build (⌘B).

### Step 4 — Build from terminal (faster)
```
cmake --build build --config Release --parallel
```
Plugins are written to:
```
build/AncientSynth_artefacts/Release/
  AU/   → AncientSynth.component
  VST3/ → AncientSynth.vst3
  Standalone/ → AncientSynth.app
```

### Step 5 — Install
```
# AU (Logic, GarageBand, MainStage)
cp -r build/AncientSynth_artefacts/Release/AU/AncientSynth.component \
      ~/Library/Audio/Plug-Ins/Components/

# VST3 (Ableton, Bitwig, etc.)
cp -r build/AncientSynth_artefacts/Release/VST3/AncientSynth.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/

# Standalone — just double-click or move to /Applications
```

### Step 6 — Validate the AU
```
auval -v aumu AS30 AnSn
```
A passing `auval` result means Logic will load it cleanly.

---

## Option B — Projucer (GUI workflow)

Use this if you prefer working entirely inside Projucer/Xcode without touching the terminal.

### Step 1 — Download JUCE
Go to https://juce.com/get-juce and download **JUCE 7**. Unzip it anywhere, e.g. `~/JUCE`.

### Step 2 — Create a new Projucer project
1. Open **Projucer** (inside the JUCE folder → `Projucer.app`)
2. **New Project → Audio Plug-In**
3. Set:
   - **Project Name:** AncientSynth
   - **Manufacturer Name:** YourStudio
   - **Manufacturer Code:** AnSn
   - **Plugin Code:** AS30
   - **Plugin Formats:** ✓ AU  ✓ VST3  ✓ Standalone
   - **Plugin is a Synth:** ✓
   - **Plugin MIDI Input:** ✓
4. Click **Create Project** and save it next to the unzipped source files.

### Step 3 — Add source files
In Projucer's file browser (left panel):
1. Right-click the **Source** group → **Add Existing Files**
2. Navigate to the unzipped `AncientSynth/Source/` folder
3. Add all files:
   - `PluginProcessor.cpp`
   - `Engines.h`
   - `DSP.h`
   - `Presets.h`
   - `UI/UI.h`
4. **Delete** the auto-generated `PluginProcessor.h`, `PluginProcessor.cpp`, `PluginEditor.h`, and `PluginEditor.cpp` that Projucer created — replace them entirely with the files above.

### Step 4 — Remove auto-generated content
The Projucer template creates its own PluginProcessor/Editor files. Since Ancient Synth is self-contained in a single `.cpp`, you only need `PluginProcessor.cpp` in the compile list. In Projucer, right-click any other `.cpp` files and **Exclude from build** or remove them.

### Step 5 — Add required JUCE modules
In Projucer → **Modules** panel, ensure these are added:
- `juce_audio_basics`
- `juce_audio_devices`
- `juce_audio_formats`
- `juce_audio_plugin_client`
- `juce_audio_processors`
- `juce_audio_utils`
- `juce_core`
- `juce_data_structures`
- `juce_dsp`
- `juce_events`
- `juce_graphics`
- `juce_gui_basics`
- `juce_gui_extra`

### Step 6 — C++ standard
In Projucer → **Exporters → Xcode (macOS)** → **Extra compiler flags:**
```
-std=c++17
```
Or set the **C++ Language Standard** dropdown to **C++17**.

### Step 7 — Deployment target
Set **macOS Deployment Target** to **11.0** (supports Apple Silicon and Intel).

### Step 8 — Code signing
In Projucer → **Exporters → Xcode (macOS)**:
- Set **Code Signing Identity** to your Apple Developer certificate, e.g.:
  `Apple Development: Your Name (XXXXXXXXXX)`
- Or set to `-` (hyphen) for local development without signing.

### Step 9 — Save and open Xcode
Click **Save and Open in Xcode** in Projucer. Then in Xcode:
1. Select the **AncientSynth_AU** scheme
2. Set destination to **My Mac**
3. Press ⌘B to build

---

## Code Signing for Distribution

If you want to distribute the plugin or run it in a sandboxed DAW:

```bash
# Sign the AU
codesign --force --sign "Developer ID Application: Your Name (XXXXXXXXXX)" \
  --timestamp \
  ~/Library/Audio/Plug-Ins/Components/AncientSynth.component

# Sign the VST3
codesign --force --sign "Developer ID Application: Your Name (XXXXXXXXXX)" \
  --timestamp \
  ~/Library/Audio/Plug-Ins/VST3/AncientSynth.vst3

# Verify
codesign --verify --verbose \
  ~/Library/Audio/Plug-Ins/Components/AncientSynth.component
```

Replace `Your Name (XXXXXXXXXX)` with your actual Developer ID certificate name.
Find yours with: `security find-identity -v -p codesigning`

---

## Troubleshooting

**Logic says "not validated"**
Run `auval -v aumu AS30 AnSn` and check the output. Common cause: code signing mismatch. Try setting identity to `-` for local use.

**CMake can't find Xcode tools**
Run `sudo xcode-select -s /Applications/Xcode.app/Contents/Developer`

**Build fails: JuceHeader.h not found**
This only appears if you try to compile a header file directly. Ensure only `PluginProcessor.cpp` is in the CMake sources list (it already is).

**Apple Silicon (M-series) vs Intel**
The CMake setup builds for your native architecture by default. For a universal binary:
```
cmake -B build -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_BUILD_TYPE=Release
```

**JUCE download fails behind a proxy**
Set `GIT_REPOSITORY` in CMakeLists.txt to a local clone of JUCE, or download manually and use:
```
FetchContent_Declare(JUCE SOURCE_DIR /path/to/local/JUCE)
```

---

## Quick Reference

| Task | Command |
|------|---------|
| First-time configure | `cmake -B build -G Xcode` |
| Build Release | `cmake --build build --config Release --parallel` |
| Open in Xcode | `open build/AncientSynth.xcodeproj` |
| Validate AU | `auval -v aumu AS30 AnSn` |
| Find signing certs | `security find-identity -v -p codesigning` |
| Reset plugin cache | Delete `~/Library/Caches/AudioUnitCache` |
| Plugin codes | Manufacturer: `AnSn` · Plugin: `AS30` |
