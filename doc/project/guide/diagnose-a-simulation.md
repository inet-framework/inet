# Diagnose a simulation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [architecture.md](../rule/architecture.md), [testing.md](../rule/testing.md)

How to turn a failed run or an implausible result into a reproducible, evidence-backed explanation.
The objective is the first divergent transition, not the largest possible collection of diagnostics.

## 1. Fix the question and the reproduction

State the expected and observed behavior at one boundary: a state transition, message delivery,
packet exchange, recorded value, or failure. Start with one configuration and one run/seed, and
restrict the time, event, module, packet, or flow only when the restricted run still reproduces the
problem.

Expand seeds or parameters only after the narrow case is understood and only when the claim requires
stochastic robustness, a boundary sweep, or a regression/statistical campaign. Define the campaign's
conditions, repetitions, reductions and uncertainty under
[analyze-simulation-results.md](analyze-simulation-results.md) before running it.

Record the working directory, commit and binaries, build mode, launch command, INI configuration,
run and seed, effective parameter overrides, relevant instantiated module types, and the first known
wrong time or event. A similarly named example or an unevaluated wildcard is not the effective
model.

## 2. Classify every claim

Keep three kinds of claim distinct:

| Kind | Meaning | Example |
| --- | --- | --- |
| **Direct evidence** | One artifact establishes what happened at its own observation boundary. | A receiver capture contains a decoded frame; an event log records delivery; a debugger shows a state value. |
| **Correlated evidence** | Independent artifacts align by time and a stable identity. | A packet identity and timestamp connect a MAC log decision to frames at two capture points. |
| **Inference** | The conclusion explains direct or correlated evidence but is not itself recorded. | The first missing transition and source logic indicate why a retry was abandoned. |

Label inference and name plausible alternatives that the available evidence does not exclude.
Dissector heuristics and reconstructed protocol semantics are not direct simulator evidence.

## 3. Add the smallest decisive artifact

Begin with the ordinary Cmdenv failure or behavior and the effective configuration. Add targeted
module logs for decisions and state transitions, packet captures for protocol content at explicit
observation points, scalar or vector results for recorded quantities, and an event log only when
scheduling or message causality is missing. Use a source-level debugger after the evidence has
identified a suspicious event, object, state, or source path.

Enable temporary logging, captures, event logs, and result recording with command-line overrides.
Keep those overrides with the artifacts: diagnostics such as computed checksum/FCS processing or
extra recording can change execution cost or behavior and are part of the reproduction.

## 4. Respect observation boundaries

A sender capture proves observation at the sender, not reception or upper-layer delivery. Absence
from one receiver capture proves only that the packet was not observed at that selected point; it
does not by itself prove a collision, drop, or loss. When delivery, loss, forwarding, or
retransmission is disputed, compare both endpoints and add an intermediate boundary, targeted log,
counter, or event-log trace to locate the first missing transition.

Capture frame numbers are local to each capture. OMNeT++ event numbers belong to the simulator and
are not capture frame numbers. Correlate artifacts by simulation timestamp and the strongest
available stable identity: message or packet identity, addresses and protocol, sequence or fragment
number, ports, payload identity, and module path. State clock precision and ambiguity when several
events can match.

## 5. Build the shortest causal timeline

Trace backward from the first wrong or missing observation to the decision, send, schedule, or
state change that produced it, then forward to the resulting delivery, cancellation, timeout,
drop, or failure. Each row of the timeline names the simulation time, event when available, module,
object identity, action, artifact, and evidence classification.

Aggregates can confirm extent but rarely establish the transition that caused it. Use
[analyze-simulation-results.md](analyze-simulation-results.md) for comparisons and statistical
summaries, and return to time-aligned evidence for causal claims.

## 6. Preserve comparability

For a before/after or control/treatment comparison, hold the working directory, NED path, effective
configuration, traffic, binaries, build mode, run/seed, recording window, and diagnostic overrides
constant except for the named variable under test. Do not compare independent randomized
trajectories as if they were the same reproduction. When a diagnostic override cannot be held
constant, report it as a limitation and run an appropriate control.

Preserve original logs, captures, event logs, scalar/vector files, and debugger transcripts. Derived
or filtered artifacts name their source and transformation; they do not replace the original.

## 7. Report a reproducible conclusion

The report includes:

- the expected and observed behavior and the first divergent transition;
- working directory, commit/binaries, build mode, configuration, run/seed, exact command and exit
  status;
- effective overrides and relevant instantiated module types;
- artifact paths, observation points, filters, result names and time/event interval;
- a minimal timeline with direct, correlated, and inferred claims distinguished;
- comparison controls, decoding or recording limitations, and unresolved alternatives.

A conclusion is no stronger than its observation boundary. If the evidence identifies only where
the transition disappeared, report that boundary and the remaining uncertainty instead of naming an
unsupported root cause.
