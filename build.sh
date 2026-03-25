#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════════
#  Ancient Synth — Build Script
#  Rebuilds and reinstalls the plugin from existing source.
#  Run this after setup.sh has completed at least once.
#
#  Usage:
#    chmod +x build.sh
#    ./build.sh
#
#  Options:
#    --universal    Build universal binary (arm64 + x86_64)
#    --debug        Build Debug instead of Release
#    --clean        Wipe build folder before building
#    --sign "ID"    Codesign after install
#    --no-install   Build only, don't install
#    --help         Show usage
# ═══════════════════════════════════════════════════════════════════════════════

set -euo pipefail

# ── Colours & symbols ──────────────────────────────────────────────────────────
BOLD="\033[1m"
DIM="\033[2m"
AMBER="\033[38;5;172m"
GREEN="\033[38;5;71m"
RED="\033[38;5;160m"
BLUE="\033[38;5;68m"
CYAN="\033[38;5;74m"
GREY="\033[38;5;240m"
WHITE="\033[38;5;252m"
RESET="\033[0m"

TICK="${GREEN}✓${RESET}"
CROSS="${RED}✗${RESET}"
ARROW="${BLUE}▸${RESET}"
WARN="${AMBER}⚠${RESET}"
DOTS="${GREY}···${RESET}"

# ── Options ────────────────────────────────────────────────────────────────────
UNIVERSAL=false
CONFIG="Release"
CLEAN=false
SIGN_ID=""
NO_INSTALL=false

for arg in "$@"; do
  case "$arg" in
    --universal)  UNIVERSAL=true ;;
    --debug)      CONFIG="Debug" ;;
    --clean)      CLEAN=true ;;
    --no-install) NO_INSTALL=true ;;
    --sign)       shift; SIGN_ID="${1:-}" ;;
    --help|-h)
      echo ""
      echo -e "  ${BOLD}Ancient Synth — Build Script${RESET}"
      echo ""
      echo "  Usage: ./build.sh [options]"
      echo ""
      echo "  Options:"
      echo "    --universal     Build for Apple Silicon + Intel"
      echo "    --debug         Build Debug configuration"
      echo "    --clean         Wipe build folder first"
      echo "    --no-install    Build only, skip install step"
      echo "    --sign \"ID\"    Codesign with Developer ID"
      echo "    --help          Show this message"
      echo ""
      exit 0
      ;;
  esac
done

# ── Spinner ────────────────────────────────────────────────────────────────────
SPIN_PID=""
SPIN_MSG=""

spin_start() {
  SPIN_MSG="$1"
  (
    FRAMES=("⠋" "⠙" "⠹" "⠸" "⠼" "⠴" "⠦" "⠧" "⠇" "⠏")
    i=0
    while true; do
      printf "\r  ${AMBER}${FRAMES[$((i % 10))]}${RESET}  %s" "$SPIN_MSG"
      sleep 0.08
      ((i++)) || true
    done
  ) &
  SPIN_PID=$!
  disown "$SPIN_PID" 2>/dev/null || true
}

spin_stop() {
  if [[ -n "$SPIN_PID" ]]; then
    kill "$SPIN_PID" 2>/dev/null || true
    wait "$SPIN_PID" 2>/dev/null || true
    SPIN_PID=""
    printf "\r%80s\r" ""  # clear the line
  fi
}

# ── Progress bar ───────────────────────────────────────────────────────────────
progress() {
  local current=$1 total=$2 label="${3:-}"
  local pct=$(( current * 100 / total ))
  local filled=$(( current * 30 / total ))
  local bar=""
  for ((i=0;i<filled;i++)); do bar+="█"; done
  for ((i=filled;i<30;i++)); do bar+="░"; done
  printf "\r  ${AMBER}[${bar}]${RESET} ${WHITE}%3d%%${RESET}  ${GREY}%s${RESET}" "$pct" "$label"
}

progress_done() {
  printf "\r%80s\r" ""
}

