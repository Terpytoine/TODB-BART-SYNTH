# TODB BART SYNTH

**Bay Area Rhythm & Tone — Twon On Da Beat**

A playable Windows x64 VST3 instrument for FL Studio and compatible hosts. 350 presets, editable synth controls, note-responsive BART trains, and six visual environments. No standalone app, sequencer, or MIDI generator.

## Downloads

Download the Windows installer from **Releases**. Close FL Studio before installing, then scan installed plugins and load **TODB BART SYNTH**.

## Ownership

This repository and its releases are maintained by Terpytoine / TODB. Public downloads do not grant write access to this repository, the owner's computer, or the owner's local master source. Others may only modify their own copies without maintainer permission to contribute here.

## Source and sound assets

The complete prepared build-source archive is attached to the release. It contains the native instrument code, prepared UI, and sound assets needed for this build. Original personal reference photos and reference video/audio recordings are excluded. Third-party components retain their individual licenses. No blanket open-source license is granted to sound assets or branding; their respective rights remain in force.

## Version 1.3.1

All 350 preset names, order, and settings are retained. Output is about 3.52 dB louder by default without adding distortion. The editor defaults to 1100×760 and scales down to 900×640. Sequencing and arpeggiated note generation are disabled, including legacy patch settings. MIDI notes play the instrument directly.

## Build from source

Use Windows x64, Node.js and Zig 0.15.2. Run build.ps1 -Zig <path to zig.exe> in this folder. The prepared assets and dependencies are included. The Microsoft Edge WebView2 Runtime is required for the editor.

