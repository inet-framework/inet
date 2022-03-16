import logging

# TODO automate as much as possible from the following processes

# - check release
#   - check github
#     - check outstanding issues
#     - check outstanding pull requests
#   - check source code
#     - check for outstanding TODOs, FIXMEs, and KLUDGEs
#   - check source formatting
#     - check trailing whitespace
#     - check leading tab characters
#     - check C++ header guards
#   - check naming conventions
#   - check documentation
#     - check for invalid C++ and NED references
#   - run all tests
#     - run old opp tests
#       - run packet tests
#       - run queueing tests
#       - run protocol tests
#       - run module tests
#       - run unit tests
#     - run smoke tests
#     - run fingerprint tests
#     - run statistical tests
#     - run validation tests
#     - run leak tests
#     - run speed tests
#     - run feature tests

# - pre release tasks
#   - update version number
#   - format source code
#     - remove trailing whitespace
#     - remove leading tab characters
#     - fix C++ header guards
#     - clang tidy
#     - C++ reformat
#   - update documentation
#     - update ChangeLog files
#     - update WHATSNEW

# - post release tasks
#   - add ChangeLog tags
#   - create git tag
#   - upload artifacts

# - create new release
#   - compile C++ source
#     - compile debug version
#     - compile release version
#   - build documentation
#     - generate all charts
#       - create statistical results
#       - export all charts
#     - generate self documentation
#     - build User's Guide
#     - build Developer's Guide
#     - build NED documentation
#     - build doxygen documentation
#   - create archive
