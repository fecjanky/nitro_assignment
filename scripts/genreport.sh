#!/bin/sh
cd $(dirname $0)/..
rm -rf report
mkdir report
python -m gcovr -r . --html --html-details -o report/coverage_report.html --filter="om-lib/*" --exclude="test/*" --html-title "coverage report" --exclude-unreachable-branches
