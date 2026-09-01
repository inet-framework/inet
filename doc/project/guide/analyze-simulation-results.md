# Analyze simulation results

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [testing.md](../rule/testing.md), [Collecting Results](../../src/users-guide/ch-collecting-results.rst)

How to turn OMNeT++ scalar, vector, statistic, and histogram results into a reproducible comparison
without turning dependent samples into false repetitions. The product-facing recording mechanisms
are described in [Collecting Results](../../src/users-guide/ch-collecting-results.rst); this guide
governs the contributor's analysis and report.

## 1. Define the estimand before selecting data

Name the question, metric, expected result type, module population, analysis window, unit, conditions,
independent repetition identifier, within-run reduction, module aggregation, and uncertainty method.
Decide whether a vector represents independent events, a state that remains in effect until its next
change, or a cumulative counter. These have different valid reductions.

Every varying experiment parameter belongs in the condition key unless it is explicitly the
repetition identifier. A label such as the configuration name is not a substitute for the actual
iteration variables and effective settings.

## 2. Select exact runs and inspect metadata

Choose `.sca` and `.vec` inputs by run metadata; do not assume that every file in a result directory
belongs to the requested experiment. Discover the actual module paths, result names, result types,
units, run attributes, iteration variables, warm-up period, recording settings, and seeds before
filtering.

Reject or report an empty or ambiguous selection, incompatible units, unexpected duplicate results,
missing conditions or repetitions, malformed vector timestamps/values, and files produced by
incomparable binaries or effective configurations. Keep the exact input paths and selection filter.

## 3. Reduce within each run first

The independent experimental runs are the repetitions. Packets, nodes, modules, and vector samples
inside one run normally share simulation state and do not increase the independent sample count.
Produce one justified estimate per run and condition before computing confidence intervals or other
across-run uncertainty.

If several modules contribute to one run, aggregate them with the operation defined by the metric:
for example, a total may be summed while a population average may require explicit weighting. Never
average module values merely because they have the same result name. State the operation and its
weights.

For piecewise-constant state vectors, compute a time-weighted value over the analysis window. The
vector must define the state at the window start; do not silently extrapolate backward. For event
samples, use an event-weighted reduction only when that is the estimand. Derive rates from counter
changes and elapsed time, and expose counter resets or nonpositive intervals rather than folding them
into the estimate.

## 4. Preserve units, windows, and exclusions

Use recorded metadata and model configuration for units and warm-up; do not invent either. Apply the
same analysis window to comparable runs, document whether it excludes the configured warm-up, and
state any unit conversion. Results without compatible or resolvable units are not directly
comparable.

Define missing-data handling and exclusions before inspecting the desired outcome. Report every
excluded run, module, interval, or sample with its criterion and count. An absent result is not zero
unless the model's result contract defines it that way.

## 5. Summarize independent repetitions

Compute uncertainty from the one-estimate-per-run table, grouped by the complete condition key.
Report the number of independent runs for every condition. A confidence interval from one run is
undefined, not zero-width. Do not pool packets or vector samples across runs and call the pooled
count the repetition count.

For a pooled distribution, state whether samples come from one run, are pooled across runs, or are
balanced per run. Naive pooling weights runs in proportion to their sample counts; use it only when
that weighting matches the question and disclose it.

## 6. Plot without changing the calculation

Match the display to the data: steps for piecewise-constant state, lines for continuous samples,
scatter for event observations, uncertainty summaries for parameter studies, and ECDFs or
histograms for distributions. Do not connect categorical or unrelated observations.

Downsampling is for display only. Compute reductions and uncertainty from the full selected data,
use a deterministic display reduction, state its method and extent, and preserve transitions in
step data. Prefer a narrower time window when downsampling would erase the event being explained.

## 7. Make the analysis reproducible

Keep extraction, transformation, and rendering separate in a saved non-interactive script or exact
command sequence. The report names:

- repository commit/binaries, working directory, input artifacts, configurations, runs and seeds;
- result type, module/result filter, run attributes, iteration variables and complete condition key;
- analysis window, warm-up treatment, units and conversions;
- per-run reduction, module aggregation and weighting;
- independent-run counts, uncertainty method and missing-data handling;
- exclusions, derived quantities and display-only downsampling;
- output tables/figures, commands and exit statuses.

Use [diagnose-a-simulation.md](diagnose-a-simulation.md) when an aggregate difference needs a causal
explanation. A scalar or summary can demonstrate that outcomes differ; it does not by itself locate
the state transition or packet decision that caused the difference.
