./build/tests/minimacore_tests
gcovr --exclude='tests/src/*' --sonarqube > sonar-coverage.xml
gcovr --exclude='tests/src/*' --xml > cobertura-coverage.xml
return 0
