#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint dart.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'RdkafkaDart'
  s.version          = '0.1.0'
  s.summary          = 'An librdkafka dart library.'
  s.description      = <<-DESC
  An librdkafka dart library.
                       DESC
  s.homepage         = 'http://example.com'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Your Company' => 'email@example.com' }
  s.source           = { :path => '.' }
  s.source_files = 'Classes/**/*'
  s.dependency 'Flutter'
  s.platform = :ios, '11.0'
  s.static_framework = false
  # Flutter.framework does not contain a i386 slice.
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES', 'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386' }
  s.swift_version = '5.0'
  s.vendored_frameworks = 'RdkafkaDart.framework'
end
