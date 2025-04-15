#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "tiny-ipc::tiny-ipc" for configuration "Debug"
set_property(TARGET tiny-ipc::tiny-ipc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(tiny-ipc::tiny-ipc PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libtiny-ipc.so"
  IMPORTED_SONAME_DEBUG "libtiny-ipc.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS tiny-ipc::tiny-ipc )
list(APPEND _IMPORT_CHECK_FILES_FOR_tiny-ipc::tiny-ipc "${_IMPORT_PREFIX}/lib/libtiny-ipc.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
