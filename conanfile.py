# SPDX-License-Identifier: GPL-3.0-only
from conan import ConanFile
from conan.tools.files import copy, load
import os


class LimitlessConan(ConanFile):
    name = "limitless"
    package_type = "header-library"
    license = "GPL-3.0-only"
    url = "https://github.com/tgergo1/limitless"
    homepage = "https://github.com/tgergo1/limitless"
    description = "Single-header exact arbitrary-precision integer/rational library for C and C++."
    topics = ("arbitrary-precision", "bigint", "rational", "header-only", "c", "c++")
    no_copy_source = True
    exports_sources = (
        "limitless.h",
        "limitless.hpp",
        "LICENSE",
        "README.md",
        "LIMITLESS_VERSION.txt",
        "VERSION",
        "CMakeLists.txt",
        "cmake/*",
        "packaging/pkgconfig/*",
    )

    def set_version(self):
        version_path = os.path.join(self.recipe_folder, "LIMITLESS_VERSION.txt")
        if not os.path.exists(version_path):
            version_path = os.path.join(self.recipe_folder, "VERSION")
        self.version = load(self, version_path).strip()

    def package(self):
        copy(self, "limitless.h", self.source_folder, os.path.join(self.package_folder, "include"))
        copy(self, "limitless.hpp", self.source_folder, os.path.join(self.package_folder, "include"))
        copy(self, "LICENSE", self.source_folder, os.path.join(self.package_folder, "licenses"))

    def package_id(self):
        self.info.clear()

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_file_name", "Limitless")
        self.cpp_info.set_property("cmake_target_name", "limitless::limitless")
        self.cpp_info.set_property("pkg_config_name", "limitless")
