# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

This is **`umaa-cpp`**, a modern C++ library (currently C++23, moving to C++20 — see "Project
state" below) providing reusable abstractions for building UMAA
(Unmanned Maritime Autonomy Architecture) services that communicate over DDS. It produces
shared/static libraries consumed by downstream service projects as a git submodule or linked
dependency.

The current libraries (names predate the rename — see "Project state" below):

- **`umaa-sdk-common`** — the full common library (config, domain types, DDS/UDP IO, observer,
  health/log services, plus all UMAA implementations).
- **`umaa++`** — a narrower library of just the UMAA C++ implementations (state machines,
  objective executors, services).

## Project state & direction

> **Read this before making structural changes.** This repo is mid-transition. Much of what
> the rest of this file describes is the *current* (legacy) state; the items below are the
> *intended* direction. When a task touches these areas, move toward the target architecture
> rather than entrenching the old one — but ask if a change would be large or ambiguous.

This project was previously `umaa-sdk-common` and is being reframed and renamed to **`umaa-cpp`**,
a broader vision than a "common" grab-bag library.

**First branch/MR (foundation — minimal code changes):** vendor the UMAA IDLs + generate types,
add the devcontainer, and perform the rename. This MR lays the development foundation rather than
changing behavior. Decided toolchain/runtime targets for this work:

- **Base/runtime image: UBI 10** (Red Hat Universal Base Image) — replaces the external
  `umaa-cyclone-cxx-types` dev image.
- **Compiler: GCC 15 toolset.**
- **Language standard: C++20** (stepping *down* from the current C++23) for a stable-but-modern
  production library. Note `CMakeLists.txt` still sets `CMAKE_CXX_STANDARD 23` — moving it to 20 is
  part of this work; avoid introducing C++23-only features in new code.

Known in-flight and planned work:

- **Rename**: `umaa-sdk-common` → `umaa-cpp`. Identifiers, target names (`umaa-sdk-common`,
  `umaa++`), image names, and docs still use the old name; treat the rename as ongoing, not done.
- **Vendor the UMAA IDLs into this repo** under `idl/UMAA/...`, removing the hard dependency on
  the external `umaa-cyclone-cxx-types` base image for the generated types. The build will
  generate the CXX types from these IDLs instead of consuming them from `/usr/local/umaa-6.0`.
- **`.devcontainer/devcontainer.json`**: add a cross-platform dev container so contributors on
  any OS can spin up a fully-provisioned environment (DDS, codegen, toolchain) with one command.
  This replaces the current "must run inside a specific Linux image" friction.
- **Middleware abstraction across DDS vendors**: extend the IO layer to switch intelligently
  between **RTI Connext DDS** and **Cyclone DDS**. Intended CMake behavior: detect an RTI Connext
  installation first and build against it; otherwise fall back to Cyclone. The existing
  `ReaderBase`/`SenderBase` interfaces and `io/dds/cyclone` impls are the seam for this — new work
  should add a parallel vendor impl behind the same abstractions, not special-case Cyclone.
- **DDS listener support**: add listener/callback-driven IO (alongside the current poll/read model)
  for low-latency command and control.
- **Mission management as a behavior tree**: a longer-term rework where the current objective /
  objective-executor paradigm collapses into behavior-tree **leaf nodes**, and the mission manager
  becomes a C++ behavior-tree runtime with DDS hooks for command & control, monitoring, planning, etc.
- **Plugin architecture**: enable users to add new UMAA services, behaviors, and autonomies as
  plugins for faster extensibility.

### Current build dependency (legacy, being removed)

Today this repo builds **only on Linux** inside a prebuilt dev Docker image and depends on
system-installed CycloneDDS-CXX, Log4CXX, GeographicLib, and the **generated UMAA Cyclone CXX IDL
types** (`umaa-cyclone-cxx-types`, headers under `/usr/local/umaa-6.0`) supplied by that base image.
A native Windows/macOS build does not currently work. The devcontainer + vendored-IDL + multi-vendor
work above is specifically aimed at removing these constraints, so prefer solutions that move in
that direction over ones that deepen the reliance on the external image or on Cyclone-only paths.

## Build & test

The build is driven by CMake with three feature options (all default OFF):

- `TEST_COMMON` — builds the `umaa_sdk_common_test` and `umaa_cxx_test` executables.
- `TEST_CYCLONE` — builds `umaa_sdk_cyclone_test` (needs a live Cyclone DDS environment).
- `CODE_COVERAGE` — adds `-fprofile-arcs -ftest-coverage` and links `gcov`.

Standard build + test (run inside the dev container):

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DTEST_COMMON=1 -DTEST_CYCLONE=1
make -j"$(nproc)"
ctest --verbose
```

Build the dev/CI Docker image locally:

```bash
docker build . -t umaa-sdk-common -f ./ci/umaa-sdk-common.Dockerfile
```

### Running tests

Tests use GoogleTest (fetched at configure time via `cmake/CMakeLists.txt.gtest.in`). There are
three test executables registered with CTest:

- `umaa_sdk_common_test` — non-UMAA common code (domain, observer, common, io/udp, io/dds base).
- `umaa_cxx_test` — UMAA implementations (state machines, conditionals, services, objective executors).
- `umaa_sdk_cyclone_test` — Cyclone DDS integration tests (only with `-DTEST_CYCLONE=1`).

```bash
# Run one test executable directly with a gtest filter
cd build
./test/umaa_cxx_test --gtest_filter='ConditionalFactory*'

# Run all CTest tests, or just one by name
ctest --verbose
ctest -R umaa_cxx_test --verbose

# Stress / flake detection: run the full ctest suite 20x into testResults.txt
../scripts/runTestXTimes.sh
```

Adding a new test file requires editing `test/CMakeLists.txt` — source files are listed
explicitly in the relevant `add_executable(...)` block (there is no globbing).

### Coverage

```bash
cmake .. -DTEST_COMMON=1 -DTEST_CYCLONE=1 -DCODE_COVERAGE=1
make -j"$(nproc)" && ctest
../scripts/run_gcov.sh build   # emits *.gcov into gcov-reports/
```

## Lint / format

- **Formatting**: `.clang-format` (Google-based). **Line length is 120**, not 80.
- **clang-tidy**: `.clang-tidy` enables a curated bugprone/cert/cppcoreguidelines/modernize/
  performance/readability set (all-off baseline, opt-in checks). Honor `// NOLINT` markers.
- **cpplint**: configured via `CPPLINT.cfg` (`linelength=120`, `-build/c++11` filtered out). CI runs:
  `cpplint --filter=-builder/c++11 --linelength=120 --recursive include src`.

## Architecture

Everything lives under the `arlcore` namespace family (e.g. `arlcore::io`, `arlcore::umaa`,
`arlcore::umaa::conditional`, `arlcore::umaa::domain`, `arlcore::env`; algorithms use
`arl::algorithm`). Headers are in `include/`, implementations in `src/` mirroring the same tree.

The big-picture layers (see the per-directory `README.md` files — `include/umaa/README.md`
and `include/umaa/services/base/README.md` are the most important):

- **IO / transport (`include/io`)** — `ReaderBase` / `SenderBase` are pure-virtual transport
  interfaces. Concrete DDS impls live under `io/dds/cyclone` (`CycloneReader`, `CycloneSender`,
  `CycloneBufferedReader`). `BufferedReaderBase` / `ObservableReader` add buffering and
  observer hooks. `IOReaderRegistry` / `IOWriterRegistry` are type-erased (`std::type_index`)
  registries mapping topic strings to readers/senders. UDP transport is under `io/udp`.

- **Observer (`include/observer`)** — a `Subject` / `Observer` pub-sub used to fan out the
  latest samples of "Situational Awareness" report types to consumers (notably conditionals).

- **UMAA services (`include/umaa/services`)** — the core service abstraction. `ReportConsumer`
  / `ReportProvider` are thin read-latest / send wrappers over a reader/sender. `CommandConsumerBase`
  / `CommandProviderBase` model the full UMAA command flow: a provider is driven by a periodic
  `cycle()` call and customized by overriding `on*()` lifecycle hooks (`onIssued`, `onCommanded`,
  `onExecuting`, `onCompleted`, ...) plus validation predicates (`isCommandValid`,
  `isCommandCompleted`, `isCommandFailed`). Providers have a configurable behavior profile
  (`CANCEL_EXISTING` / `COMPLETE_EXISTING` / `REJECT_INCOMING`) for handling overlapping commands.

- **State machines (`include/umaa`)** — `CommandStateMachine`, `TaskStateMachine`, and
  `objective-executors/core/ObjectiveStateMachine` encode UMAA command/task/objective lifecycle states.

- **Conditionals (`include/umaa/conditionals`)** — a polymorphic conditional system. `ConditionalBase`
  is the abstract evaluable type; `ConditionalFactory` (fed a `ConditionalFactoryIo`) builds
  specialized conditionals (Depth, Speed, HeadingSector, logical AND/OR/NOT, etc.) from generalized
  UMAA `ConditionalType` samples, resolving dependencies and wiring observers automatically.
  `ConditionalReportConsumer` turns a Conditional Report (+ its Large Set) into evaluable conditionals.

- **Specializations & caches (`include/umaa/specializations`, `SpecializationCache`)** — UMAA
  "specialization" types are carried generically and resolved by `specializationReferenceID`.
  `SpecializationCache` continuously polls a reader and maintains a lookup table keyed by that ID;
  `ObjectiveSpecFactory` and the `SendableSpecialization` hierarchy handle building/sending them.

- **Large Collections (`include/umaa/Large{List,Set}*`)** — helpers for UMAA "Large List/Set"
  patterns where a collection is published as element topics plus a metadata signal. The `*Writer`
  classes manage element publishing and metadata; the `*Reader` classes aggregate all elements of
  a type through a single shared reader and reconstruct lists/sets on metadata update. Read the
  caveats in `include/umaa/README.md` (manual disposal, read-on-metadata-update, unfiltered storage,
  and `ElementHasher` specialization for sets).

- **Objective executors (`include/umaa/objective-executors`)** — `ObjectiveController`,
  `ObjectiveBase`/`ObjectiveFactory`, and the global Hover/Vector/Waypoint control service consumers
  plus the ObjectiveExecutor control/state-control providers.

- **Domain types (`include/domain`)** — value/covariance/pose/velocity wrappers around UMAA data.

- **Config (`include/config`, `config/`)** — `ConfigurationManager` / `AppConfig` load
  `config/system-config.yml` (DDS, network, vehicle config) via the vendored `third-party/yaml-cpp`.
  Runtime also uses `config/CYCLONE_QOS_PROFILES.xml` and `config/log4cxx.xml`, which CMake copies
  into the build dir. `include/env/Env.h` provides templated `getEnv<T>()` helpers.

## Key conventions

- C++23 today, targeting **C++20** (see "Project state"); don't add C++23-only features in new code.
  `-fPIC -fpermissive`. Header guards are full-path uppercased (e.g. `INCLUDE_ENV_ENV_H_`).
- Cyclone DDS union/discriminator manipulation is non-obvious — see `include/io/dds/cyclone/README.md`
  for the discriminator/union-subtype instantiation idiom.
- Geographic math formulas (projection, lat/long shift, heading, haversine) are documented in
  `include/algorithms/README.md`.
- Every source file carries the PSU/NAVSEA copyright + Distribution Statement A header block.

## CI

GitLab CI (`.gitlab-ci.yml`) runs against a shared RAIL template and the `umaa-cyclone-cxx-types`
dev image: stages are secret-detection → lint (cpplint) → build → test (ctest, JUnit reports) →
static-analysis (SonarQube + gcov) → containerize → publish. A push to the default branch triggers
`UpdateParentProjects.py`, which bumps this submodule in downstream parent projects.
