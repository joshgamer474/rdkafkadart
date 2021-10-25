import os
from conans import ConanFile, CMake, tools

class Rdkafka(ConanFile):
    name = "rdkafka-dart"
    version = "1.8.2"
    settings = "os", "compiler", "arch"
    options = {"shared": [True, False]}
    settings = {"os" : ["Windows", "Linux", "Android"], 
      "arch": ["x86", "x86_64", "armv7", "armv8"],
      "compiler": ["Visual Studio", "gcc", "clang"],
      "build_type": ["Debug", "Release"]}
    default_options = "shared=True"
    generators = "cmake"
    requires = (
      "librdkafka/1.8.2",
      "gtest/1.11.0"
    )
    exports_sources = "src/*"

    #def configure(self):
      #self.options["gtest"].shared = self.options.shared
      #self.options["librdkafka"].shared = self.options.shared
      #self.options["lz4"].shared = self.options.shared

    def imports(self):
        dest = os.getenv("CONAN_IMPORT_PATH", "bin")
        libDest = os.getenv("CONAN_IMPORT_PATH", "lib")
        libDest += os.sep + str(self.settings.arch)
        self.copy("*.dll", src="bin", dst=dest)
        self.copy("*.a", src="lib", dst=libDest)
        self.copy("*.so*", src="lib", dst=libDest)
        if (self.settings.os == "Android"):
            self.copy("*.h", src="include", dst="include")
        self.keep_imports = True

    def _configure_cmake(self):
      cmake = CMake(self)
      cmake.definitions["BUILD_UNIT_TEST"] = "true"
      if (self.settings.os == "Windows"):
        cmake.definitions["CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS"] = "true"
      cmake.configure()
      return cmake

    def build(self):
      with tools.run_environment(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
      with tools.run_environment(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
      self.cpp_info.libs = ["rdkafka-dart"]
