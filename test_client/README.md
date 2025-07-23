# Test Client for First Client-Server

This is a scriptable test client that can be used to automate testing of your client-server application.

## Features

- **Scriptable**: Run tests from script files or command line
- **Automated**: Perfect for CI/CD pipelines
- **Flexible**: Support for various test scenarios
- **Detailed reporting**: Get comprehensive test results

## Building

The test client is built as part of the main project:

```bash
cd build
cmake ..
cmake --build .
```

The executable will be located at `build/test_client/test_client.exe` (Windows) or `build/test_client/test_client` (Linux/Mac).

## Usage

### Command Line Interface

#### Basic Usage
```bash
./test_client --help
```

#### Running a Script File
```bash
./test_client --script basic_test.txt
```

#### Direct Commands
```bash
./test_client --connect --send "hello" --expect "hello" --disconnect
```

#### Custom Server Configuration
```bash
./test_client --ip 192.168.1.100 --port 9090 --script advanced_test.txt
```

#### Verbose Mode
```bash
./test_client --verbose --script stress_test.txt
```

### Script File Format

Script files contain one command per line. Lines starting with `#` are comments.

#### Available Commands:
- `connect` - Connect to the server
- `send <message>` - Send a message to the server
- `expect <message>` - Expect a specific response from the server
- `wait <milliseconds>` - Wait for the specified time
- `disconnect` - Disconnect from the server

#### Example Script:
```
# Test basic echo functionality
connect
send hello world
expect hello world
wait 1000
send another message
expect another message
disconnect
```

## Example Test Scripts

### Basic Test (`basic_test.txt`)
Tests basic connectivity and single message exchange.

### Advanced Test (`advanced_test.txt`)
Tests multiple messages with delays between them.

### Stress Test (`stress_test.txt`)
Tests rapid message exchange to verify server stability.

### Concurrent Test (`concurrent_test.txt`)
Basic concurrent test script for multiple simultaneous clients.

### Unique Client Test (`unique_client_test.txt`)
Advanced concurrent test where each client sends unique messages using the `ID_PLACEHOLDER` substitution feature.

## Concurrent Testing

To test how your server handles multiple simultaneous connections, use the concurrent testing features:

### Simple Concurrent Test
```bash
./run_concurrent_test.bat    # Windows
./run_concurrent_test.sh     # Linux/Mac
```

This runs 4 identical clients simultaneously using `concurrent_test.txt`.

### Unique Client Concurrent Test
```bash
./run_unique_concurrent_test.bat    # Windows
```

This runs 4 clients with unique identifiers (ClientA, ClientB, ClientC, ClientD) using `unique_client_test.txt`. Each client sends messages with their unique ID, making it easier to track which messages came from which client.

### Manual Concurrent Testing
You can also manually start multiple clients:

```bash
# Terminal 1
./test_client --client-id Client1 --script unique_client_test.txt --verbose

# Terminal 2  
./test_client --client-id Client2 --script unique_client_test.txt --verbose

# Terminal 3
./test_client --client-id Client3 --script unique_client_test.txt --verbose

# Terminal 4
./test_client --client-id Client4 --script unique_client_test.txt --verbose
```

### Client ID Substitution

When using `--client-id <id>`, the test client will replace `ID_PLACEHOLDER` in script files with the specified ID. This allows you to create generic test scripts that can be customized per client instance.

Example script content:
```
send Hello from ID_PLACEHOLDER
expect Hello from ID_PLACEHOLDER
```

With `--client-id ClientA`, this becomes:
```
send Hello from ClientA
expect Hello from ClientA
```

## Configuration

The test client reads server configuration from:
1. Command line arguments (`--ip`, `--port`)
2. Environment file (`../.env`)
3. Defaults (localhost:8080)

## Exit Codes

- `0`: All tests passed
- `1`: One or more tests failed or error occurred

## Integration with CI/CD

You can use this test client in your CI/CD pipeline:

```bash
# Start your server first
./server &
SERVER_PID=$!

# Wait for server to start
sleep 2

# Run tests
./test_client --script basic_test.txt
TEST_RESULT=$?

# Clean up
kill $SERVER_PID

# Exit with test result
exit $TEST_RESULT
```

## Extending the Test Client

To add new commands:

1. Add the command to the `TestCommand` parsing in `TestScriptRunner.cpp`
2. Implement the command logic in `executeCommand()`
3. Update this documentation

## Troubleshooting

### Connection Issues
- Make sure your server is running before starting tests
- Check IP address and port configuration
- Verify firewall settings

### Script Issues
- Check script file format (one command per line)
- Ensure all expected messages match exactly what the server sends
- Use `--verbose` mode to see detailed execution logs

### Timeout Issues
- Increase timeout values for slow responses
- Check network latency
- Verify server processing time
