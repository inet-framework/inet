:orphan:

Tracking Down Fingerprint Differences
=====================================

workflow in detail!

enable eventlog-recording in both baseline and updated version
run both simulations
run difffingerpints on the resulting eventlog files
identify first events in both where the fingerprints differ
check sequence chart around the first differing events in both
run until and debug first events (maybe both) where the fingerprints differ in the eventlog files
the difference in the simulation execution may be way before these events, potentially in initialize
an event may be inserted or removed in one compared to the other simulation run
compare module states in differing events in the debugger
think about coming up with another fingerprint ingredients which ignores this difference
there may or may *not* be an error in the updated simulation
actually there even may be an error in the baseline
