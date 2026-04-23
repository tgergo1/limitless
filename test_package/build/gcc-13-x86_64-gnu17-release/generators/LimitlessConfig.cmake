########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(Limitless_FIND_QUIETLY)
    set(Limitless_MESSAGE_MODE VERBOSE)
else()
    set(Limitless_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/LimitlessTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${limitless_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(Limitless_VERSION_STRING "0.2.0")
set(Limitless_INCLUDE_DIRS ${limitless_INCLUDE_DIRS_RELEASE} )
set(Limitless_INCLUDE_DIR ${limitless_INCLUDE_DIRS_RELEASE} )
set(Limitless_LIBRARIES ${limitless_LIBRARIES_RELEASE} )
set(Limitless_DEFINITIONS ${limitless_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${limitless_BUILD_MODULES_PATHS_RELEASE} )
    message(${Limitless_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


