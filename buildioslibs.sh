#!/bin/bash
# x86 and x86_64 is only for simulator/emulator
conan install . -if=iosarmv8build --profile=profiles/iosarmv8 --build=outdated

conan build . -bf=iosarmv8build

conan package . -bf=iosarmv8build -pf=apkg

cp -r apkg/lib/ ../pokestonks_mobile/ios/Runner/Frameworks
