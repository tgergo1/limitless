# SPDX-License-Identifier: GPL-3.0-only
from conan import ConanFile
from conan.tools.files import copy, get
from conan.tools.layout import basic_layout
import os

required_conan_version = ">=1.53.0"


class LimitlessConan(ConanFile):
    name = "limitless"
    description = "Single-header exact arbitrary-precision integer/rational library for C and C++."
    license = "GPL-3.0-only"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/tgergo1/limitless"
    topics = ("arbitrary-precision", "bigint", "rational", "header-only", "c", "c++")
    package_type = "header-library"
    settings = "os", "arch", "compiler", "build_type"
    no_copy_source = True

    def layout(self):
        basic_layout(self, src_folder="src")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def package(self):
        copy(self, pattern="LICENSE", dst=os.path.join(self.package_folder, "licenses"), src=self.source_folder)
        copy(self, pattern="limitless.h", dst=os.path.join(self.package_folder, "include"), src=self.source_folder)
        copy(self, pattern="limitless.hpp", dst=os.path.join(self.package_folder, "include"), src=self.source_folder)

    def package_id(self):
        self.info.clear()

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_file_name", "Limitless")
        self.cpp_info.set_property("cmake_target_name", "limitless::limitless")
        self.cpp_info.set_property("pkg_config_name", "limitless")
