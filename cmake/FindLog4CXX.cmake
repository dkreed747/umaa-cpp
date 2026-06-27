#---------------------------------------------------------------------------
# Applied Research Laboratory
# Pennsylvania State University
# P.O. Box 30
# State College, PA 16804-0030
#
# Copyright 2022 The Pennsylvania State University
#---------------------------------------------------------------------------

find_path(LOG4CXX_INCLUDE_DIR log4cxx/log4cxx.h
  /usr/include
)

find_library(LOG4CXX_LIBRARY log4cxx
  HINTS /usr/lib/x86_64-linux-gnu /usr/lib
)

message("Found LOG4CXX_LIBRARY: ${LOG4CXX_LIBRARY}")

if(LOG4CXX_LIBRARY)
  add_library(LOG4CXX SHARED IMPORTED)
  set_target_properties(LOG4CXX PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION ${LOG4CXX_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES "${LOG4CXX_INCLUDE_DIR}"
    IMPORTED_SO_NAME 1
  )

endif()
