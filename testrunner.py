from __future__ import print_function

import json
import os
import sys

from select import select
from subprocess import Popen, PIPE


class Colors:
    GRAY = "\x1b[90m"
    RED = "\x1b[91m"
    GREEN = "\x1b[92m"
    DEFAULT = "\x1b[39m"


class TestSuite:

    def __init__(self, test_path, idx):
        self.idx = idx
        self.finished = False
        self.failed = False
        self.events = []
        self.proc = Popen([os.path.abspath(test_path)], stdout=PIPE)

    def read_events(self):
        # Check the status of the process
        self.proc.poll()

        # Yield all pending events
        for line in self.proc.stdout:
            try:
                event = json.loads(line.decode())
            except ValueError:
                event = {
                    'event': 'testcase_error',
                    'message': "Failed to parse test case output",
                    'crash': False
                }
            yield event

        # Handle process completion
        if self.proc.returncode is not None:
            self.finished = True
            if self.proc.returncode != 0:
                self.failed = True

    def fileno(self):
        return self.proc.stdout.fileno()

    def read(self, size):
        return self.proc.stdout.read(size)


class Runner:

    def __init__(self, suite_names):
        self.suite_names = suite_names
        self.observers = []
        self.cur_idx = 0

    def register(self, observer):
        self.observers.append(observer)

    def run_suites(self):
        suites = [TestSuite(t, i) for i, t in enumerate(self.suite_names)]
        running_suites = suites[:]
        while running_suites:
            # Attempt to read from all running suites
            rs, _, _ = select(running_suites, [], [])
            for r in rs:
                # Handle all pending events
                for event in r.read_events():
                    self.handle_event(r, event)
                # Handle suite completion
                if r.finished:
                    running_suites.remove(r)
                    if r.failed:
                        self.handle_event(r, {
                            'event': 'testcase_error',
                            'message': "Test suite crashed",
                            'crash': True
                        })
                    # Handle all remaining completed suites
                    while suites[self.cur_idx].finished:
                        self.cur_idx += 1
                        if self.cur_idx == len(suites):
                            break
                        list(map(self.emit, suites[self.cur_idx].events))

    def handle_event(self, suite, event):
        if not event:
            return
        # Emit the event if it belongs to the current suite
        if suite.idx == self.cur_idx:
            self.emit(event)
        # Buffer the event for later if it belongs to any other suite
        else:
            suite.events.append(event)

    def emit(self, event):
        for observer in self.observers:
            observer.call(event)


class Observer:

    def call(self, event):
        handler = getattr(self, 'handle_' + event['event'], None)
        if handler:
            handler(event)


class TestEmitter(Observer):

    def __init__(self):
        self.start_event = None

    def handle_suite_start(self, event):
        print("  {[name]}".format(event))

    def handle_testcase_start(self, event):
        self.start_event = event

    def handle_testcase_end(self, event):
        self.print_testcase(event)

    def handle_testcase_error(self, event):
        self.print_testcase(event)

    def print_testcase(self, event):
        event_type = event['event']
        success = event['success'] if event_type == 'testcase_end' else False
        result = (Colors.GREEN + "PASS") if success else (Colors.RED + "FAIL")
        print("    [ {result}{colors.DEFAULT} ] {colors.GRAY}{event[description]}{colors.DEFAULT}".format(event=self.start_event, result=result, colors=Colors), end='')
        if event_type == 'testcase_end':
            print(" ({event[duration]:.3f} ms)".format(event=event))
            if not success:
                for failure in event['failures']:
                    print("           * " + failure['message'])
                    print("             @ {file}:{line}".format(**failure))
                with open(self.start_event['output']) as f:
                    for line in f:
                        print("           > " + line, end='')
        elif event_type == 'testcase_error':
            print('')
            print("           ! " + event['message'])


class SummaryEmitter(Observer):

    def __init__(self):
        self.suite_counter = 0
        self.suite_fail_counter = 0
        self.test_counter = 0
        self.test_fail_counter = 0
        self.assert_counter = 0
        self.assert_fail_counter = 0
        self.duration_total = 0
        self.cpu_time_total = 0

    def handle_suite_start(self, event):
        self.has_reported_failing_test = False

    def handle_testcase_end(self, event):
        self.assert_counter += event['asserts']
        self.assert_fail_counter += len(event['failures'])
        self.test_counter += 1
        self.duration_total += event['duration']
        self.cpu_time_total += event['cpu_time']
        if event['failures']:
            self.test_fail_counter += 1
            self.has_reported_failing_test = True

    def handle_testcase_error(self, event):
        self.test_counter += 1
        self.test_fail_counter += 1
        if event['crash']:
            self.suite_counter += 1
            self.suite_fail_counter += 1

    def handle_suite_end(self, event):
        self.suite_counter += 1
        if self.has_reported_failing_test:
            self.suite_fail_counter += 1

    def print_summary(self):
        print("\n  Run summary:\n")
        print((
            "  +----------+------+------+----------+\n"
            "  |          | Pass | Fail |    Total |\n"
            "  +----------+------+------+----------+\n"
            "  | Suites   | {:4} | {:4} |    {:5} |\n"
            "  | Tests    | {:4} | {:4} |    {:5} |\n"
            "  | Asserts  | {:4} | {:4} |    {:5} |\n"
            "  | Elapsed  |      |      | {:7.3f}s |\n"
            "  | CPU time |      |      | {:7.3f}s |\n"
            "  +---------+------+------+-----------+\n"
        ).format(
            self.suite_counter - self.suite_fail_counter,
            self.suite_fail_counter,
            self.suite_counter,
            self.test_counter - self.test_fail_counter,
            self.test_fail_counter,
            self.test_counter,
            self.assert_counter - self.assert_fail_counter,
            self.assert_fail_counter,
            self.assert_counter,
            self.duration_total,
            self.cpu_time_total
        ))


class TestCleaner(Observer):

    def __init__(self):
        self.start_event = None

    def handle_testcase_start(self, event):
        self.start_event = event

    def handle_testcase_end(self, event):
        self.clean_testcase()

    def handle_testcase_error(self, event):
        self.clean_testcase()

    def clean_testcase(self):
        os.unlink(self.start_event['output'])


if __name__ == '__main__':
    runner = Runner(sys.argv[1:])
    emitter = SummaryEmitter()
    runner.register(TestEmitter())
    runner.register(emitter)
    runner.register(TestCleaner())
    runner.run_suites()
    emitter.print_summary()
