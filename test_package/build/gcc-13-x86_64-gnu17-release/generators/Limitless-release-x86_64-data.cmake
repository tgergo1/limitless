########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(limitless_COMPONENT_NAMES "")
if(DEFINED limitless_FIND_DEPENDENCY_NAMES)
  list(APPEND limitless_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES limitless_FIND_DEPENDENCY_NAMES)
else()
  set(limitless_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(limitless_PACKAGE_FOLDER_RELEASE "/home/runner/.conan2/p/b/limitf2c30e5de9837/p")
set(limitless_BUILD_MODULES_PATHS_RELEASE )


set(limitless_INCLUDE_DIRS_RELEASE "${limitless_PACKAGE_FOLDER_RELEASE}/include")
set(limitless_RES_DIRS_RELEASE )
set(limitless_DEFINITIONS_RELEASE )
set(limitless_SHARED_LINK_FLAGS_RELEASE )
set(limitless_EXE_LINK_FLAGS_RELEASE )
set(limitless_OBJECTS_RELEASE )
set(limitless_COMPILE_DEFINITIONS_RELEASE )
set(limitless_COMPILE_OPTIONS_C_RELEASE )
set(limitless_COMPILE_OPTIONS_CXX_RELEASE )
set(limitless_LIB_DIRS_RELEASE )
set(limitless_BIN_DIRS_RELEASE )
set(limitless_LIBRARY_TYPE_RELEASE UNKNOWN)
set(limitless_IS_HOST_WINDOWS_RELEASE 0)
set(limitless_LIBS_RELEASE )
set(limitless_SYSTEM_LIBS_RELEASE )
set(limitless_FRAMEWORK_DIRS_RELEASE )
set(limitless_FRAMEWORKS_RELEASE )
set(limitless_BUILD_DIRS_RELEASE )
set(limitless_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(limitless_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${limitless_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${limitless_COMPILE_OPTIONS_C_RELEASE}>")
set(limitless_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${limitless_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${limitless_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${limitless_EXE_LINK_FLAGS_RELEASE}>")


set(limitless_COMPONENTS_RELEASE )