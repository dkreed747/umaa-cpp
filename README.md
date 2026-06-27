# Content

[[_TOC_]]

***

# UMAA SDK Common

This Project contains common software elements that are shared and used by multiple software projects.  This is a common repository that provides abstract code which can be re-used by services.

This project will output C++ shared objects and archives to be used for compilation.


# SDK General Usage Overview

To utilize this project, add it as a submodule or compile it separately and link it against the project that wants to use it.


# Build Image Locally

To build the development Docker container, run the following command in the project directory:
```bash
docker build . -t "umaa-sdk-common" -f ./ci/umaa-sdk-common.Dockerfile
```

# Build the Project

1. To build the project, run the following command:
```bash
mkdir build
cd build 
cmake .. -DCMAKE_BUILD_TYPE=Debug -DTEST_COMMON=1 -DTEST_CYCLONE=1
make
```

***

