#!/usr/bin/env python3
"""Convert the text output of `make runtests` (AUnit test binaries run
through EpoxyDuino) into a JUnit XML report.

AUnit has no native JUnit output, so this script scrapes the same text that
a developer would read on a terminal:

    === run <dir>            (from tests/Makefile and tests/next.mk based
    ==== Running: <dir>        recipes in libraries/*/tests/Makefile)
    ...
    <ansi> <name><ansi> passed.
    <ansi> <name><ansi> failed.
     <name> skipped.
    TestRunner summary: X passed, Y failed, Z skipped, W timed out, out of N test(s).

Each "=== run <dir>" / "==== Running: <dir>" marker starts a new <testsuite>
named after the test directory. Any output printed between a marker (or the
previous test case) and the next test case's pass/fail/skip line is treated
as diagnostic output (e.g. AUnit's "Assertion failed: ..." message) and
attached as the <failure> text for that test case.

Usage:
    make -C tests runtests 2>&1 | tee test-output.log
    make -C libraries runtests 2>&1 | tee -a test-output.log
    python3 .github/scripts/aunit_to_junit.py test-output.log junit.xml
"""
import re
import sys
from xml.sax.saxutils import escape

ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
SUITE_MARKER_RE = re.compile(r"^={3,4}\s*(?:run|Running:)\s+(\S+)\s*$")
ANY_MARKER_RE = re.compile(r"^={3,4}\s")
CASE_RE = re.compile(r"^\s*(\S+)\s+(passed|failed|skipped)\.\s*$")
SUMMARY_RE = re.compile(
    r"^TestRunner summary: (\d+) passed, (\d+) failed, (\d+) skipped, "
    r"(\d+) timed out, out of (\d+) test\(s\)\.$"
)
NOISE_RE = re.compile(
    r"^(make(\[\d+\])?: (Entering|Leaving) directory |"
    r"TestRunner started on \d+ test\(s\)\.$|"
    r"-{5,}\[ running .* \]-{5,}$)"
)


class Suite:
    def __init__(self, name):
        self.name = name
        self.cases = []  # list of (name, status, detail)
        self.timed_out = 0


def parse(lines):
    suites = []
    suite = None
    pending = []  # lines seen since the last resolved test case

    for raw in lines:
        line = ANSI_RE.sub("", raw.rstrip("\r\n"))

        marker = SUITE_MARKER_RE.match(line)
        if marker:
            suite = Suite(marker.group(1))
            suites.append(suite)
            pending = []
            continue
        if ANY_MARKER_RE.match(line):
            # A build/clean marker (e.g. "=== all X", "==== Making: X") that
            # is not a run marker: it ends whatever suite was running.
            suite = None
            pending = []
            continue

        if suite is None:
            continue

        case = CASE_RE.match(line)
        if case:
            name, status = case.groups()
            detail = "\n".join(pending).strip()
            suite.cases.append((name, status, detail))
            pending = []
            continue

        summary = SUMMARY_RE.match(line)
        if summary:
            suite.timed_out = int(summary.group(4))
            pending = []
            continue

        if line.strip() and not NOISE_RE.match(line):
            pending.append(line)

    return suites


def to_junit_xml(suites):
    out = ['<?xml version="1.0" encoding="UTF-8"?>', "<testsuites>"]
    for s in suites:
        if not s.cases:
            continue
        failures = sum(1 for _, status, _ in s.cases if status in ("failed",))
        skipped = sum(1 for _, status, _ in s.cases if status == "skipped")
        out.append(
            '  <testsuite name="{}" tests="{}" failures="{}" skipped="{}">'.format(
                escape(s.name), len(s.cases), failures, skipped
            )
        )
        for name, status, detail in s.cases:
            out.append(
                '    <testcase classname="{}" name="{}">'.format(
                    escape(s.name), escape(name)
                )
            )
            if status == "failed":
                out.append(
                    '      <failure message="assertion failed">{}</failure>'.format(
                        escape(detail) if detail else "assertion failed"
                    )
                )
            elif status == "skipped":
                out.append("      <skipped/>")
            out.append("    </testcase>")
        out.append("  </testsuite>")
    out.append("</testsuites>")
    return "\n".join(out) + "\n"


def to_markdown_summary(suites):
    lines = ["| Suite | Passed | Failed | Skipped | Total |",
             "| --- | ---: | ---: | ---: | ---: |"]
    total_passed = total_failed = total_skipped = total = 0
    any_failed = False
    for s in suites:
        if not s.cases:
            continue
        passed = sum(1 for _, status, _ in s.cases if status == "passed")
        failed = sum(1 for _, status, _ in s.cases if status == "failed")
        skipped = sum(1 for _, status, _ in s.cases if status == "skipped")
        n = len(s.cases)
        total_passed += passed
        total_failed += failed
        total_skipped += skipped
        total += n
        if failed or s.timed_out:
            any_failed = True
        mark = "" if not failed else " :x:"
        lines.append(
            "| {}{} | {} | {} | {} | {} |".format(
                s.name, mark, passed, failed, skipped, n
            )
        )
    lines.append(
        "| **Total** | **{}** | **{}** | **{}** | **{}** |".format(
            total_passed, total_failed, total_skipped, total
        )
    )

    header = "### AUnit test results: " + (
        ":x: FAILED" if any_failed else ":white_check_mark: PASSED"
    )
    md = [header, ""] + lines

    failing = [
        (s.name, name, detail)
        for s in suites
        for name, status, detail in s.cases
        if status == "failed"
    ]
    if failing:
        md.append("")
        md.append("<details><summary>Failure details</summary>")
        md.append("")
        for suite_name, case_name, detail in failing:
            md.append("**{}: {}**".format(suite_name, case_name))
            md.append("")
            md.append("```")
            md.append(detail or "(no assertion detail captured)")
            md.append("```")
            md.append("")
        md.append("</details>")

    return "\n".join(md) + "\n"


def main():
    if len(sys.argv) < 3:
        print(
            "Usage: aunit_to_junit.py <input.log> <output-junit.xml> "
            "[output-summary.md]",
            file=sys.stderr,
        )
        return 2

    in_path, out_path = sys.argv[1], sys.argv[2]
    summary_path = sys.argv[3] if len(sys.argv) > 3 else None

    with open(in_path, "r", errors="replace") as f:
        suites = parse(f)

    with open(out_path, "w") as f:
        f.write(to_junit_xml(suites))

    summary = to_markdown_summary(suites)
    if summary_path:
        with open(summary_path, "w") as f:
            f.write(summary)
    else:
        print(summary)

    any_failed = any(
        status == "failed" for s in suites for _, status, _ in s.cases
    )
    return 1 if any_failed else 0


if __name__ == "__main__":
    sys.exit(main())
