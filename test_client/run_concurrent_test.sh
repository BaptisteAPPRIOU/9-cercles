#!/bin/bash
# Concurrent test runner - launches 4 test clients simultaneously

echo "Starting Concurrent Test with 4 Clients"
echo "========================================"
echo

# Check if test_client exists
if [ ! -f "../build/test_client/test_client" ]; then
    echo "Error: test_client not found. Please build the project first."
    echo "Expected location: ../build/test_client/test_client"
    exit 1
fi

# Create a temporary directory for test results
mkdir -p test_results

echo "Starting 4 concurrent clients..."
echo


# Start 4 clients in parallel, each with a unique test script
../build/test_client/test_client --script client1_test.txt --verbose > test_results/client1_output.log 2>&1 && echo "Client 1 COMPLETED" > test_results/client1_done.txt &
CLIENT1_PID=$!

../build/test_client/test_client --script client2_test.txt --verbose > test_results/client2_output.log 2>&1 && echo "Client 2 COMPLETED" > test_results/client2_done.txt &
CLIENT2_PID=$!

../build/test_client/test_client --script client3_test.txt --verbose > test_results/client3_output.log 2>&1 && echo "Client 3 COMPLETED" > test_results/client3_done.txt &
CLIENT3_PID=$!

../build/test_client/test_client --script client4_test.txt --verbose > test_results/client4_output.log 2>&1 && echo "Client 4 COMPLETED" > test_results/client4_done.txt &
CLIENT4_PID=$!

echo "All clients started with PIDs: $CLIENT1_PID, $CLIENT2_PID, $CLIENT3_PID, $CLIENT4_PID"
echo "Waiting for completion..."
echo

# Wait for all background processes to complete
wait $CLIENT1_PID
CLIENT1_EXIT=$?
wait $CLIENT2_PID
CLIENT2_EXIT=$?
wait $CLIENT3_PID
CLIENT3_EXIT=$?
wait $CLIENT4_PID
CLIENT4_EXIT=$?

echo
echo "========================================"
echo "All clients completed! Showing results:"
echo "========================================"
echo

# Display results from each client
for i in 1 2 3 4; do
    echo "--- Client $i Results (Exit Code: $(eval echo \$CLIENT${i}_EXIT)) ---"
    if [ -f "test_results/client${i}_output.log" ]; then
        cat "test_results/client${i}_output.log"
    else
        echo "No output file found for client $i"
    fi
    echo
done

echo "========================================"
echo "Concurrent test completed!"
echo "Check test_results folder for detailed logs."
echo "========================================"

# Calculate overall result
TOTAL_FAILURES=$((CLIENT1_EXIT + CLIENT2_EXIT + CLIENT3_EXIT + CLIENT4_EXIT))
if [ $TOTAL_FAILURES -eq 0 ]; then
    echo "All clients passed their tests!"
    exit 0
else
    echo "Some clients failed their tests (total failures: $TOTAL_FAILURES)"
    exit 1
fi
