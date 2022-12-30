import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:path/path.dart' as path;

/// Returns rdkafka library path
String getLibraryPath({String? libpath}) {
  var libraryPath = path.join(Directory.current.parent.path, 'build_release',
      'lib', 'libRdkafkaDart.so');
  if (Platform.isMacOS) {
    libraryPath = path.join(
        Directory.current.parent.path, 'build_release', 'bin', 'Rdkafka.dylib');
  } else if (Platform.isWindows) {
    libraryPath = path.join(Directory.current.parent.path, 'build_release',
        'bin', 'RdkafkaDart.dll');
  } else if (Platform.isAndroid) {
    libraryPath = path.join('libRdkafkaDart.so');
  } else if (Platform.isIOS) {
    //libraryPath = path.join('libRdkafkaDart.dylib');
    libraryPath = path.join('RdkafkaDart.framework/RdkafkaDart');
  }
  //print("libraryPath: $libraryPath");
  return libraryPath;
}

/// Load library and return
ffi.DynamicLibrary loadLibrary({String? libpath}) {
  return ffi.DynamicLibrary.open(getLibraryPath(libpath: libpath));
}
