#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════════
#  Ancient Synth — Setup Script
#  github.com/Nrbessmer/AncientSynth
#
#  Covers: prerequisites check, git clone, cmake configure + build,
#          plugin install, AU validation, optional code signing
#
#  Usage:
#    chmod +x setup.sh
#    ./setup.sh
#
#  Options:
#    --universal    Build universal binary (arm64 + x86_64)
#    --sign "ID"   Codesign with your Developer ID (see --help)
#    --projucer     Skip build steps, print Projucer instructions only
#    --help         Show usage
# ═══════════════════════════════════════════════════════════════════════════════

set -euo pipefail

# ── Colours ───────────────────────────────────────────────────────────────────
BOLD="\033[1m"
DIM="\033[2m"
AMBER="\033[38;5;172m"
GREEN="\033[38;5;71m"
RED="\033[38;5;160m"
BLUE="\033[38;5;68m"
GREY="\033[38;5;240m"
RESET="\033[0m"

# ── Options ───────────────────────────────────────────────────────────────────
UNIVERSAL=false
SIGN_ID=""
PROJUCER_ONLY=false

for arg in "$@"; do
  case "$arg" in
    --universal)  UNIVERSAL=true ;;
    --sign)       shift; SIGN_ID="${1:-}" ;;
    --projucer)   PROJUCER_ONLY=true ;;
    --help|-h)
      echo ""
      echo "  Ancient Synth — Setup Script"
      echo ""
      echo "  Usage: ./setup.sh [options]"
      echo ""
      echo "  Options:"
      echo "    --universal       Build for both Apple Silicon and Intel"
      echo "    --sign \"ID\"      Codesign with Developer ID certificate"
      echo "    --projucer        Print Projucer setup instructions and exit"
      echo "    --help            Show this message"
      echo ""
      echo "  Examples:"
      echo "    ./setup.sh"
      echo "    ./setup.sh --universal"
      echo "    ./setup.sh --sign \"Developer ID Application: Your Name (XXXXXXXXXX)\""
      echo ""
      exit 0
      ;;
  esac
done

# ── Helpers ───────────────────────────────────────────────────────────────────

# Print a section banner
banner() {
  echo ""
  echo -e "${AMBER}${BOLD}━━━  $1  ━━━${RESET}"
  echo ""
}

# Print a step
step() {
  echo -e "  ${BLUE}▸${RESET} $1"
}

# Print success
ok() {
  echo -e "  ${GREEN}✓${RESET}  $1"
}

# Print info/dim line
info() {
  echo -e "  ${GREY}$1${RESET}"
}

# Print a warning (non-fatal)
warn() {
  echo -e "  ${AMBER}⚠${RESET}  $1"
}

# Print error and exit
fail() {
  echo ""
  echo -e "  ${RED}✗  Error: $1${RESET}"
  echo ""
  exit 1
}

# Run a command, show it, exit on failure
run() {
  echo -e "  ${GREY}\$ $*${RESET}"
  if ! "$@"; then
    fail "Command failed: $*"
  fi
}

# Run a command quietly, show it only on failure
run_q() {
  echo -e "  ${GREY}\$ $*${RESET}"
  if ! output=$("$@" 2>&1); then
    echo "$output"
    fail "Command failed: $*"
  fi
}

# Check a command exists
need() {
  if ! command -v "$1" &>/dev/null; then
    fail "'$1' not found. $2"
  fi
}

