# Simple C Unit Test Framework (SCU)

SCU is a simple C unit testing framework that integrates with _STR_, the simple test runner.

## Design

SCU consists of two parts: a test runner and tests. Individual test cases are arranged into test
modules. The runner and the test modules communicate the test results via a JSON-based protocol.

One of the primary concern of SCU is to make sure that accidental buffer overflow errors does
not break operation of the test runner. It achieves this by trying to isolate the memory as much as
possible from the tests themselves, and to try to detect when corruption has occurred.

The isolation part is solved by avoiding memory allocations in the protocol code. This means that
even if memory has been corrupted, the protocol itself will most likely remain intact. Hence, the
test runner will still be able to parse the corrupt test data.

## Writing tests

TODO: document how to create unit test modules

## Compiling tests

TODO: document how to compile tests with SCU

## Executing tests

TODO: document how to run the tests with STR
