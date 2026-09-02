# Working in this repository

INET's project requirements, design, rules, audits, enforcement and contributor procedures are
canonical under [doc/project/](doc/project/README.md). This instruction file is only a routing page;
it contains no second copy of project policy
([DR-NO-SECOND-COPY](doc/project/rule/documentation.md#dr-no-second-copy)).

Start every change with:

- the [project map](doc/project/README.md);
- the [contribution workflow](doc/project/guide/contribute-a-change.md); and
- the [seal registry](doc/project/audit/seal-list.md) and
  [sealing rules](doc/project/rule/sealing.md) for source-path status.

Then use the entry point for the task:

- architecture and implementation constraints:
  [architecture.md](doc/project/rule/architecture.md), the applicable
  [domain rules](doc/project/domain/README.md), and the canonical exception ledgers under
  [audit/](doc/project/audit/README.md);
- testing and evidence: [testing.md](doc/project/rule/testing.md),
  [test-anatomy.md](doc/project/design/test-anatomy.md), and
  [run-the-gates.md](doc/project/guide/run-the-gates.md);
- recorded expectations: [change-a-baseline.md](doc/project/guide/change-a-baseline.md);
- code review: [review-a-code-change.md](doc/project/guide/review-a-code-change.md), additionally
  [review-a-pull-request.md](doc/project/guide/review-a-pull-request.md) for a pull request or branch
  series, and the [general](doc/project/enforcement/checklist/general.md) and
  [IEEE 802.11](doc/project/enforcement/checklist/ieee80211.md) checklists for rule compliance; and
- repository layout: [repository-layout.md](doc/project/design/repository-layout.md).
