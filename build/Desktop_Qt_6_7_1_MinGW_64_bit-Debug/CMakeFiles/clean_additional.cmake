# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "client\\CMakeFiles\\client_autogen.dir\\AutogenUsed.txt"
  "client\\CMakeFiles\\client_autogen.dir\\ParseCache.txt"
  "client\\client_autogen"
  "server\\CMakeFiles\\server_autogen.dir\\AutogenUsed.txt"
  "server\\CMakeFiles\\server_autogen.dir\\ParseCache.txt"
  "server\\server_autogen"
  "utils\\CMakeFiles\\utils_autogen.dir\\AutogenUsed.txt"
  "utils\\CMakeFiles\\utils_autogen.dir\\ParseCache.txt"
  "utils\\utils_autogen"
  )
endif()
