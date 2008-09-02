#!/bin/bash
rev=r`svn info|grep ^Revision|sed "s,^Revision: ,,"`
cp -p lecturer_tomtom Lecturer/lecturer
arm-linux-strip Lecturer/lecturer
rm -f lecturer-$rev.zip
7z a -tzip '-xr!.svn' '-x!*/text/*' '-x!*/conf' lecturer-$rev.zip SDKRegistry Lecturer
