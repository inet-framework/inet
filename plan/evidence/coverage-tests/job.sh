#!/bin/bash
# Usage: job.sh build | job.sh run <command...>
# Runs the steps of the planned GitHub coverage job in the container, in coverage mode. Without
# inet-gpl: its packetdrill runner is built in debug mode only, so the packetdrill wrappers skip.
export GITHUB_WORKSPACE=/home/runner/work/inet/inet
export MODE=coverage
export HOME=/tmp/home
mkdir -p $HOME /home/runner/work/ccache
cmd=$1; shift
if [ "$cmd" = build ]; then
  clang --version | head -1
  source $GITHUB_WORKSPACE/inet/_scripts/github/build-omnetpp.sh
  [ -x $GITHUB_WORKSPACE/omnetpp/bin/opp_run_coverage ] || { echo "ERROR: omnetpp build failed"; exit 1; }
  source $GITHUB_WORKSPACE/inet/_scripts/github/build-inet.sh
  ls -la $GITHUB_WORKSPACE/inet/src/libINET_coverage.so || { echo "ERROR: inet build failed"; exit 1; }
  echo BUILD-OK
else
  # the commands that the opp_repl worktree adds; the image has the opp_repl of github_ci
  mkdir -p $HOME/bin
  for command in export_coverage:opp_export_coverage coverage_report:opp_coverage_report; do
    printf '#!/bin/sh\nexec python3 -c "from opp_repl.main import %s_main; %s_main()" "$@"\n' ${command%%:*} ${command%%:*} > $HOME/bin/${command#*:}
    chmod +x $HOME/bin/${command#*:}
  done
  export PATH=$HOME/bin:$PATH
  cd $GITHUB_WORKSPACE/omnetpp && . setenv -f >/dev/null
  cd $GITHUB_WORKSPACE/inet && . setenv -f >/dev/null
  "$@"
fi
