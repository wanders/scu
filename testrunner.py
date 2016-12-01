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


class Testcase:

    def __init__(self, test_path, idx):
        self.idx = idx
        self.finished = False
        self.events = []
        self.proc = Popen([os.path.abspath(test_path)], stdout=PIPE)

    def read_event(self):
        line = self.proc.stdout.readline()
        if line == b'':
            self.finished = True
            return None
        return json.loads(line.decode())

    def fileno(self):
        return self.proc.stdout.fileno()


class Runner:

    def __init__(self, tests):
        self.observers = []
        self.tests = tests

    def register(self, observer):
        self.observers.append(observer)

    def run_tests(self):
        tests = [Testcase(t, i) for i, t in enumerate(self.tests)]
        running_tests = tests[:]
        cur_idx = 0
        while running_tests:
            rs, _, _ = select(running_tests, [], [])
            for r in rs:
                event = r.read_event()
                if event is None:
                    running_tests.remove(r)
                    while tests[cur_idx].finished:
                        cur_idx += 1
                        if cur_idx == len(tests):
                            break
                        list(map(self.handle_event, tests[cur_idx].events))
                else:
                    if r.idx == cur_idx:
                        self.handle_event(event)
                    else:
                        r.events.append(event)

    def handle_event(self, event):
        for observer in self.observers:
            observer.call(event)


class Observer:

    def call(self, event):
        handler = getattr(self, 'handle_' + event['event'], None)
        if handler:
            handler(event)


class TestEmitter(Observer):

    def handle_suite_start(self, event):
        print("  {[name]}".format(event))

    def handle_testcase(self, event):
        self.print_testcase(event)

    def print_testcase(self, event):
        result = (Colors.GREEN + "PASS") if event['success'] else (Colors.RED + "FAIL")
        print("    [ {result}{colors.DEFAULT} ] {colors.GRAY}{event[description]}{colors.DEFAULT} ({event[duration]:.3f} ms)".format(event=event, result=result, colors=Colors))
        if not event['success']:
            for failure in event['failures']:
                print("           * " + failure['message'])
                print("             @ {file}:{line}".format(**failure))
            with open(event['output']) as f:
                for line in f:
                    print("           > " + line, end='')
        # FIXME: This will remove the output file for all other observers; it should probably be somewhere else
        os.unlink(event['output'])


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

    def handle_testcase(self, event):
        self.assert_counter += event['asserts']
        self.assert_fail_counter += len(event['failures'])
        self.test_counter += 1
        self.duration_total += event['duration']
        self.cpu_time_total += event['cpu_time']
        if event['failures']:
            self.test_fail_counter += 1
            self.has_reported_failing_test = True

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


if __name__ == '__main__':
    runner = Runner(sys.argv[1:])
    emitter = SummaryEmitter()
    runner.register(TestEmitter())
    runner.register(emitter)
    runner.run_tests()
    emitter.print_summary()
