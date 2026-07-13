# syntax=docker/dockerfile:1
#
# umaa-cpp SDK image
# ---------------------------------------------------------------------------
# Base:  the umaa-cyclone-cpp development container (UBI 10 + gcc-toolset-15 +
#        CycloneDDS at /opt/cyclonedds + UMAA types at /opt/umaa).
# Adds:  the umaa-cpp SDK installed at /opt/umaa-cpp, find_package(umaa-cpp)-
#        ready for projects under /workspace/projects with zero setup.
#
# The published package tarball is `tar -C /opt/umaa-cpp .` of this image's
# SDK prefix, so the package and the image can never drift.
#
# Tests are NOT run here -- the CI test job runs the full suite in the dev
# image before this image is built.
# ---------------------------------------------------------------------------

ARG BASE_IMAGE=gitlab.bongo-barley.ts.net/poseidon/utility/development-containers/umaa-cyclone-cpp:latest

# ===========================================================================
# Stage: build + install the SDK -> /opt/umaa-cpp
# Narrow COPY set keeps unrelated edits (docs, CI, devcontainer) from
# invalidating this expensive layer.
# ===========================================================================
FROM ${BASE_IMAGE} AS sdk-build
USER root
WORKDIR /src/umaa-cpp
COPY CMakeLists.txt CMakePresets.json ./
COPY cmake/ cmake/
COPY include/ include/
COPY src/ src/
COPY config/ config/
# Single RUN: configure + build + install + clean, so the layer (and the
# pushed kaniko cache) holds only /opt/umaa-cpp, not the Ninja build tree.
RUN cmake --preset ci \
        -DCMAKE_INSTALL_PREFIX=/opt/umaa-cpp \
        -DUMAA_CPP_BUILD_TESTS=OFF \
        -DUMAA_CPP_BUILD_DDS_TESTS=OFF \
    && cmake --build --preset ci \
    && cmake --install build \
    && rm -rf build

# ===========================================================================
# Final image: dev container + preinstalled SDK.
# ===========================================================================
FROM ${BASE_IMAGE} AS final
ARG USERNAME=dev
USER root
ENV UMAA_CPP_HOME=/opt/umaa-cpp
COPY --from=sdk-build /opt/umaa-cpp ${UMAA_CPP_HOME}
ENV CMAKE_PREFIX_PATH=${UMAA_CPP_HOME}:${CMAKE_PREFIX_PATH}
RUN printf '%s\n' "${UMAA_CPP_HOME}/lib" "${UMAA_CPP_HOME}/lib64" \
        > /etc/ld.so.conf.d/umaa-cpp.conf \
    && ldconfig

USER ${USERNAME}
WORKDIR /workspace/umaa-cpp

CMD ["/bin/bash"]
