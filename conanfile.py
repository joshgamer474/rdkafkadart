import os
from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration

required_conan_version = ">=1.33.0"

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
      #self.settings_target = getattr(self, 'settings_target', None)
      #self.options["gtest"].shared = self.options.shared
      #self.options["librdkafka"].shared = self.options.shared
      #self.options["lz4"].shared = self.options.shared

    def imports(self):
        dest = os.getenv("CONAN_IMPORT_PATH", "bin")
        libDest = os.getenv("CONAN_IMPORT_PATH", "lib")
        libDest += os.sep + str(self.settings.arch)
        self.copy("*.dll", src="bin", dst=dest)
        #self.copy("*.a", src="lib", dst=libDest)
        self.copy("*.so*", src="lib", dst=libDest)
        if self.settings.os == "Android":
            self.copy("*.h", src="include", dst="include")
            self.copy("libc++_shared.so", src="", dst=libDest, root_package="android-ndk")
        self.keep_imports = True

    def _configure_cmake(self):
      cmake = CMake(self)
      cmake.definitions["BUILD_UNIT_TEST"] = "true"
      if (self.settings.os == "Windows"):
        cmake.definitions["CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS"] = "true"
      if (self.settings.os == "Android"):
        cmake.definitions["BUILD_ANDROID"] = "true"
      cmake.configure()
      return cmake

    def build_requirements(self):
      if self.settings.os == "Android":
        self.build_requires("android-ndk/r23")

    def build(self):
      with tools.run_environment(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
      libDest = os.getenv("CONAN_IMPORT_PATH", "lib")
      if (self.settings.arch == "armv7"):
          libDest += os.sep + "armeabi-v7a"
      elif (self.settings.arch == "armv8"):
          libDest += os.sep + "arm64-v8a"
      else:
          libDest += os.sep + str(self.settings.arch)
      self.copy("Rdkafka*", src="bin", dst="bin", keep_path=False, excludes="RdkafkaTest*")
      self.copy("*.dll", src="bin", dst="bin", excludes="g*.dll")
      self.copy("*.h", src="src", dst="include")
      self.copy("*.h", src="include", dst="include")
      #self.copy("*.a", src="lib", dst=libDest, keep_path=False)
      self.copy("*.so", src="lib", dst=libDest, keep_path=False)

    def package_info(self):
      self.cpp_info.libs = ["rdkafka-dart"]
