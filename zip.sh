#!/bin/bash
cp -p lecturer_tomtom Lecturer/
rm -f lecturer.zip
7z a -tzip '-xr!.svn' lecturer.zip SDKRegistry Lecturer
