# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

This is **`umaa-cpp`**, a modern C++20 library providing reusable abstractions for building UMAA
(Unmanned Maritime Autonomy Architecture) services that communicate over DDS. It builds one
library, **`umaa-cpp`** (linked as `umaa-cpp::umaa-cpp`), covering config, domain types, DDS/UDP
IO, observer pub/sub, health/log services, and all UMAA implementations (state machines,
conditionals, objective executors, services). Downstream projects consume it either as a git
submodule (`add_subdirectory`) or as an installed package (`find_package(umaa-cpp)`) — the target
name is identical in both modes.

The UMAA DDS types and CycloneDDS come from the **umaa-cyclone-cpp** development container
(`find_package(umaa_cyclone_cpp)` resolves against `/opt/umaa`); the SDK does not generate types
itself. Other dependencies (yaml-cpp, GeographicLib, log4cxx, libuuid, GoogleTest) are also
provided by that image.

## Project state & direction

The `umaa-sdk-common` → `umaa-cpp` transition is **done**: renamed project/targets, UBI 10 +
gcc-toolset-15 + C++20 toolchain, `.devcontainer/`, CMake presets, install/export package config,
and a GitLab CI pipeline publishing an SDK image and a package tarball. The former `umaa-sdk-common`
and `umaa++` targets were consolidated into the single `umaa-cpp` library (`umaa++` was a strict
subset compiled twice).

Planned future work:

- **Pathed includes**: headers are currently included flat (`#include "AppConfig.h"`), so every
  `include/` subdirectory is exported on the target interface. Migrating consumers to
  `#include <umaa-cpp/config/AppConfig.h>` style is a planned whole-tree cleanup.
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

## Build & test

Build inside the umaa-cyclone-cpp dev container (`.devcontainer/`, or under
`/workspace/projects` in the shared container). CMake presets drive everything:

```bash
cmake --preset dev-debug      # Debug, all tests on
cmake --build --preset dev-debug
ctest --preset dev-debug
```

Presets: `dev-debug` (Debug + tests, `build/`), `dev-release` (Release, `build-release/`),
`ci` (Release + tests, `build/`, installs to `install/`).

Options (all prefixed, defaults in parens): `UMAA_CPP_BUILD_TESTS` (OFF), `UMAA_CPP_BUILD_DDS_TESTS`
(OFF), `UMAA_CPP_COVERAGE` (OFF), `UMAA_CPP_INSTALL` (ON), `BUILD_SHARED_LIBS` (ON).

The SDK image is built by CI from `Dockerfile` (dev image + `/opt/umaa-cpp`); the package tarball
is `tar -C /opt/umaa-cpp .` of the image contents.

### Running tests

Tests use GoogleTest from the dev image (`find_package(GTest)` — no network fetch). Three test
executables registered with CTest:

- `umaa-cpp-core-test` — non-UMAA common code (domain, observer, common, io/udp, io/dds base).
- `umaa-cpp-umaa-test` — UMAA implementations (state machines, conditionals, services, objective executors).
- `umaa-cpp-dds-test` — Cyclone DDS integration tests (only with `UMAA_CPP_BUILD_DDS_TESTS=ON`);
  runs over loopback via `config/cyclonedds-loopback.xml`, wired through the test's
  `CYCLONEDDS_URI` environment property.

```bash
# Run one test executable directly with a gtest filter
./build/test/umaa-cpp-umaa-test --gtest_filter='ConditionalFactory*'

# Run all CTest tests, or just one by name
ctest --preset dev-debug
ctest --preset dev-debug -R umaa-cpp-umaa-test

# Stress / flake detection: run the full ctest suite 20x into testResults.txt
../scripts/runTestXTimes.sh
```

Adding a new test file requires editing `test/CMakeLists.txt` — source files are listed
explicitly in the relevant `add_executable(...)` block (there is no globbing).

`ci/smoke/` is a minimal standalone consumer (`find_package(umaa-cpp)` + loopback DDS round-trip)
used by the CI package/integration jobs; keep it building when changing the public surface.

### Coverage

```bash
cmake --preset dev-debug -DUMAA_CPP_COVERAGE=ON
cmake --build --preset dev-debug && ctest --preset dev-debug
../scripts/run_gcov.sh build   # emits *.gcov into gcov-reports/
```

## Lint / format

- **Formatting**: `.clang-format` (Google-based). **Line length is 120**, not 80.
- **clang-tidy**: `.clang-tidy` enables a curated bugprone/cert/cppcoreguidelines/modernize/
  performance/readability set (all-off baseline, opt-in checks). Honor `// NOLINT` markers.
- **cpplint**: configured via `CPPLINT.cfg` (`linelength=120`, `-build/c++11` filtered out):
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
  `config/system-config.yml` (DDS, network, vehicle config) via the dev image's yaml-cpp.
  Runtime also uses `config/CYCLONE_QOS_PROFILES.xml` and `config/log4cxx.xml`, which CMake copies
  into the build dir (and installs to `share/umaa-cpp/config/`). `include/env/Env.h` provides
  templated `getEnv<T>()` helpers.

- **Test utilities (`test-utils/`)** — `umaa-cpp::test-utils` (INTERFACE): `LocalReaderSender`
  in-memory loopback IO plus gmock mocks. Build-tree only, deliberately not installed; downstream
  projects that build the SDK as a submodule may link it for their own tests.

## Key conventions

- **C++20** (`target_compile_features(... cxx_std_20)`, exported to consumers); no `-fpermissive`.
  Header guards are full-path uppercased (e.g. `INCLUDE_ENV_ENV_H_`).
- Template members meant to be used by consumers/tests must be defined (or their explicit
  specializations declared) in headers — symbols emitted only as implicit instantiations inside a
  `.cpp` vanish at `-O3` (this bit `ConditionalFactory::createConditional` and
  `ConditionalReportProvider::getTopicAndWriter` during the port).
- Cyclone DDS union/discriminator manipulation is non-obvious — see `include/io/dds/cyclone/README.md`
  for the discriminator/union-subtype instantiation idiom.
- Geographic math formulas (projection, lat/long shift, heading, haversine) are documented in
  `include/algorithms/README.md`.
- Every source file carries the PSU/NAVSEA copyright + Distribution Statement A header block.

## CI

GitLab CI (`.gitlab-ci.yml`): `test → image → package → integration → publish`.

- `build-test` runs the full ctest suite (JUnit to MR widgets) in the umaa-cyclone-cpp dev image;
  it gates everything downstream.
- `build-image` (Kaniko) builds `Dockerfile` → SDK image (`:short-sha` + `:ref-slug` always;
  `:latest` on the default branch; `:tag` + `:latest` on tags). `BASE_IMAGE_TAG` (default
  `latest`) selects the dev-image tag — override it per-pipeline while testing against an
  unmerged dev-container branch.
- `package` builds `ci/smoke` inside the SDK image with zero setup and tars `/opt/umaa-cpp`.
- `integration` proves the tarball standalone in the plain dev image.
- `publish` (default branch / tags) uploads to the generic package registry as
  `umaa-cpp-<tag-or-short-sha>.tar.gz`.

Cross-project note: pulling the dev image with `CI_JOB_TOKEN` requires this project on the
umaa-cyclone-cpp project's job-token allowlist (Settings → CI/CD → Job token permissions).
