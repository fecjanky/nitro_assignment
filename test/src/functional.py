#!/usr/bin/env python

import argparse
import re
import subprocess
import difflib
import sys


def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        "-e", "--exec", help="path to executable binary", required=True)
    parser.add_argument("-b", "--baseline",
                        help="baseline expected output text", required=True)
    parser.add_argument("-t", "--timeout",
                        help="timeout value for runtime", type=int, default=60)
    parser.add_argument("input", help="Input json file")
    args = parser.parse_args()
    result = subprocess.check_output(
        [args.exec, args.input, str(args.timeout)]).decode()
    result = result.replace('\r\n', '\n')
    result = [l for l in result.splitlines() if l.strip()]

    with open(args.baseline) as fbaseline:
        baseline = [l.strip('\n') for l in fbaseline.readlines() if l.strip()]

    diff = difflib.unified_diff(
        result, baseline, fromfile=args.baseline, tofile=args.exec)
    if diff is not None:
        hasdiff = False
        for line in diff:
            hasdiff = True
            print(line)
        if hasdiff:
            return "output differs from baseline"


if __name__ == "__main__":
    sys.exit(main())
