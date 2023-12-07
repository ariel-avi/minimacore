cmake -S . -B build/coverage -DCMAKE_BUILD_TYPE="Coverage"
cmake --build build/coverage --config Debug --target minimacore_tests
lcov --capture --no-external --initial --directory . --output-file coverage_baseline.info --exclude '/usr/*' --exclude '*/tests/src/*'
./build/coverage/tests/minimacore_tests
lcov --capture --directory . --output-file coverage_total.info --exclude '/usr/*' --exclude '*/tests/src/*'
lcov -a coverage_baseline.info -a coverage_total.info -o coverage.info
