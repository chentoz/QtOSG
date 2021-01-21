# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "RelWithDebInfo")
  file(REMOVE_RECURSE
  "CMakeFiles\\qtosg_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\qtosg_autogen.dir\\ParseCache.txt"
  "qtosg_autogen"
  )
endif()
