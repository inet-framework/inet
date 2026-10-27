//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocolelement.redundancy;

//
// Merges member streams using the Vector Recovery Algorithm in IEEE Std
// 802.1CB-2017, 7.4.3.2-7.4.3.4 and 7.4.3.6. Each output stream in mapping has
// independent recovery state; inputs mapped to the same output share that state.
// All such inputs must originate from the same sequence generation instance.
// recoveryMapping can assign explicit instance names independently of output
// names, e.g. to remove tags for several independent compound streams.
//
// Accepts the first numbered frame after reset, eliminates duplicates, accepts
// unseen frames within the history, and rejects sequence numbers outside the
// interval (-historyLength, historyLength] relative to the reference, with the
// forward-boundary correction published in IEEE 802.1CBdb-2021 (item 0257).
// The ambiguous half-space difference 32768 is interpreted as -32768.
// Sequence numbers wrap modulo 65536. Valid SequenceNumberInd values are 0..65535;
// an absent tag or an out-of-range value represents a missing sequence number.
// Frames without StreamInd or without a mapping pass through unchanged.
//
// resetTimeout is rounded up to whole ticks. A timeout takes priority over
// ordinary frame arrivals at the same simulation time. After timeout the next
// numbered frame is accepted. Individual recovery reloads the timer on numbered
// discards as well, and requires separate recovery instances for member streams.
//
// Processes complete, validated frames; streaming/cut-through is unsupported
// (7.4.3). bufferSize is inherited but unused. Records aggregate recovery counters.
// Implements the 2017 functions with the CBdb-2021 forward-boundary correction
// and retains reset-history loss accounting. Latent error detection, management
// tables, MRA, and amendment requirements beyond this documented correction
// are outside this module's scope; this is not a claim of full FRER conformance.
//
simple VectorStreamMerger extends StreamMerger
{
    parameters:
        int historyLength = default(2); // frerSeqRcvyHistoryLength; supported range 2..32768.
        int resetTimeout @unit(ms) = default(100ms); // frerSeqRcvyResetMSec; integer milliseconds, 0 disables timeout. 100ms is a model default.
        int ticksPerSecond = default(1000); // Timer tick rate; at least 100.
        bool takeNoSequence = default(false); // frerSeqRcvyTakeNoSequence.
        bool individualRecovery = default(false); // frerSeqRcvyIndividualRecovery; one member stream per recovery instance.
        object recoveryMapping @mutable = default({}); // Optional input stream -> recovery instance name; otherwise uses the output stream name.
        @class(VectorStreamMerger);

        @signal[recoveryPassedPackets](type=long);
        @signal[recoveryDiscardedPackets](type=long);
        @signal[recoveryRoguePackets](type=long);
        @signal[recoveryOutOfOrderPackets](type=long);
        @signal[recoveryLostPackets](type=long);
        @signal[recoveryTaglessPackets](type=long);
        @signal[recoveryResets](type=long);

        @statistic[recoveryPassedPackets](record=last);
        @statistic[recoveryDiscardedPackets](record=last);
        @statistic[recoveryRoguePackets](record=last);
        @statistic[recoveryOutOfOrderPackets](record=last);
        @statistic[recoveryLostPackets](record=last);
        @statistic[recoveryTaglessPackets](record=last);
        @statistic[recoveryResets](record=last);

}
