#!/bin/bash
cp -p lecturer_tomtom Lecturer/lecturer
arm-linux-strip Lecturer/lecturer
rm -f lecturer.zip
7z a -tzip '-xr!.svn' '-x!*/text/*.txt' '-x!*/conf' lecturer.zip SDKRegistry Lecturer
