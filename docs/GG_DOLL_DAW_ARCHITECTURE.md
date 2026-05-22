# GG Doll DAW — Production Architecture Specification (V0.1→V1.0)

## 1) Executive Summary
GG Doll DAW is an offline-first, Linux-first cyberpunk beat-chop workstation optimized for fast iteration, low-latency manipulation, and Steam Deck usability. The product scope focuses on deterministic slicing/remix workflows, not a maximalist DAW clone. The architecture is intentionally layered so real-time audio remains isolated from UI, file I/O, and long-running tasks.

**Primary outcomes:**
- Real-time-safe audio core with deterministic callback behavior.
- Secure import/save pipeline treating all files as untrusted input.
- Modular subsystem boundaries that support VST/LV2/CLAP growth later.
- Steam-ready deployment with reproducible builds and stable performance envelopes.

## 2) Tech Stack Reasoning
### Mandatory core
- **C++20 + JUCE** for mature low-latency audio, cross-platform windowing, plugin ecosystem compatibility, and practical Linux deployability.

### Supporting components
- **CMake + Ninja** for portable deterministic builds.
- **Catch2 / GoogleTest** for unit and integration testing.
- **nlohmann/json + JSON schema validator** for strict project metadata validation.
- **dr_mp3 / libsndfile / minimp3 wrappers** behind hardened decode facade.
- **SQLite (optional hybrid mode)** for indexed project metadata/caches once projects exceed pure JSON scaling limits.

### Why not Electron/web wrappers
- Callback determinism, memory overhead, and input latency constraints conflict with browser-threaded execution and heavyweight runtime costs.

## 3) Full Architecture (Layered)
```text
Presentation Layer
  -> Application Layer
      -> Domain Layer
          -> Audio Engine Layer
              -> Infrastructure Layer
                  -> Security Layer (cross-cutting)
```

### 3.1 Presentation Layer
Owns JUCE Components, timeline widgets, mixer panels, transport UI, stem browser, export dialogs, mascot/console visuals.
- No direct file decoding.
- No direct DSP graph mutation on audio thread.
- Talks to Application layer through command/event interfaces.

### 3.2 Application Layer
Use-case orchestration:
- `ImportAudioUseCase`
- `AutoSliceUseCase`
- `RearrangeSlicesUseCase`
- `ExportMixdownUseCase`
- `SaveProjectUseCase` / `RecoverProjectUseCase`

Handles transactions and thread dispatching (UI thread → worker pool → audio-safe command queue).

### 3.3 Domain Layer
Pure business objects/rules:
- `Project`, `Track`, `Clip`, `Slice`, `Marker`, `TransportState`, `MixerState`.
- Stateless domain services: beat-grid snapping, slice ordering policy, routing constraints.

No JUCE dependencies here except thin value conversions at boundaries.

### 3.4 Audio Engine Layer
Hard real-time graph execution:
- Graph nodes (clips/effects/buses/master)
- Preallocated processing buffers
- Lock-free parameter updates
- Sample-accurate transport synchronization

### 3.5 Infrastructure Layer
- File system adapters, codec wrappers, logging sinks (non-RT only), config storage, cache store, telemetry exporters.

### 3.6 Security Layer (cross-cutting)
- Input validation, signature sniffing, schema guards, path normalization, atomic save/rollback, denial policies.

## 4) Threading Model
### Threads
1. **Audio RT thread**: callback processing only.
2. **UI thread**: rendering + event handling.
3. **Worker pool (N threads)**: decode, waveform generation, transient analysis, export rendering chunks.
4. **I/O thread**: serialized writes (project save/autosave/cache).
5. **Plugin sandbox subprocesses (future)**: isolated scanning/hosting.

### Ownership and data flow
- UI emits commands -> `ApplicationCommandBus`.
- Application transforms to immutable deltas.
- Deltas copied into lock-free SPSC queue for audio thread.
- Audio thread applies deltas at block boundary.
- Metering/transport stats return through lock-free ring buffer to UI.

## 5) Audio Engine Design
### Audio callback guarantees
Inside callback:
- No malloc/new.
- No locks (or only wait-free atomics).
- No filesystem/net access.
- No logging.

