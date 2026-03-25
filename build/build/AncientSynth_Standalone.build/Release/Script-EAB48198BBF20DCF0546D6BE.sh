#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Volumes/Rocket_XTRM/Projects/AncientSynth/build
  /Volumes/Rocket_XTRM/Projects/AncientSynth/build/juce_build/tools/extras/Build/juceaide/juceaide_artefacts/Custom/juceaide pkginfo "Standalone Plugin" /Volumes/Rocket_XTRM/Projects/AncientSynth/build/AncientSynth_artefacts/JuceLibraryCode/AncientSynth_Standalone/PkgInfo
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Volumes/Rocket_XTRM/Projects/AncientSynth/build
  /Volumes/Rocket_XTRM/Projects/AncientSynth/build/juce_build/tools/extras/Build/juceaide/juceaide_artefacts/Custom/juceaide pkginfo "Standalone Plugin" /Volumes/Rocket_XTRM/Projects/AncientSynth/build/AncientSynth_artefacts/JuceLibraryCode/AncientSynth_Standalone/PkgInfo
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Volumes/Rocket_XTRM/Projects/AncientSynth/build
  /Volumes/Rocket_XTRM/Projects/AncientSynth/build/juce_build/tools/extras/Build/juceaide/juceaide_artefacts/Custom/juceaide pkginfo "Standalone Plugin" /Volumes/Rocket_XTRM/Projects/AncientSynth/build/AncientSynth_artefacts/JuceLibraryCode/AncientSynth_Standalone/PkgInfo
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Volumes/Rocket_XTRM/Projects/AncientSynth/build
  /Volumes/Rocket_XTRM/Projects/AncientSynth/build/juce_build/tools/extras/Build/juceaide/juceaide_artefacts/Custom/juceaide pkginfo "Standalone Plugin" /Volumes/Rocket_XTRM/Projects/AncientSynth/build/AncientSynth_artefacts/JuceLibraryCode/AncientSynth_Standalone/PkgInfo
fi

