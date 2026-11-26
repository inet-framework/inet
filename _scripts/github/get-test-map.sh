#!/bin/env bash

# This script finds the test map of the change-based test selection for a pull request job of
# GitHub Actions Workflows (the test selection plan of opp_repl, plan/pending/test-selection.md,
# section 15.2). The selection is correct with any map whose commit is an ancestor of the base, but
# a newer map selects fewer tests. The script takes the map of the newest finished push run of the
# workflow on master whose commit is an ancestor of the base. If no such run has the map, it takes
# the newest map of the history of the map repository (the weekly map). It needs no opp_repl,
# because the header of the map names the opp_repl commit that the job installs.
#
# Usage: get-test-map.sh WORKFLOW KIND BASE FOLDER
#    WORKFLOW               - the file name of the workflow of the push runs, for example
#                             fingerprint-tests.yml
#    KIND                   - the kind of the tests; the artifact of the map is test-map-KIND
#    BASE                   - the base commit of the change, for a pull request the first parent
#                             of its merge commit
#    FOLDER                 - the folder for the map; the script removes it first
#
# Run it in the INET checkout, with the history back to the map commit (fetch-depth: 0).
#
# The following environment variables must be set when invoked:
#    GH_TOKEN               - a token that can read the artifacts of the repository
#    GITHUB_REPOSITORY      - this is provided by GitHub
#
# Optional:
#    MAP_REPOSITORY         - the URL of the map repository, default
#                             https://github.com/inet-framework/coverage
#    GITHUB_OUTPUT          - this is provided by GitHub; the script writes these outputs:
#                             found (true or false), folder, commit (the map commit), distance
#                             (the commits from the map commit to the base), omnetpp_commit,
#                             opp_repl_commit and source

set -e

if [ $# -ne 4 ]; then
  echo "Usage: $0 WORKFLOW KIND BASE FOLDER" >&2
  exit 2
fi
workflow=$1
kind=$2
base=$(git rev-parse --verify "$3^{commit}")
folder=$4
map_repository=${MAP_REPOSITORY:-https://github.com/inet-framework/coverage}
output=${GITHUB_OUTPUT:-/dev/null}

rm -rf "$folder"

# Returns true if the commit is in the checkout and is an ancestor of the base.
is_ancestor() {
  [ -n "$1" ] && git cat-file -e "$1^{commit}" 2>/dev/null && git merge-base --is-ancestor "$1" "$base"
}

# Uses the map in $folder: writes the outputs and stops the script. Removes the map and returns
# false if the job cannot use it.
use_map() {
  local source=$1
  local commit omnetpp_commit opp_repl_commit distance
  commit=$(jq -r '.commit // empty' "$folder/header.json")
  omnetpp_commit=$(jq -r '.omnetpp_commit // empty' "$folder/header.json")
  opp_repl_commit=$(jq -r '.opp_repl_commit // empty' "$folder/header.json")
  if ! is_ancestor "$commit"; then
    echo "The map of $source is for $commit, which is not an ancestor of the base."
  elif [ "$(jq -r '.changed_files' "$folder/header.json")" = true ]; then
    # such a map selects all tests (section 4.3 of the plan)
    echo "The map of $source comes from a tree with changes."
  elif [ -z "$omnetpp_commit" ] || [ -z "$opp_repl_commit" ]; then
    # without them, the job cannot use the environment of the map, and the map selects all tests
    echo "The map of $source names no omnetpp or opp_repl commit."
  else
    distance=$(git rev-list --count "$commit..$base")
    echo "The map of $source is for $commit, $distance commits older than the base $base."
    {
      echo "found=true"
      echo "folder=$folder"
      echo "commit=$commit"
      echo "distance=$distance"
      echo "omnetpp_commit=$omnetpp_commit"
      echo "opp_repl_commit=$opp_repl_commit"
      echo "source=$source"
    } >> "$output"
    exit 0
  fi
  rm -rf "$folder"
  return 1
}

# 1. The map of the newest finished push run whose commit is an ancestor of the base. A run that
# failed can have the map too: the map job writes the map also if a test fails.
runs=$(gh api "repos/$GITHUB_REPOSITORY/actions/workflows/$workflow/runs?branch=master&event=push&status=completed&per_page=50" \
  --jq '.workflow_runs[] | "\(.id) \(.head_sha)"' || true)
while read -r run_id head_sha; do
  [ -n "$run_id" ] || continue
  is_ancestor "$head_sha" || continue
  artifact=$(gh api "repos/$GITHUB_REPOSITORY/actions/runs/$run_id/artifacts?name=test-map-$kind" \
    --jq '[.artifacts[] | select(.expired == false)] | length' || echo 0)
  [ "$artifact" != 0 ] || continue
  if gh run download "$run_id" --repo "$GITHUB_REPOSITORY" --name "test-map-$kind" --dir "$folder" && [ -f "$folder/header.json" ]; then
    use_map "the push run ${GITHUB_SERVER_URL:-https://github.com}/$GITHUB_REPOSITORY/actions/runs/$run_id" || true
  fi
  rm -rf "$folder"
done <<< "$runs"

# 2. The newest version of the weekly map in the history of the map repository.
repository=$(mktemp -d)
if git clone -q --filter=blob:none --no-checkout "$map_repository" "$repository" 2>/dev/null; then
  for commit in $(git -C "$repository" log --format=%H -200 HEAD -- map/header.json 2>/dev/null); do
    map_commit=$(git -C "$repository" show "$commit:map/header.json" | jq -r '.commit // empty' 2>/dev/null || true)
    is_ancestor "$map_commit" || continue
    mkdir -p "$folder"
    git -C "$repository" archive "$commit" map | tar -x -C "$folder" --strip-components=1
    use_map "the map repository $map_repository at $commit" || true
  done
fi

echo "No map has a commit that is an ancestor of the base $base; the job runs all tests."
echo "found=false" >> "$output"