# ── Header ────────────────────────────────────────────────────────────────────
clear
echo ""
echo -e "${AMBER}${BOLD}"
echo "   █████╗ ███╗   ██╗ ██████╗██╗███████╗███╗   ██╗████████╗"
echo "  ██╔══██╗████╗  ██║██╔════╝██║██╔════╝████╗  ██║╚══██╔══╝"
echo "  ███████║██╔██╗ ██║██║     ██║█████╗  ██╔██╗ ██║   ██║   "
echo "  ██╔══██║██║╚██╗██║██║     ██║██╔══╝  ██║╚██╗██║   ██║   "
echo "  ██║  ██║██║ ╚████║╚██████╗██║███████╗██║ ╚████║   ██║   "
echo "  ╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝   "
echo -e "${RESET}"
echo -e "  ${BOLD}SYNTH${RESET} ${GREY}— Setup Script${RESET}"
echo -e "  ${GREY}github.com/Nrbessmer/AncientSynth${RESET}"
echo -e "  ${GREY}JUCE 8.0.7 · AU · VST3 · Standalone · macOS 11.0+${RESET}"
echo ""
echo -e "  ${GREY}$(date '+%Y-%m-%d %H:%M:%S')${RESET}"
echo ""

# ── Projucer-only mode ────────────────────────────────────────────────────────
if $PROJUCER_ONLY; then
  banner "PROJUCER SETUP INSTRUCTIONS"

  echo -e "  These steps replace Sections 04–05 of the setup guide."
  echo ""

  echo -e "  ${BOLD}1.  Download JUCE 7${RESET}"
  info     "      juce.com/get-juce — download JUCE 7, unzip to ~/JUCE"
  info     "      Open Projucer.app from inside that folder."
  info     "      If it asks to upgrade modules, click No."
  echo ""

  echo -e "  ${BOLD}2.  New Project settings${RESET}"
  info     "      New Project → Audio Plug-In, then use these values:"
  echo ""
  printf   "      %-28s %s\n" "Project Name"             "AncientSynth"
  printf   "      %-28s %s\n" "Manufacturer Name"        "YourStudio"
  printf   "      %-28s %s\n" "Manufacturer Code"        "AnSn"
  printf   "      %-28s %s\n" "Plugin Code"              "AS30"
  printf   "      %-28s %s\n" "Formats"                  "AU  VST3  Standalone"
  printf   "      %-28s %s\n" "Plugin is a Synth"        "Yes"
  printf   "      %-28s %s\n" "MIDI Input"               "Yes"
  printf   "      %-28s %s\n" "C++ Standard"             "C++17"
  printf   "      %-28s %s\n" "macOS Deployment Target"  "11.0"
  echo ""

  echo -e "  ${BOLD}3.  Add source files${RESET}"
  info     "      Right-click Source → Add Existing Files"
  info     "      Add all 5 files from AncientSynth/Source/"
  info     "      DELETE the Projucer stubs: PluginProcessor.h,"
  info     "      PluginEditor.h, PluginEditor.cpp"
  info     "      Only PluginProcessor.cpp should be compiled."
  echo ""

  echo -e "  ${BOLD}4.  Required JUCE modules${RESET}"
  info     "      Go to Modules and confirm all are present:"
  echo ""
  for m in juce_audio_utils juce_audio_processors juce_audio_plugin_client \
           juce_audio_basics juce_audio_devices juce_audio_formats juce_dsp \
           juce_gui_basics juce_gui_extra juce_graphics juce_core \
           juce_data_structures juce_events; do
    info "      · $m"
  done
  echo ""

  echo -e "  ${BOLD}5.  Build${RESET}"
  info     "      Click Save and Open in Xcode."
  info     "      Scheme: AncientSynth_AU"
  info     "      Destination: My Mac"
  info     "      Press ⌘B to build."
  echo ""

  exit 0
fi

# ══════════════════════════════════════════════════════════════════════════════
#  STEP 1 — Prerequisites
# ══════════════════════════════════════════════════════════════════════════════
banner "STEP 1 / 6  —  CHECKING PREREQUISITES"

# Xcode — check full Xcode app is present and selected (not just CLI tools)
step "Checking Xcode..."

XCODE_APP="/Applications/Xcode.app"
if [[ ! -d "$XCODE_APP" ]]; then
  echo ""
  fail "Xcode is not installed. Please install it from the Mac App Store, then re-run this script."
fi

CURRENT_DEV=$(xcode-select -p 2>/dev/null || true)

