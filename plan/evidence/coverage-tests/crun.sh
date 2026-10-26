#!/bin/bash
# Runs "job.sh build" or "job.sh run <command...>" in the coverage container.
# CPUSET (default 0-31) gives the cores; nproc in the container reports their number, for
# example CPUSET=0-3 for a GitHub runner. POOL names the profile pool folder below profiles/.
W=/home/levy/workspace/ghci-coverage
exec taskset -c ${CPUSET:-0-31} podman run --rm --userns=keep-id \
  -v $W:/home/runner/work/inet/inet \
  -v $W/ccache:/home/runner/work/ccache \
  -v $W/tools:/tools:ro \
  -v /home/levy/workspace/opp_repl-coverage-tests:/opp_repl:ro \
  -e PYTHONPATH=/opp_repl:/tools/py \
  ${POOL:+-e LLVM_PROFILE_FILE=/home/runner/work/inet/inet/profiles/$POOL/%4m.profraw} \
  localhost/ghci-coverage:24.04-clang16 /tools/job.sh "$@"
