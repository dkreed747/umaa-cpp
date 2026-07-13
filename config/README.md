# Configuration Directory

Source runtime configuration for umaa-cpp. The build copies these files into
the build directory at configure time, and `cmake --install` ships them at
`<prefix>/share/umaa-cpp/config/` (the SDK image carries them at
`/opt/umaa-cpp/share/umaa-cpp/config/`).

Projects that build the SDK as a submodule and want the configs next to their
binaries can copy them the same way:

```cmake
# Copy files in config directory to build directory
file(COPY ${CMAKE_CURRENT_LIST_DIR}/umaa-cpp/config/ DESTINATION ${CMAKE_BINARY_DIR})
```

## Files

- `log4cxx.xml`

  Configuration preferences for the log4cxx logger. If this file is not
  present in the working directory the logger falls back on defaults defined
  in `include/Logger.h`.

- `system-config.yml`

  Unified configuration file for defining DDS variables, network properties,
  and vehicle specific configurations.

- `CYCLONE_QOS_PROFILES.xml`

  Defines the default QoS profile used by applications that use DDS for
  communication.

- `cyclonedds-loopback.xml`

  CycloneDDS transport profile for single-host testing: binds to `lo`,
  disables multicast, unicast discovery to localhost. Point `CYCLONEDDS_URI`
  at it (`file://.../cyclonedds-loopback.xml`) — CI and the DDS tests use it
  so DDS traffic never leaves the machine.