# If pointed at CLI tools only, switch to Xcode
if [[ "$CURRENT_DEV" == *"CommandLineTools"* ]] || [[ "$CURRENT_DEV" != *"Xcode.app"* ]]; then
  warn "Developer directory is set to CLI tools — switching to Xcode..."
  run sudo xcode-select -s "$XCODE_APP/Contents/Developer"
  ok "Switched to: $XCODE_APP/Contents/Developer"
else
  ok "Xcode: $CURRENT_DEV"
fi

# Accept licence if needed
step "Checking Xcode licence..."
if ! xcodebuild -version &>/dev/null 2>&1; then
  step "Accepting Xcode licence..."
  run sudo xcodebuild -license accept
fi
XCODE_VER=$(xcodebuild -version 2>/dev/null | head -1)
ok "Licence accepted — $XCODE_VER"

# Git
step "Checking git..."
need git "Install with: brew install git"
GIT_VER=$(git --version | awk '{print $3}')
ok "git $GIT_VER"

# CMake
step "Checking cmake..."
need cmake "Install with: brew install cmake"
CMAKE_VER=$(cmake --version | head -1 | awk '{print $3}')
CMAKE_MAJOR=$(echo "$CMAKE_VER" | cut -d. -f1)
CMAKE_MINOR=$(echo "$CMAKE_VER" | cut -d. -f2)
if [[ "$CMAKE_MAJOR" -lt 3 ]] || [[ "$CMAKE_MAJOR" -eq 3 && "$CMAKE_MINOR" -lt 22 ]]; then
  fail "cmake $CMAKE_VER is too old. Need 3.22+. Run: brew upgrade cmake"
fi
ok "cmake $CMAKE_VER"

# macOS version
step "Checking macOS version..."
OS_VER=$(sw_vers -productVersion)
OS_MAJOR=$(echo "$OS_VER" | cut -d. -f1)
if [[ "$OS_MAJOR" -lt 11 ]]; then
  fail "macOS $OS_VER detected. Ancient Synth requires macOS 11.0 or later."
fi
ok "macOS $OS_VER"

# Architecture
ARCH=$(uname -m)
step "Checking architecture..."
if [[ "$ARCH" == "arm64" ]]; then
  ok "Apple Silicon (arm64)"
  if ! $UNIVERSAL; then
    info "  Tip: pass --universal to also support Intel Macs"
  fi
else
  ok "Intel ($ARCH)"
fi

# ══════════════════════════════════════════════════════════════════════════════
#  STEP 2 — Clone or update
# ══════════════════════════════════════════════════════════════════════════════
banner "STEP 2 / 6  —  GET THE CODE"

# ══════════════════════════════════════════════════════════════════════════════
#  STEP 2 — Get the source files
# ══════════════════════════════════════════════════════════════════════════════
banner "STEP 2 / 6  —  GET THE CODE"

REPO="https://github.com/Nrbessmer/AncientSynth.git"
PROJECT_DIR="AncientSynth"

# Work out where the script itself lives — source files may be next to it
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SOURCE_FILES=(
  "CMakeLists.txt"
  "Source/PluginProcessor.cpp"
  "Source/Engines.h"
  "Source/DSP.h"
  "Source/Presets.h"
  "Source/UI/UI.h"
)

# Helper: check if all source files exist in a given directory
files_present() {
  local dir="$1"
  for f in "${SOURCE_FILES[@]}"; do
    [[ ! -f "$dir/$f" ]] && return 1
  done
  return 0
}

# ── Case 1: we are already inside the project folder (files right here) ───────
if files_present "$SCRIPT_DIR"; then
  ok "Source files found alongside this script — using them"
  cd "$SCRIPT_DIR"

# ── Case 2: project subfolder already exists with files ───────────────────────
elif [[ -d "$PROJECT_DIR" ]] && files_present "$PROJECT_DIR"; then
  ok "Project folder $PROJECT_DIR exists and is complete"
  cd "$PROJECT_DIR"

