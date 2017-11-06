# TODO

 - Summary colors. Use yellow color when not all tests pass and mark failures with red; green if all is okay.
 - Make the buffering of stdout and stderr configurable per test case
 - Break the runner and the framework up into separate parts (name them STRUT and STRUT-C perhaps?)
 - Support more JSON encodings (currently only ASCII)
 - Restrict the number of running processes by a command line option and default to number of CPU cores
 - Format numbers in summary so they are easier to read (e.g. insert `,`)
 - Allow for longer periods in summary than seconds (display in minutes or hours if relevant)
 - Make test modules read from stdin and accept commands, default command should be to list test cases. Other commands can be to accept a list of which tests to run.
 - Make add option to filter test cases by tags
 - Mark if a test case exceeded the maximum limit on number of failures or length of a failure message
 - Collect failed test cases and prompt if the user wants to re-run them with break points set at the start of each test case
 - Print backtrace when test modules crash
 - Add option which defines how many times to run each test case
 - Implement "fatal" asserts which exits the module if the assertion does not succeed
