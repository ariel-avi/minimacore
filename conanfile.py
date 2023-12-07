import os
from conan import ConanFile
from conan.tools.microsoft import MSBuildToolchain


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "MSBuildToolchain", "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("eigen/3.4.0")
        self.requires("gtest/1.14.0")

    def layout(self):
        multi = True if self.settings.get_safe("compiler") == "msvc" else False

        if multi:
            self.folders.generators = os.path.join("build", "generators")
            self.folders.build = "build"
        else:
            self.folders.generators = os.path.join("build", str(self.settings.build_type), "generators")
            self.folders.build = os.path.join("build", str(self.settings.build_type))

