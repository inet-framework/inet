#!/bin/bash
# Full run-0 parity campaign against a freshly regenerated baseline store.
# Regenerates tplx + ~tNl + ~tND baselines on the fp-master worktree, builds the
# combined baseline store, installs it into the branch worktree, runs the branch
# suite without and with the trajectory-verification mode, then restores the
# branch store.json. All artifacts go to plan/artifacts/phase4/ with the given
# tag suffix.
#
# Usage: rerun_parity_r0.sh <tag>   (e.g. "fixedfp")
set -e
TAG=$1
[ -n "$TAG" ] || { echo "usage: $0 <tag>" >&2; exit 1; }
A=/home/levy/workspace/inet-infrastructure/plan/artifacts
P=$A/phase4
M=/home/levy/workspace/inet-fp-master
B=/home/levy/workspace/inet-infrastructure

run_in() { # run_in <worktree> <command...>
    local W=$1; shift
    bash -c "cd /home/levy/workspace/omnetpp && source setenv -q && cd $W && source setenv -q && $*"
}

echo "== baseline update: tplx"
run_in $M "opp_update_fingerprint_test_results --load @opp -p inet --no-build -m release -r 0 --result-file $P/fp-update-master-r0-tplx-$TAG.json" > $P/fp-update-master-r0-tplx-$TAG.log 2>&1 || true
echo "== baseline update: ~tNl"
run_in $M "/home/levy/.venv/bin/python $A/fp_update_ingredients.py '~tNl' $P/fp-update-master-r0-tNl-$TAG.json" > $P/fp-update-master-r0-tNl-$TAG.log 2>&1 || true
echo "== baseline update: ~tND"
run_in $M "/home/levy/.venv/bin/python $A/fp_update_ingredients.py '~tND' $P/fp-update-master-r0-tND-$TAG.json" > $P/fp-update-master-r0-tND-$TAG.log 2>&1 || true

echo "== build combined baseline store"
/home/levy/.venv/bin/python - <<EOF
import json, re
def parse_params(s):
    m = re.match(r'^(\S+?)(?: -f (\S+))?(?: -c (\S+))?(?: -r (\d+))? for (.+)\$', s)
    assert m, s
    wd, ini, cfg, run, stl = m.groups()
    return (wd, ini or 'omnetpp.ini', cfg or 'General', int(run or 0), stl)
ok = {}
for ing, path in (('tplx', '$P/fp-update-master-r0-tplx-$TAG.json'),
                  ('~tNl', '$P/fp-update-master-r0-tNl-$TAG.json'),
                  ('~tND', '$P/fp-update-master-r0-tND-$TAG.json')):
    res = json.load(open(path))
    ok[ing] = {parse_params(r['parameters']) for r in res['results'] if r['result'] in ('KEEP', 'INSERT', 'UPDATE')}
    print(ing, 'ok tasks:', len(ok[ing]))
store = json.load(open('$M/tests/fingerprint/store.json'))
best = {}
for e in store:
    ing = e['ingredients']
    if ing not in ok:
        continue
    k = (e['working_directory'], e['ini_file'], e['config'], e['run_number'], e['sim_time_limit'])
    if k in ok[ing]:
        bk = k + (ing,)
        if bk not in best or e['timestamp'] > best[bk]['timestamp']:
            best[bk] = e
final = list(best.values())
print('combined baseline entries:', len(final))
json.dump(final, open('$P/store-baseline-512dd6f642-r0-$TAG.json', 'w'), indent=0)
json.dump(final, open('$B/tests/fingerprint/store.json', 'w'), indent=0)
EOF

echo "== branch suite: no mode"
run_in $B "opp_run_fingerprint_tests --load @opp -p inet --no-build -m release -r 0 --result-file $P/fp-branch-r0-$TAG.json" > $P/fp-branch-r0-$TAG.log 2>&1 || true
echo "== branch suite: trajectory-verification mode"
run_in $B "INET_TRAJECTORY_VERIFICATION=1 opp_run_fingerprint_tests --load @opp -p inet --no-build -m release -r 0 --result-file $P/fp-branch-r0-dp-$TAG.json" > $P/fp-branch-r0-dp-$TAG.log 2>&1 || true

echo "== restore branch store"
git -C $B checkout -- tests/fingerprint/store.json
echo "== done"