# ── Timing ─────────────────────────────────────────────────────────────────────
SCRIPT_START=$(date +%s)

elapsed() {
  local now=$(date +%s)
  local secs=$(( now - SCRIPT_START ))
  printf "%dm%02ds" $(( secs/60 )) $(( secs%60 ))
}

section_start=0
section_elapsed() {
  local now=$(date +%s)
  echo $(( now - section_start ))
}

# ── Helpers ────────────────────────────────────────────────────────────────────
banner() {
  echo ""
  echo -e "${AMBER}${BOLD}━━━  $1  ━━━${RESET}  ${GREY}[$(elapsed)]${RESET}"
  echo ""
  section_start=$(date +%s)
}

step()  { echo -e "  ${ARROW} $1"; }
ok()    { echo -e "  ${TICK}  $1"; }
info()  { echo -e "  ${GREY}$1${RESET}"; }
warn()  { echo -e "  ${WARN}  $1"; }
fail()  { spin_stop; echo ""; echo -e "  ${CROSS}  ${RED}${BOLD}Error:${RESET} $1"; echo ""; exit 1; }

run_silent() {
  # Run command, capture output, show on failure only
  local output
  if ! output=$("$@" 2>&1); then
    spin_stop
    echo ""
    echo "$output" | tail -30
    fail "Command failed: $*"
  fi
}

run_visible() {
  echo -e "  ${GREY}\$ $*${RESET}"
  if ! "$@"; then
    fail "Command failed: $*"
  fi
}

# ── Header ─────────────────────────────────────────────────────────────────────
clear
echo ""
echo -e "${AMBER}${BOLD}"
cat << 'ASCII'
   █████╗ ███╗   ██╗ ██████╗██╗███████╗███╗   ██╗████████╗
  ██╔══██╗████╗  ██║██╔════╝██║██╔════╝████╗  ██║╚══██╔══╝
  ███████║██╔██╗ ██║██║     ██║█████╗  ██╔██╗ ██║   ██║   
  ██╔══██║██║╚██╗██║██║     ██║██╔══╝  ██║╚██╗██║   ██║   
  ██║  ██║██║ ╚████║╚██████╗██║███████╗██║ ╚████║   ██║   
  ╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝  
ASCII
echo -e "${RESET}"
echo -e "  ${BOLD}SYNTH — Build Script${RESET}  ${GREY}$(date '+%Y-%m-%d %H:%M:%S')${RESET}"
echo -e "  ${GREY}github.com/Nrbessmer/AncientSynth${RESET}"
echo ""
echo -e "  ${GREY}Config: ${WHITE}$CONFIG${RESET}  ${GREY}|  Universal: ${WHITE}$UNIVERSAL${RESET}  ${GREY}|  Install: ${WHITE}$([ $NO_INSTALL = true ] && echo No || echo Yes)${RESET}"
echo ""

# ══════════════════════════════════════════════════════════════════════════════
#  PHASE 1 — Environment check
# ══════════════════════════════════════════════════════════════════════════════
banner "PHASE 1 / 5  —  ENVIRONMENT"

step "Confirming Xcode..."
if [[ ! -d "/Applications/Xcode.app" ]]; then
  fail "Xcode not found at /Applications/Xcode.app"
fi
XCODE_VER=$(xcodebuild -version 2>/dev/null | head -1)
ok "$XCODE_VER"

step "Confirming CMake..."
need_cmake() { ! command -v cmake &>/dev/null; }
if need_cmake; then fail "cmake not found. Run: brew install cmake"; fi
CMAKE_VER=$(cmake --version | head -1 | awk '{print $3}')
ok "cmake $CMAKE_VER"

step "Confirming source files..."
REQUIRED=("CMakeLists.txt" "Source/PluginProcessor.cpp" "Source/Engines.h"
          "Source/DSP.h" "Source/Presets.h" "Source/UI/UI.h")
