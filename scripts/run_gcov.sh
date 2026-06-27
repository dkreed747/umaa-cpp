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

# Pull build directory location as first parameter
if [ -z $1 ];
then
  echo "+++ BUILD DIR NOT SPECIFIED, DEFAULTING TO: build"
  BUILD_DIR=build

else

  BUILD_DIR=$1

fi

COV_DIR=gcov-reports

find $BUILD_DIR -name "*.o" |tr '\n' ' ' | awk '{print "gcov " $_}' | bash
mkdir -p $COV_DIR
mv -f *.gcov $COV_DIR/.
