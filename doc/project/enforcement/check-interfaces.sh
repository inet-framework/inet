#!/usr/bin/env bash
#
# The interface gate for INET -- a T3 fitness function (see AR-QUAL-ENFORCED).
# A C++ class named I<Stem> is an interface and holds no implementation:
#
#   NR-CPP-TYPE              -- the I prefix is a promise; a class with a body must not carry it
#   AR-ORG-CONTRACT-PURITY   -- what the promise means: pure virtuals, a destructor, signal ids, types
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-interfaces.sh                  # all of src/inet
#   doc/project/enforcement/check-interfaces.sh src/inet/queueing
#   doc/project/enforcement/check-interfaces.sh --verbose     # show the offending line under each hit
#
# Exit status 0 = clean, 1 = findings.
set -uo pipefail
cd "$(dirname "$0")/../../.." || exit 2
exec python3 doc/project/enforcement/check_interfaces.py "$@"
