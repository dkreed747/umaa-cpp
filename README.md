# umaa-cpp

A modern C++ library for building UMAA (Unmanned Maritime Autonomy
Architecture) services that communicate over DDS. It provides configuration,
domain types, DDS/UDP IO abstractions, observer pub/sub, health/log services,
and full UMAA implementations (state machines, conditionals, objective
executors, services).

The library links against the UMAA DDS types and CycloneDDS provided by the
[umaa-cyclone-cpp](https://gitlab.bongo-barley.ts.net/poseidon/utility/development-containers/umaa-cyclone-cpp)
development container (`find_package(umaa_cyclone_cpp)` from `/opt/umaa`).

## Artifacts

CI publishes two artifacts from the default branch and tags:

1. **SDK image** (`$CI_REGISTRY_IMAGE`): the umaa-cyclone-cpp dev container
   with the SDK preinstalled at `/opt/umaa-cpp` — `find_package(umaa-cpp)`
   works with zero setup.
2. **SDK package** (Generic Package Registry): `umaa-cpp-<version>.tar.gz`,
   a relocatable install prefix (`lib64/libumaa-cpp.so*`, headers, CMake
   package config, runtime config). Unpack next to the umaa-cyclone-cpp
   package (which bundles the UMAA types and CycloneDDS) and put both on
   `CMAKE_PREFIX_PATH`. `<version>` is the git tag, or the short SHA on
   default-branch builds.

## Consuming the SDK

```cmake
find_package(umaa-cpp CONFIG REQUIRED)
target_link_libraries(my-service PRIVATE umaa-cpp::umaa-cpp)
```

Or build it from source as a git submodule with `add_subdirectory(umaa-cpp)` —
the same `umaa-cpp::umaa-cpp` target name works in both modes.

## Building

Build inside the umaa-cyclone-cpp dev container (open this repo with the
included `.devcontainer/`, or work under `/workspace/projects` in the
container). Presets drive everything:

```bash
cmake --preset dev-debug     # Debug + all tests
cmake --build --preset dev-debug
ctest --preset dev-debug
```

| Preset | Build type | Tests | Build dir |
| --- | --- | --- | --- |
| `dev-debug` | Debug | all | `build/` |
| `dev-release` | Release | none | `build-release/` |
| `ci` | Release | all | `build/` (installs to `install/`) |

CMake options: `UMAA_CPP_BUILD_TESTS`, `UMAA_CPP_BUILD_DDS_TESTS`,
`UMAA_CPP_COVERAGE`, `UMAA_CPP_INSTALL` (default ON), `BUILD_SHARED_LIBS`
(default ON).

## Tests

Three CTest-registered GoogleTest executables (GoogleTest comes from the dev
image via `find_package(GTest)`):

- `umaa-cpp-core-test` — non-UMAA common code (domain, observer, common,
  io/udp, io/dds base).
- `umaa-cpp-umaa-test` — UMAA implementations (state machines, conditionals,
  services, objective executors).
- `umaa-cpp-dds-test` — Cyclone DDS integration tests over loopback
  (`config/cyclonedds-loopback.xml` is wired via the test's `CYCLONEDDS_URI`).

```bash
ctest --preset dev-debug                  # everything
ctest --preset dev-debug -R umaa-cpp-umaa-test
./build/test/umaa-cpp-umaa-test --gtest_filter='ConditionalFactory*'
```

`ci/smoke/` contains a minimal standalone consumer used by CI to prove the
installed package works; it doubles as a usage example.

## Coverage

```bash
cmake --preset dev-debug -DUMAA_CPP_COVERAGE=ON
cmake --build --preset dev-debug && ctest --preset dev-debug
./scripts/run_gcov.sh build   # emits *.gcov into gcov-reports/
```
