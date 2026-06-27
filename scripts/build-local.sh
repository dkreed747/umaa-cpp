#!/bin/bash
#
# Copyright 2025 Pennsylvania State University
#
# Applied Research Laboratory
# Pennsylvania State University
# P.O. Box 30
# State College, PA 16804-0030
#
# DISTRIBUTION STATEMENT A. Approved for public release.
# Distribution is unlimited.
# This software was developed by the Department of the Navy,
# NAVSEA Unmanned and Small Combatants. It is provided under the terms of
# use found in the LICENSE file at the source code root directory.
#
#

# Verify environment is setup otherwise the docker build will fail
if [ -z "${ARTIFACTORY_API_USER}" ] || [ -z "${ARTIFACTORY_API_KEY}" ] ; 
then
    echo "Build environment is not set up correctly"
    echo "Use the following syntax to resolve the issue: "
    echo "export ARTIFACTORY_API_USER=<your_artifactory_api_username>"
    echo "export ARTIFACTORY_API_KEY=<your_artifactory_api_key>"
else
    # Set Variables
    docker build -t umaa-sdk-common:debug-latest -f ci/umaa-sdk-common.Dockerfile .
fi