ALL_OK=true
for f in "${REQUIRED[@]}"; do
  if [[ ! -f "$f" ]]; then
    warn "Missing: $f"
    ALL_OK=false
  fi
done
$ALL_OK && ok "All 6 source files present" || fail "Source files missing — wrong directory?"

step "Confirming JUCE..."
JUCE_DIR="$(pwd)/juce"
if [[ ! -f "$JUCE_DIR/CMakeLists.txt" ]]; then
  fail "JUCE not found at ./juce — run setup.sh first"
fi
JUCE_VER=$(grep -m1 "VERSION_MAJOR\|set.*JUCE_VERSION\|project.*VERSION" "$JUCE_DIR/CMakeLists.txt" 2>/dev/null | head -1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+' || echo "8.x")
ok "JUCE present ($JUCE_VER)"

ARCH=$(uname -m)
step "Architecture: $ARCH"
if $UNIVERSAL; then
  ok "Building universal binary (arm64 + x86_64)"
else
  ok "Building native ($ARCH)"
fi

echo ""
echo -e "  ${DIM}Phase 1 complete in $(section_elapsed)s${RESET}"

# ══════════════════════════════════════════════════════════════════════════════
#  PHASE 2 — Configure
# ══════════════════════════════════════════════════════════════════════════════
banner "PHASE 2 / 5  —  CMAKE CONFIGURE"

if $CLEAN; then
  step "Cleaning build directory..."
  spin_start "Removing build folder..."
  rm -rf build
  spin_stop
  ok "Build folder removed"
fi

if [[ -d "build" ]]; then
  ok "Existing build directory found — using incremental build"
  info "  (pass --clean to force a full rebuild)"
else
  step "Creating build directory..."
fi

step "Running cmake configure..."
spin_start "Configuring Xcode project..."

CMAKE_ARGS=("-B" "build" "-G" "Xcode" "-DCMAKE_BUILD_TYPE=$CONFIG" "-DJUCE_PATH=$JUCE_DIR")
if $UNIVERSAL; then
  CMAKE_ARGS+=("-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64")
fi

CMAKE_OUT=$(cmake "${CMAKE_ARGS[@]}" 2>&1) || {
  spin_stop
  echo "$CMAKE_OUT" | tail -20
  fail "cmake configure failed"
}
spin_stop

ok "CMake configuration complete"
echo ""
echo -e "  ${DIM}Phase 2 complete in $(section_elapsed)s${RESET}"

# ══════════════════════════════════════════════════════════════════════════════
#  PHASE 3 — Build
# ══════════════════════════════════════════════════════════════════════════════
banner "PHASE 3 / 5  —  COMPILE"

NCPU=$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
info "Compiling with $NCPU parallel jobs ($CONFIG)..."
echo ""

BUILD_START=$(date +%s)

# Run cmake build and parse xcodebuild output for progress
spin_start "Starting compiler..."

# Track compilation progress by counting CompileC lines
TMPLOG=$(mktemp)

cmake --build build --config "$CONFIG" --parallel 2>&1 | tee "$TMPLOG" | (
  total_est=120   # estimated compile units
  compiled=0
  current_file=""

  while IFS= read -r line; do
    # Detect compilation of specific files
    if echo "$line" | grep -q "^CompileC\|^Compile\|\.cpp\|\.mm" 2>/dev/null; then
      fname=$(echo "$line" | grep -oE '[A-Za-z_]+\.(cpp|mm|h)' | head -1 || true)
      if [[ -n "$fname" ]]; then
        current_file="$fname"
        ((compiled++)) || true
        spin_stop
        progress "$compiled" "$total_est" "Compiling $fname"
      fi
    fi
    # Detect linking
    if echo "$line" | grep -q "^Ld " 2>/dev/null; then
      target=$(echo "$line" | awk '{print $2}' | xargs basename 2>/dev/null || echo "...")
      spin_stop
      printf "\r%80s\r" ""
      echo -e "  ${CYAN}⛓${RESET}  Linking $target"
      spin_start "Linking..."
    fi
    # Detect code signing
    if echo "$line" | grep -q "^CodeSign" 2>/dev/null; then
      spin_stop
      printf "\r%80s\r" ""
      echo -e "  ${CYAN}🔏${RESET}  Code signing..."
      spin_start "Signing..."
    fi
    # Detect errors
    if echo "$line" | grep -qiE "error:|fatal error:" 2>/dev/null; then
      spin_stop
      printf "\r%80s\r" ""
      echo -e "  ${RED}$line${RESET}"
    fi
    # Detect warnings count
    if echo "$line" | grep -q "warning generated" 2>/dev/null; then
      wcount=$(echo "$line" | grep -oE '[0-9]+' | head -1 || echo "?")
      spin_stop
      printf "\r%80s\r" ""
      info "  $wcount warnings generated"
    fi
    # Success
    if echo "$line" | grep -q "BUILD SUCCEEDED" 2>/dev/null; then
      spin_stop
      progress_done
      echo -e "  ${GREEN}${BOLD}BUILD SUCCEEDED${RESET}"
    fi
    # Failure
    if echo "$line" | grep -q "BUILD FAILED" 2>/dev/null; then
      spin_stop
      progress_done
      echo -e "  ${RED}${BOLD}BUILD FAILED${RESET}"
    fi
  done
) || true

spin_stop
progress_done

# Check actual result from log
if grep -q "BUILD FAILED" "$TMPLOG" 2>/dev/null; then
  echo ""
  echo -e "  ${RED}${BOLD}Compilation failed. Last errors:${RESET}"
  grep -E "error:|fatal error:" "$TMPLOG" | head -10 | while read -r line; do
    echo -e "  ${RED}$line${RESET}"
  done
  rm -f "$TMPLOG"
  fail "Build failed — see errors above"
fi
rm -f "$TMPLOG"

BUILD_END=$(date +%s)
BUILD_SECS=$(( BUILD_END - BUILD_START ))

echo ""
ok "Compile complete in ${BUILD_SECS}s"

# Verify artefacts
ARTEFACT_BASE="build/AncientSynth_artefacts/$CONFIG"

step "Checking build artefacts..."
AU_SRC=$(find "${ARTEFACT_BASE}/AU"         -name "*.component" -maxdepth 1 2>/dev/null | head -1)
VST_SRC=$(find "${ARTEFACT_BASE}/VST3"      -name "*.vst3"      -maxdepth 1 2>/dev/null | head -1)
APP_SRC=$(find "${ARTEFACT_BASE}/Standalone" -name "*.app"      -maxdepth 1 2>/dev/null | head -1)

[[ -n "$AU_SRC"  ]] && ok "AU:         $(basename "$AU_SRC")" || warn "AU component not found"
[[ -n "$VST_SRC" ]] && ok "VST3:       $(basename "$VST_SRC")" || warn "VST3 not found"
[[ -n "$APP_SRC" ]] && ok "Standalone: $(basename "$APP_SRC")" || warn "Standalone app not found"

# File sizes
if [[ -n "$AU_SRC" ]]; then
  AU_SIZE=$(du -sh "$AU_SRC" 2>/dev/null | awk '{print $1}')
  info "  AU size: $AU_SIZE"
fi

echo ""
echo -e "  ${DIM}Phase 3 complete in $(section_elapsed)s${RESET}"

# ══════════════════════════════════════════════════════════════════════════════
#  PHASE 4 — Install
# ══════════════════════════════════════════════════════════════════════════════
if ! $NO_INSTALL; then
  banner "PHASE 4 / 5  —  INSTALL"

  AU_DEST="$HOME/Library/Audio/Plug-Ins/Components"
  VST_DEST="$HOME/Library/Audio/Plug-Ins/VST3"

  AU_INSTALLED=""
  VST_INSTALLED=""

  # AU
  if [[ -n "$AU_SRC" ]] && [[ -e "$AU_SRC" ]]; then
    step "Installing AU..."
    mkdir -p "$AU_DEST"
    # Remove previous version first
    OLD_AU=$(find "$AU_DEST" -name "Ancient Synth.component" -o -name "AncientSynth.component" 2>/dev/null | head -1)
    if [[ -n "$OLD_AU" ]]; then
      spin_start "Removing old AU..."
      rm -rf "$OLD_AU"
      spin_stop
      info "  Removed: $OLD_AU"
    fi
    spin_start "Copying AU..."
    cp -r "$AU_SRC" "$AU_DEST/"
    spin_stop
    AU_INSTALLED="$AU_DEST/$(basename "$AU_SRC")"
    ok "AU installed → $AU_INSTALLED"
    AU_SIZE_INST=$(du -sh "$AU_INSTALLED" 2>/dev/null | awk '{print $1}')
    info "  Size: $AU_SIZE_INST"
  else
    warn "AU not found — skipping"
  fi

  # VST3
  if [[ -n "$VST_SRC" ]] && [[ -e "$VST_SRC" ]]; then
    step "Installing VST3..."
    mkdir -p "$VST_DEST"
    OLD_VST=$(find "$VST_DEST" -name "Ancient Synth.vst3" -o -name "AncientSynth.vst3" 2>/dev/null | head -1)
    if [[ -n "$OLD_VST" ]]; then
      spin_start "Removing old VST3..."
      rm -rf "$OLD_VST"
      spin_stop
      info "  Removed: $OLD_VST"
    fi
    spin_start "Copying VST3..."
    cp -r "$VST_SRC" "$VST_DEST/"
    spin_stop
    VST_INSTALLED="$VST_DEST/$(basename "$VST_SRC")"
    ok "VST3 installed → $VST_INSTALLED"
  else
    warn "VST3 not found — skipping"
  fi

  # Clear AU cache AFTER install
  step "Resetting AU registry cache..."
  spin_start "Clearing caches..."
  rm -rf "$HOME/Library/Caches/AudioUnitCache" 2>/dev/null || true
  sudo find /Library/Caches -name "*AudioUnit*" -delete 2>/dev/null || true
  spin_stop
  ok "AU cache cleared — DAWs will rescan on next launch"

  # Optional code signing
  if [[ -n "$SIGN_ID" ]]; then
    echo ""
    step "Code signing..."
    if [[ -n "$AU_INSTALLED" ]] && [[ -e "$AU_INSTALLED" ]]; then
      spin_start "Signing AU..."
      codesign --force --sign "$SIGN_ID" --timestamp "$AU_INSTALLED" 2>&1 || true
      spin_stop
      ok "AU signed"
    fi
    if [[ -n "$VST_INSTALLED" ]] && [[ -e "$VST_INSTALLED" ]]; then
      spin_start "Signing VST3..."
      codesign --force --sign "$SIGN_ID" --timestamp "$VST_INSTALLED" 2>&1 || true
      spin_stop
      ok "VST3 signed"
    fi
  else
    echo ""
    info "  Skipping code signing (local use — no --sign flag)"
    info "  To sign: ./build.sh --sign \"Developer ID Application: Your Name (TEAMID)\""
  fi

  echo ""
  echo -e "  ${DIM}Phase 4 complete in $(section_elapsed)s${RESET}"

else
  banner "PHASE 4 / 5  —  INSTALL"
  info "Skipping install (--no-install flag set)"
  info "Artefacts are in: build/AncientSynth_artefacts/$CONFIG/"
fi

# ══════════════════════════════════════════════════════════════════════════════
#  PHASE 5 — Validate AU
# ══════════════════════════════════════════════════════════════════════════════
banner "PHASE 5 / 5  —  VALIDATE"

if $NO_INSTALL; then
  info "Skipping validation (--no-install mode)"
else
  step "Running Apple AU validation (auval)..."
  info "  Testing: aumu AS30 AnSn"
  echo ""

  spin_start "Validating AU..."
  sleep 0.5   # brief pause for cache to settle
  AUVAL_OUT=$(auval -v aumu AS30 AnSn 2>&1) || true
  spin_stop

  echo "$AUVAL_OUT" | while IFS= read -r line; do
    if echo "$line" | grep -q "VALIDATING\|TESTING\|PASS\|FAIL\|ERROR\|---" 2>/dev/null; then
      case "$line" in
        *"PASS"*)   echo -e "  ${GREEN}$line${RESET}" ;;
        *"FAIL"*|*"ERROR"*|*"FATAL"*)  echo -e "  ${RED}$line${RESET}" ;;
        *"VALIDATING"*) echo -e "  ${CYAN}$line${RESET}" ;;
        *)          echo -e "  ${GREY}$line${RESET}" ;;
      esac
    fi
  done

  echo ""
  if echo "$AUVAL_OUT" | grep -q "AU validation succeeded"; then
    ok "${GREEN}${BOLD}AU VALIDATION PASSED${RESET}"
    ok "Ancient Synth is ready to load in Logic, GarageBand, and MainStage"
  elif echo "$AUVAL_OUT" | grep -q "FAIL\|FATAL ERROR"; then
    warn "AU validation FAILED"
    echo ""
    echo -e "  ${AMBER}Common fixes:${RESET}"
    info "  1. Set Code Signing to '-' in Xcode and rebuild"
    info "  2. Run: rm -rf ~/Library/Caches/AudioUnitCache"
    info "  3. Restart Logic and rescan: Preferences → Plug-in Manager → Reset & Rescan"
  else
    info "auval returned unexpected output — check above"
  fi
