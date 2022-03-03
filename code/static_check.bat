@echo off

set searchFiles=*.h *.cpp

echo -----

echo Globals and Persists Found:
findstr -s -n -i -l "persist" %searchFiles%
findstr -s -n -i -l "global" %searchFiles%

echo -----