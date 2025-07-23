#!/bin/bash
# Shell script to run tests

echo "Starting Test Client for First Client-Server"
echo

if [ $# -eq 0 ]; then
    echo "Usage: ./run_tests.sh [script_name] [options]"
    echo
    echo "Examples:"
    echo "  ./run_tests.sh basic_test.txt"
    echo "  ./run_tests.sh advanced_test.txt --verbose"
    echo "  ./run_tests.sh stress_test.txt"
    echo
    echo "Available test scripts:"
    ls -1 *.txt 2>/dev/null || echo "No test scripts found"
    exit 1
fi

SCRIPT_FILE="$1"
shift
OPTIONS="$@"

if [ ! -f "$SCRIPT_FILE" ]; then
    echo "Error: Script file '$SCRIPT_FILE' not found."
    exit 1
fi

echo "Running test script: $SCRIPT_FILE"
echo "Options: $OPTIONS"
echo

# Navigate to build directory and run the test client
cd ../build/test_client
if [ $? -ne 0 ]; then
    echo "Error: Could not find build directory. Make sure the project is built."
    exit 1
fi

./test_client --script "../../test_client/$SCRIPT_FILE" $OPTIONS
EXIT_CODE=$?

# Return to original directory
cd ../../test_client

echo
echo "Test execution completed with exit code: $EXIT_CODE"
exit $EXIT_CODE