# ── Case 3: git clone, then check if the repo actually has the files ──────────
else
  if [[ -d "$PROJECT_DIR/.git" ]]; then
    step "Repository exists — pulling latest..."
    cd "$PROJECT_DIR"
    run git pull
    ok "Repository updated"
  else
    step "Cloning from github.com/Nrbessmer..."
    run git clone "$REPO"
    ok "Cloned"
    cd "$PROJECT_DIR"
  fi

  # If the repo was empty or only had a README, the source files won't be there.
  # In that case the user needs to push the files to GitHub first.
  if ! files_present "."; then
    echo ""
    echo -e "  ${AMBER}${BOLD}The GitHub repository exists but doesn't contain the source files yet.${RESET}"
    echo ""
    echo -e "  ${BOLD}You need to push the project files to GitHub first.${RESET}"
    echo -e "  From the folder containing CMakeLists.txt and Source/, run:"
    echo ""
    echo -e "  ${GREY}  cd /path/to/AncientSynth${RESET}"
    echo -e "  ${GREY}  git init${RESET}"
    echo -e "  ${GREY}  git remote add origin $REPO${RESET}"
    echo -e "  ${GREY}  git add .${RESET}"
    echo -e "  ${GREY}  git commit -m \"Initial commit\"${RESET}"
    echo -e "  ${GREY}  git push -u origin main${RESET}"
    echo ""
    echo -e "  Then re-run: ${GREY}sudo ./setup.sh${RESET}"
    echo ""
    echo -e "  ${AMBER}OR — if you have the zip file, unzip it, cd into it, and run setup.sh from there.${RESET}"
    echo ""
    exit 1
  fi
fi

# Final verification
step "Verifying source files..."
MISSING=()
for f in "${SOURCE_FILES[@]}"; do
  [[ ! -f "$f" ]] && MISSING+=("$f")
done

