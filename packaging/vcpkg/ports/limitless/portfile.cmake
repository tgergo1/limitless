# SPDX-License-Identifier: GPL-3.0-only
set(_limitless_local_source "${CURRENT_PORT_DIR}/../../../..")

if(EXISTS "${_limitless_local_source}/CMakeLists.txt")
    set(SOURCE_PATH "${_limitless_local_source}")
else()
    set(_limitless_source_sha512
        "0c48f94423a30ec078faa82ff8afd06b010d1653d6496a46a46dd80815f1d69f5dad3c3fa82d91e9933917d4e38006adae69e6ef861577537110706e21343cd7"
    )

    vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO tgergo1/limitless
        REF "v${VERSION}"
        SHA512 "${_limitless_source_sha512}"
        HEAD_REF main
    )
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME Limitless CONFIG_PATH lib/cmake/Limitless)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(
    INSTALL "${SOURCE_PATH}/LICENSE"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright
)
