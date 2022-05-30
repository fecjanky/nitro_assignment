#!/bin/sh
cd $(dirname $0)/..
rm -rf report
mkdir report
python -m gcovr -r . --html --html-details -o report/coverage_report.html --filter="src/*" --exclude="test/*" --html-title "coverage report" --exclude-unreachable-branches
cd report
echo "Listening on http://$(hostname):8080/coverage_report.html"
python3 -m http.server  --directory . 8080