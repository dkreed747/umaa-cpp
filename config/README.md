# Configuration Directory

The common config directory is the location that source config files are to be placed. The files will then be copied as a post build event to the binary source directory in projects that use umaa-sdk-common

## Usage

  To use config files defined in rail common in your project, you need to copy the config directory to $CMAKE_BINARY_DIR.

  To do this append this code to the CMakeLists.txt file for your project:

  ```cmake
  # Copy files in config directory to build directory
  file(COPY ${CMAKE_CURRENT_LIST_DIR}/umaa-sdk-common/config/ DESTINATION ${CMAKE_BINARY_DIR})
  # Make config files dependencies so cmake knows to recopy the files if they are edited
  add_custom_target(copy_configs ALL DEPENDS ${CMAKE_BINARY_DIR})
  ```

  You will also need to add the library you want to create a dependency with the configs. To do so, use the following code and replace the name of the library with one in your project.

  ```cmake
  add_dependencies(<NAME_OF_LIBRARY> copy_configs)
  ```

## Files

- `log4cxx.xml`

  This xml file stores our configuration preferences for the log4cxx logger. If this file is not present the logger will fallback on defaults defined in umaa-sdk-common/include/Logger.h.


- `system-config.yml`

  Unified configuration file for defining DDS variables, network properties, and vehicle specific configurations

- `CYCLONE_QOS_PROFILES.xml`

  Defines the default qos profile that is used with applications that use DDS for communication