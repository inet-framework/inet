#!/bin/bash
# Phase 2 Pass A: in-place splits & rewords (no reordering).
# Runs the interactive rebase non-interactively; dispatches on HEAD subject at each edit stop.
set -e
cd /home/levy/workspace/inet-infrastructure
D=plan/artifacts/phase2

STOPS='4c5522a4b0|85415fbf50|7f2fccbdff|f2aecee10e|ad606a05bb|d587f79440|62d30b2a6f|f57702acd4|8dee6f83f6|8360ab4d9a'
GIT_SEQUENCE_EDITOR="sed -i -E 's/^pick ($STOPS)/edit \\1/'" git -c core.abbrev=40 rebase -i 47adf296fb || true

while [ -d "$(git rev-parse --git-path rebase-merge)" ]; do
    subj=$(git log -1 --format=%s HEAD)
    echo "=== STOP at: $subj"
    case "$subj" in
        "all: Added missing Enter_Method() and take() calls into pushPacket implementations.")
            git commit --amend -F $D/msg-4c5522a4b0.txt ;;
        "todo1")
            python3 $D/split_commit.py $D/spec-todo1.json ;;
        "Removed obsolete IProtocolRegistrationListener includes and registerProtocol/registerService calls.")
            git commit --amend -m "all: Removed obsolete IProtocolRegistrationListener includes and registerProtocol/registerService calls." ;;
        "Fixed missing initialize() declaration in EchoProtocol.h.")
            git commit --amend -m "EchoProtocol: Fixed missing initialize() declaration." ;;
        "Removed obsolete streamThroughputVectors code replaced by signal-based recording.")
            git commit --amend -m "Sctp: Removed obsolete streamThroughputVectors code replaced by signal-based recording." ;;
        "todo2")
            git commit --amend -F $D/msg-todo2.txt ;;
        "Fixed 802.11 module interface lookup.")
            python3 $D/split_commit.py $D/spec-62d.json ;;
        "todo3")
            python3 $D/split_commit.py $D/spec-todo3.json ;;
        "todo8")
            python3 $D/split_commit.py $D/spec-todo8.json ;;
        "Ieee8022Llc: Fixed lower layer sink lookup to work over non-Ethernet MACs.")
            python3 $D/split_commit.py $D/spec-8360.json ;;
        *)
            echo "UNEXPECTED STOP: $subj" >&2; exit 1 ;;
    esac
    git rebase --continue || { echo "REBASE CONTINUE FAILED" >&2; exit 1; }
done

echo "=== Pass A rebase finished. Verifying tree identity (excluding plan/)..."
if git diff --quiet topic/infrastructure-backup-20260714 HEAD -- ':(exclude)plan'; then
    echo "OK: tree identical to backup outside plan/"
else
    echo "TREE MISMATCH vs backup:" >&2
    git diff --stat topic/infrastructure-backup-20260714 HEAD -- ':(exclude)plan' >&2
    exit 1
fi
git rev-list --count 47adf296fb..HEAD
