# Animation Studio PAS

Native Android animation-editor prototype built with C++20, Android NDK, CMake and OpenGL ES 3.

## Current status

This repository is an early engineering scaffold, not a complete animation editor and not a release-ready APK. The current prototype establishes the native Android entry point and starter data models for strokes, keyframes, timeline clips and performance recording.

## Build

Open the project in Android Studio with Android SDK 35 and a compatible Android NDK installed. Gradle will configure the native CMake build. Supported ABIs are arm64-v8a, armeabi-v7a and x86_64.

## Planned milestones

1. Functional canvas: touch/stylus input, pressure/tilt, GPU stroke rendering, eraser, undo/redo.
2. Drawing tools: layers, brush engine, color, selection and project persistence.
3. Animation: frame-by-frame, onion skin, keyframes, interpolation and playback.
4. Performing: record live transforms and edit recorded passes on a timeline.
5. Media and export: image/audio/video import and rendered video export using Android codecs.
6. Testing, performance profiling and signed release builds.

## Architecture

- `app/src/main/cpp`: C++20 native application and engine.
- `engine/drawing`: stroke data model.
- `engine/animation`: keyframe and interpolation model.
- `engine/timeline`: tracks and clips model.
- `engine/perform`: live-performance recording model.

## Important limitations

The current source is a foundation only. Several models are not yet connected to a renderer or interactive editing UI. Stylus support, full brush rendering, complete timeline editing, audio/video workflows, project serialization and APK validation remain to be implemented. No claim of Procreate Dreams feature parity is made.

This is an independent project and does not include proprietary Procreate code, assets or branding.