### Processing lifecycle
1. Pull pending command deltas (lock-free).
2. Resolve transport frame range.
3. Traverse active graph nodes in topological order.
4. Mix buses, process FX chain, hard-limit safety on master.
5. Push meter snapshots to UI queue.

### Core interfaces
```cpp
struct AudioBlockView { float** channels; int numChannels; int numSamples; };

class IAudioNode {
public:
  virtual ~IAudioNode() = default;
  virtual void prepare(double sampleRate, int maxBlockSize) = 0;
  virtual void process(AudioBlockView block) noexcept = 0; // RT-safe
  virtual void reset() noexcept = 0;
};

class IRealtimeCommand {
public:
  virtual ~IRealtimeCommand() = default;
  virtual void apply(class EngineState&) noexcept = 0;
};
```

## 6) DSP Graph Design
### Bus topology
- Track input -> per-track FX -> track gain/pan -> stem bus
- Multiple stems -> optional group bus
- Group buses -> master bus (EQ -> comp -> limiter)

### Latency strategy
- V0.1: zero-latency internal DSP focus.
- V0.4+: latency reporting API + compensation map in transport scheduler.

### Parameter automation
- Control-rate automation lanes resampled per block with optional sample-ramp smoothing.

## 7) Beat Slicing Engine Design
Pipeline:
1. Decode PCM (worker thread).
2. Envelope + spectral flux onset detection.
3. Adaptive thresholding by local RMS window.
4. Quantize candidates to beat-grid if enabled.
5. Generate slice objects with immutable source references.

Operations:
- Reorder: index remap only (non-destructive).
- Reverse: playback direction flag.
- Pitch shift: elastique-like abstraction (simple resample/preserve mode initially).
- Stretch: transient-preserving WSOLA-lite (MVP simplified).

## 8) UI Rendering Strategy
- JUCE retained widgets for controls.
- GPU-assisted waveform raster cache (optional OpenGL/Vulkan backend toggle later).
- Dirty-rect repaints; avoid full timeline redraw.
- Decouple UI FPS from audio callback rate.
- Steam Deck mode: larger controls, high-contrast neon accents, controller focus graph.

## 9) Security Architecture
## 9.1 Threat model
Attack surfaces:
- Imported audio/presets/projects.
- Corrupt metadata (ID3/RIFF chunks).
- Malicious plugin binaries (future).

## 9.2 Mitigations
- Strict extension allowlist + magic-byte signature check.
- Decode in bounded parser adapters with length checks.
- Canonical path checks preventing `../` traversal.
- Schema version validation and required field enforcement.
- Atomic save (`.tmp` + fsync + rename).
- Autosave journal with last-known-good checkpoint.

## 9.3 Dependency security
- Pinned versions.
- CI static analysis (clang-tidy, cppcheck).
- ASAN/UBSAN/TSAN jobs.
- Fuzz harnesses for importer/parser entrypoints.

## 10) Repository Structure
```text
GGDollDAW/
  src/
    app/            # application use cases + orchestration
    domain/         # entities + domain rules
    audio/          # RT engine, DSP graph, mixer
    ui/             # JUCE components, themes, layout
    infra/          # codecs, filesystem, persistence adapters
    security/       # validators, schema, policy gates
  tests/
    unit/
    integration/
    dsp_regression/
    fuzz/
    perf/
  docs/
  assets/
  ci/
  tools/
  third_party/
  presets/
  cache/
  logs/
```
Dependency direction: `ui -> app -> domain`, `audio` depends on `domain` contracts only, `infra/security` injected into `app`.

## 11) Save / Autosave Design
### Project format recommendation: hybrid
- **Human-readable JSON manifest** for project topology/state.
- **Binary sidecar(s)** for large automation/waveform indexes.
- Optional SQLite index for large projects in V0.3+.

### Save safety
- Write temp file + checksum + durable rename.
- Autosave ring (e.g., 10 snapshots) with crash recovery chooser.
- Versioned migration registry: `vN -> vN+1` pure transforms.

## 12) Waveform Cache Design
- Build multi-resolution min/max pyramids per clip.
- Key = hash(audio path + file mtime + decode params).
- Cache entries memory-mapped on non-RT threads only.
- UI pulls pre-decimated bins, never raw full-resolution scans.

