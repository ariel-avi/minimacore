./build/coverage/tests/minimacore_tests
gcovr --filter='src/' --xml-pretty > cobertura-coverage.xml
return 0
