./build/tests/minimacore_tests
gcovr --exclude='tests/src/*' --exclude='build/*' --xml > cobertura-coverage.xml
return 0