if [[ ${#MISSING[@]} -gt 0 ]]; then
  echo ""
  for f in "${MISSING[@]}"; do
    warn "Missing: $f"
  done
  fail "Project files incomplete. Try re-cloning."
fi

ok "All 6 source files present"

# ══════════════════════════════════════════════════════════════════════════════
#  STEP 3 — Clone JUCE and configure
# ══════════════════════════════════════════════════════════════════════════════
banner "STEP 3 / 6  —  CMAKE CONFIGURE"

JUCE_DIR="$(pwd)/juce"
JUCE_REPO="https://github.com/juce-framework/JUCE.git"
JUCE_TAG="8.0.7"

# ── Clone JUCE as the real user (not root) ────────────────────────────────────
# This avoids git safe.directory ownership errors that occur when cmake
# tries to clone as root on external drives or foreign-owned directories.

if [[ -f "$JUCE_DIR/CMakeLists.txt" ]]; then
  ok "JUCE already present at ./juce — skipping download"
else
  step "Cloning JUCE $JUCE_TAG into ./juce  (~5 min on first run)..."
  info "Cloning as user ${SUDO_USER:-$(whoami)} to avoid permission issues..."

  if [[ -n "${SUDO_USER:-}" ]]; then
    # Run git clone as the actual logged-in user, not root
    sudo -u "$SUDO_USER" git clone \
      --depth 1 \
      --branch "$JUCE_TAG" \
      "$JUCE_REPO" \
      "$JUCE_DIR"
  else
    git clone \
      --depth 1 \
      --branch "$JUCE_TAG" \
      "$JUCE_REPO" \
      "$JUCE_DIR"
  fi

  if [[ ! -f "$JUCE_DIR/CMakeLists.txt" ]]; then
    fail "JUCE clone failed — check your internet connection and try again."
  fi
  ok "JUCE $JUCE_TAG cloned successfully"
fi

# ── Clean up any old failed FetchContent build dirs ───────────────────────────
if [[ -d "build/_deps" ]]; then
  warn "Removing old build/_deps from previous failed attempt..."
  rm -rf build/_deps
  ok "Cleaned"
fi

# ── CMake configure ───────────────────────────────────────────────────────────
info "Generating Xcode project..."
echo ""

if $UNIVERSAL; then
  ARCHS="arm64;x86_64"
  step "Configuring for Universal Binary (arm64 + x86_64)..."
  run cmake -B build -G Xcode \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="$ARCHS" \
    -DJUCE_PATH="$JUCE_DIR"
else
  step "Configuring for native architecture ($ARCH)..."
  run cmake -B build -G Xcode \
    -DCMAKE_BUILD_TYPE=Release \
    -DJUCE_PATH="$JUCE_DIR"
fi

ok "CMake configuration complete"

# ══════════════════════════════════════════════════════════════════════════════
#  STEP 4 — Build
# ══════════════════════════════════════════════════════════════════════════════
banner "STEP 4 / 6  —  BUILD"

info "Compiling with $(sysctl -n hw.logicalcpu) parallel jobs..."
echo ""

BUILD_START=$(date +%s)

run cmake --build build --config Release --parallel

BUILD_END=$(date +%s)
BUILD_TIME=$((BUILD_END - BUILD_START))

echo ""
ok "Build complete in ${BUILD_TIME}s"

# Verify artefacts
ARTEFACT_BASE="build/AncientSynth_artefacts/Release"
step "Checking build artefacts..."

# Discover actual filenames (product name may contain spaces)
AU_SRC=$(find "${ARTEFACT_BASE}/AU"         -name "*.component" -maxdepth 1 2>/dev/null | head -1)
VST_SRC=$(find "${ARTEFACT_BASE}/VST3"      -name "*.vst3"      -maxdepth 1 2>/dev/null | head -1)
STANDALONE=$(find "${ARTEFACT_BASE}/Standalone" -name "*.app"   -maxdepth 1 2>/dev/null | head -1)

[[ -n "$AU_SRC"     ]] && ok "AU:         $AU_SRC"    || warn "AU not found in ${ARTEFACT_BASE}/AU"
[[ -n "$VST_SRC"    ]] && ok "VST3:       $VST_SRC"   || warn "VST3 not found in ${ARTEFACT_BASE}/VST3"
[[ -n "$STANDALONE" ]] && ok "Standalone: $STANDALONE" || warn "Standalone not found"

# ══════════════════════════════════════════════════════════════════════════════
#  STEP 5 — Install
# ══════════════════════════════════════════════════════════════════════════════
banner "STEP 5 / 6  —  INSTALL PLUGINS"

# AU_SRC and VST_SRC already set by artefact discovery above
AU_DEST="$HOME/Library/Audio/Plug-Ins/Components"
VST_DEST="$HOME/Library/Audio/Plug-Ins/VST3"

# AU
AU_INSTALLED=""
if [[ -n "$AU_SRC" ]] && [[ -e "$AU_SRC" ]]; then
  step "Installing AU → $AU_DEST/..."
  mkdir -p "$AU_DEST"
  run cp -r "$AU_SRC" "$AU_DEST/"
  AU_INSTALLED="$AU_DEST/$(basename "$AU_SRC")"
  ok "AU installed: $AU_INSTALLED"
else
  warn "AU not found — skipping AU install"
fi

# VST3
VST_INSTALLED=""
if [[ -n "$VST_SRC" ]] && [[ -e "$VST_SRC" ]]; then
  step "Installing VST3 → $VST_DEST/..."
  mkdir -p "$VST_DEST"
  run cp -r "$VST_SRC" "$VST_DEST/"
  VST_INSTALLED="$VST_DEST/$(basename "$VST_SRC")"
  ok "VST3 installed: $VST_INSTALLED"
else
  warn "VST3 not found — skipping VST3 install"
fi

# Clear AU cache AFTER install so the system registers the new component
step "Resetting AU cache so auval and Logic can find the plugin..."
AU_CACHE="$HOME/Library/Caches/AudioUnitCache"
rm -rf "$AU_CACHE" 2>/dev/null || true
# Also clear the system-level cache
sudo find /Library/Caches -name "*AudioUnit*" -delete 2>/dev/null || true
ok "AU cache cleared"

# ══════════════════════════════════════════════════════════════════════════════
#  STEP 5b — Optional code signing
# ══════════════════════════════════════════════════════════════════════════════
if [[ -n "$SIGN_ID" ]]; then
  banner "STEP 5b — CODE SIGNING"

  step "Signing AU with: $SIGN_ID"
  if [[ -e "$AU_INSTALLED" ]]; then
    run codesign --force --sign "$SIGN_ID" --timestamp \
      "$AU_INSTALLED"
    ok "AU signed"
  fi

  step "Signing VST3..."
  if [[ -e "$VST_INSTALLED" ]]; then
    run codesign --force --sign "$SIGN_ID" --timestamp \
      "$VST_INSTALLED"
    ok "VST3 signed"
  fi

  step "Verifying AU signature..."
  if codesign --verify --verbose \
    "$AU_INSTALLED" 2>&1 | grep -q "valid on disk"; then
    ok "Signature valid"
  else
    warn "Signature verification — check the output above"
  fi
else
  echo ""
  info "  Skipping code signing (not needed for local use)."
  info "  To sign: ./setup.sh --sign \"Developer ID Application: Your Name (TEAMID)\""
  info "  List your certs: security find-identity -v -p codesigning"
fi

# ══════════════════════════════════════════════════════════════════════════════
#  STEP 6 — Validate AU
# ══════════════════════════════════════════════════════════════════════════════
banner "STEP 6 / 6  —  VALIDATE AU"

info "Running Apple's AU validation tool (auval)..."
info "Logic will refuse to load any AU that fails this test."
echo ""

AUVAL_OUT=$(auval -v aumu AS30 AnSn 2>&1) || true

if echo "$AUVAL_OUT" | grep -q "AU validation succeeded"; then
  ok "AU validation PASSED"
elif echo "$AUVAL_OUT" | grep -q "FAILED"; then
  echo "$AUVAL_OUT" | tail -20
  echo ""
  warn "AU validation FAILED — see output above"
  warn "Common fix: set Code Signing Identity to - in Xcode project settings"
  warn "Then rebuild and reinstall."
else
  echo "$AUVAL_OUT" | tail -10
  warn "auval returned unexpected output — check above"
fi

# ══════════════════════════════════════════════════════════════════════════════
#  SUMMARY
# ══════════════════════════════════════════════════════════════════════════════
echo ""
echo -e "${AMBER}${BOLD}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
echo ""
echo -e "  ${GREEN}${BOLD}All done!${RESET}  Ancient Synth is installed."
echo ""

# Plugin locations
echo -e "  ${BOLD}Installed to:${RESET}"
if [[ -e "$AU_INSTALLED" ]]; then
  echo -e "  ${GREEN}✓${RESET}  AU    $AU_DEST/AncientSynth.component"
fi
if [[ -e "$VST_INSTALLED" ]]; then
  echo -e "  ${GREEN}✓${RESET}  VST3  $VST_DEST/AncientSynth.vst3"
fi

STANDALONE="${ARTEFACT_BASE}/Standalone/AncientSynth.app"
if [[ -e "$STANDALONE" ]]; then
  echo -e "  ${GREEN}✓${RESET}  App   $STANDALONE"
fi

echo ""
echo -e "  ${BOLD}Open in Xcode:${RESET}"
echo -e "  ${GREY}\$ open build/AncientSynth.xcodeproj${RESET}"
echo ""
echo -e "  ${BOLD}Next update:${RESET}"
echo -e "  ${GREY}\$ cd AncientSynth && git pull && ./setup.sh${RESET}"
echo ""
echo -e "  ${BOLD}Plugin codes:${RESET}  Manufacturer ${AMBER}AnSn${RESET}  ·  Plugin ${AMBER}AS30${RESET}"
echo ""
echo -e "${AMBER}${BOLD}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
echo ""