## 13) Plugin Architecture (Future-Compatible)
V0.x internal DSP-first. Prepare abstraction:
```cpp
class IPluginProcessor {
public:
  virtual ~IPluginProcessor() = default;
  virtual bool initialize(double sr, int maxBlock) = 0;
  virtual void process(AudioBlockView) noexcept = 0;
  virtual int latencySamples() const noexcept = 0;
};
```
Future hosting:
- Out-of-process scanner.
- Signed/known plugin policy + denylist.
- Crash/time-out quarantine.

## 14) Performance Optimization Plan
- SIMD kernels (SSE/AVX on desktop; NEON on Steam Deck CPU where applicable).
- SoA-friendly buffer layouts in hot DSP paths.
- Preallocated voice/effect pools.
- Work stealing on analysis/export threads only (never RT path).
- Profiling cadence: Tracy + perf + valgrind massif + cachegrind.
- Telemetry (local-only): callback overrun counters, XRuns, peak RAM, render times.

## 15) Steam Deck Optimization Strategy
- 1280x800 and 800p-first layout profiles.
- Controller mapping layer (transport, timeline zoom, clip select, slice nudge).
- Touch-first hit targets >= 44 px logical.
- Dynamic quality knobs for waveform detail and UI effects.
- Power-aware mode limiting background analysis concurrency.

## 16) Testing / CI Strategy
### Test matrix
- Unit tests: domain math, command routing, validators.
- Integration: import->slice->arrange->export flows.
- DSP regression: golden-file null/error tolerance diffs.
- Serialization tests: forward/back compatibility fixtures.
- Crash recovery tests: forced kill during autosave.
- Fuzz tests: RIFF/MP3/JSON parsers.
- Perf benchmarks: callback CPU %, slice latency, export throughput.

### CI pipeline
1. Format/lint
2. Static analysis
3. Debug tests + sanitizers
4. Release build + perf smoke
5. Artifact packaging

## 17) Deployment Strategy
- Linux AppImage + Steam runtime packaging.
- Deterministic build metadata embed (git SHA, compiler flags).
- Offline installer assets bundled locally.
- Post-install self-test for audio device/permissions.

## 18) Engineering Tradeoffs
- **JSON-only vs hybrid**: JSON is simple/diffable; hybrid prevents giant manifests and improves load times.
- **Real-time purity vs feature speed**: strict callback rules slow rapid prototyping but prevent non-deterministic glitches.
- **Internal DSP first vs plugin first**: internal first enables consistent UX/perf; plugin hosting deferred for security hardening.

## 19) Pseudocode / Examples
### 19.1 RT-safe command handoff
```cpp
// UI/Application thread
auto cmd = ReorderSliceCommand{trackId, oldIndex, newIndex};
realtimeQueue.try_enqueue(cmd); // preallocated object pool

// Audio thread
ReorderSliceCommand cmd;
while (realtimeQueue.try_dequeue(cmd)) {
    cmd.apply(engineState);
}
```

### 19.2 Import security gate
```cpp
Result<DecodedAudio> ImportAudioUseCase::run(Path p) {
    if (!security.extensionAllowed(p)) return Err("ext_not_allowed");
    if (!security.magicMatches(p)) return Err("signature_mismatch");
    auto safePath = security.canonicalizeInsideWorkspace(p)?;
    return codecFacade.decodeBounded(safePath, DecodeLimits{.maxSeconds=3600});
}
```

### 19.3 Atomic save
```cpp
Result<void> ProjectStore::saveAtomically(const Project& pj, Path file) {
    auto tmp = file.string() + ".tmp";
    writeAll(tmp, serialize(pj));
    fsync(tmp);
    rename(tmp, file);
    return Ok();
}
```

## 20) MVP Implementation Roadmap
### V0.1 (Beat-chop core)
- WAV/MP3 import, waveform generation, transient slicing, timeline arrangement, mixer basics, WAV/MP3 export.

### V0.2
- Project save/load, autosave/recovery, core DSP set (EQ/Comp/Limiter/Delay/Reverb/Distortion/Bitcrusher).

### V0.3
- Local stem separation integration point, render cache optimization, project capsule portability.

### V0.4
- Plugin sandbox prototype, advanced slicing algorithms, latency mapping groundwork.

### V1.0
- Hardened crash recovery, full Steam Deck UX polish, performance budget lock, release QA certification.