fi

echo ""
echo -e "  ${DIM}Phase 5 complete in $(section_elapsed)s${RESET}"

# ══════════════════════════════════════════════════════════════════════════════
#  SUMMARY
# ══════════════════════════════════════════════════════════════════════════════
TOTAL_SECS=$(( $(date +%s) - SCRIPT_START ))

echo ""
echo -e "${AMBER}${BOLD}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
echo ""
echo -e "  ${GREEN}${BOLD}Build complete${RESET}  ${GREY}(total time: ${TOTAL_SECS}s)${RESET}"
echo ""

# Installed locations
if ! $NO_INSTALL; then
  echo -e "  ${BOLD}Installed:${RESET}"
  AU_CHECK=$(find "$HOME/Library/Audio/Plug-Ins/Components" -name "*.component" 2>/dev/null | grep -i "ancient" | head -1)
  VST_CHECK=$(find "$HOME/Library/Audio/Plug-Ins/VST3"      -name "*.vst3"      2>/dev/null | grep -i "ancient" | head -1)
  APP_CHECK="$APP_SRC"
  [[ -n "$AU_CHECK"  ]] && echo -e "  ${TICK}  AU    ${GREY}$AU_CHECK${RESET}"
  [[ -n "$VST_CHECK" ]] && echo -e "  ${TICK}  VST3  ${GREY}$VST_CHECK${RESET}"
  [[ -n "$APP_CHECK" ]] && echo -e "  ${TICK}  App   ${GREY}$APP_CHECK${RESET}"
  echo ""
fi

# Quick actions
echo -e "  ${BOLD}Quick commands:${RESET}"
echo -e "  ${GREY}Open standalone:${RESET}  open \"$APP_SRC\""
echo -e "  ${GREY}Open in Xcode:${RESET}    open build/AncientSynth.xcodeproj"
echo -e "  ${GREY}Rebuild clean:${RESET}    ./build.sh --clean"
echo -e "  ${GREY}Universal binary:${RESET} ./build.sh --universal"
echo ""
echo -e "  ${BOLD}Plugin codes:${RESET}  Manufacturer ${AMBER}AnSn${RESET}  ·  Plugin ${AMBER}AS30${RESET}  ·  Type ${AMBER}aumu${RESET}"
echo ""
echo -e "${AMBER}${BOLD}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
echo ""
