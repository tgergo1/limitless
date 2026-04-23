# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(limitless_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(limitless_FRAMEWORKS_FOUND_RELEASE "${limitless_FRAMEWORKS_RELEASE}" "${limitless_FRAMEWORK_DIRS_RELEASE}")

set(limitless_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET limitless_DEPS_TARGET)
    add_library(limitless_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET limitless_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${limitless_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${limitless_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### limitless_DEPS_TARGET to all of them
conan_package_library_targets("${limitless_LIBS_RELEASE}"    # libraries
                              "${limitless_LIB_DIRS_RELEASE}" # package_libdir
                              "${limitless_BIN_DIRS_RELEASE}" # package_bindir
                              "${limitless_LIBRARY_TYPE_RELEASE}"
                              "${limitless_IS_HOST_WINDOWS_RELEASE}"
                              limitless_DEPS_TARGET
                              limitless_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "limitless"    # package_name
                              "${limitless_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${limitless_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET limitless::limitless
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${limitless_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${limitless_LIBRARIES_TARGETS}>
                 )

    if("${limitless_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET limitless::limitless
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     limitless_DEPS_TARGET)
    endif()

    set_property(TARGET limitless::limitless
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${limitless_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET limitless::limitless
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${limitless_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET limitless::limitless
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${limitless_LIB_DIRS_RELEASE}>)
    set_property(TARGET limitless::limitless
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${limitless_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET limitless::limitless
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${limitless_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(limitless_LIBRARIES_RELEASE limitless::limitless)
