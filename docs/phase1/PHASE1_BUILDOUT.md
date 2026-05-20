# GG Doll DAW Phase 1 Buildout

This phase starts implementation of the architecture with a minimal production-oriented C++ core.

## What was started
- Added CMake-based C++20 build entry point (`CMakeLists.txt`).
- Introduced core module boundaries under `src/`:
  - `app/` for use-cases.
  - `domain/` for core entities.
  - `security/` for import validation gates.
- Implemented `ImportAudioUseCase` with two phase-1 security gates:
  - extension allowlist (`.wav`, `.mp3`, `.flac`).
  - canonical workspace path boundary check.
- Added initial unit test binary (`tests/unit/import_validator_test.cpp`).

## Why this first
Import/security is the first irreversible trust boundary in an offline DAW. Starting here enables safe ingest before waveform generation and slicing are wired in.

## Next immediate steps
1. Add magic-byte signature validation for WAV/MP3/FLAC.
2. Add decode facade with bounded-duration limits.
3. Add transient detector module + deterministic slice list generation.
4. Add serialization of project manifest with schema versioning.
