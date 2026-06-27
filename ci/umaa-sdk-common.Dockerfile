#---------------------------------------------------------------------------
#  Copyright 2025 Pennsylvania State University
#
#  Applied Research Laboratory
#  Pennsylvania State University
#  P.O. Box 30
#  State College, PA 16804-0030
#
# DISTRIBUTION STATEMENT A. Approved for public release.
# Distribution is unlimited.
# This software was developed by the Department of the Navy,
# NAVSEA Unmanned and Small Combatants. It is provided under the terms of
# use found in the LICENSE file at the source code root directory.
#---------------------------------------------------------------------------

ARG SDK_DOCKER_REGISTRY="registry.partybarge.il5.blackpearl.us/peo-usc/pms406/rail/projects/umaa/umaa-sdk"

ARG BASE_IMAGE="umaa-cyclone-cxx-types/umaa-cyclone-cxx-types/dev"
ARG BASE_IMAGE_TAG="latest"

## Base Container to be used, Copy in dependencies
FROM $SDK_DOCKER_REGISTRY/$BASE_IMAGE:$BASE_IMAGE_TAG as umaa-base
 
## Build Stage
FROM umaa-base as build-code

WORKDIR /app

# Copy source files
COPY third-party ./third-party
COPY CPPLINT.cfg .
COPY sonar-project.properties .
COPY cmake ./cmake
COPY config ./config
COPY CMakeLists.txt .
COPY include ./include
COPY scripts ./scripts
COPY src ./src
COPY test ./test
COPY test-utils ./test-utils

WORKDIR /app/build

RUN cmake .. -DTEST_COMMON=1 -DTEST_CYCLONE=1 \
  && make -j"$(nproc)" \
  && ctest --verbose



