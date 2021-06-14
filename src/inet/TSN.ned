//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

//
// @page TSN, Time-Sensitive Networking
//
// This page describes the INET Framework modules that implement a subset of the
// IEEE standards related to Time-Sensitive Networking (TSN). Some of them are
// specific to the IEEE standards, but several of them are more generic and they
// are simply reused to achieve the required functionality.
//
// <h2>Per-Stream Filtering and Policing (IEEE 802.1Qci-2017)</h2>
//
// TODO work in progress
//
// The simplest module for IEEE 802.1Q per-stream filtering and policing is the
// ~SimpleIeee8021qciFilter compound module. This module combines several submodules:
// a packet classifier at the input, a packet multiplexer at the output, and
// one packet meter, one packet filter, and one packet gate per stream. Each
// one of the latter per-stream 3 modules are optional.
//
// When a packet arrives at the input of the ~SimpleIeee8021qciFilter, it first gets
// classified into one of the filtering and policing submodule paths. Then the
// packet meter measures the packet as part of the packet stream that was seen
// so far, and attaches the result of the measurement. The result may be as
// simple as a label on the packet. After the metering, the packet filter checks
// if the packet matches the required conditions and either lets the packet go
// through or drops it. Finally, the packet gate allows the automatic time based
// or programmatic control of the packet passing through the selected path of the
// policing module. Packets are never enqueued in the ~SimpleIeee8021qciFilter, they
// either pass through or get dropped immedately.
//
// Note that any of the ~SimpleIeee8021qciFilter default submodules can be replaced
// with other variants. Moreover, other more complicated internal structures
// are also possible, this is especially the case when the packet meters are
// replaced with token bucket classifiers as described below.
//
// As the first step, the default policing process starts with a packet classifier,
// module, the ~StreamClassifier by default, that classifies packets based on the
// attached stream information. This classifier simply maps stream names to output
// gate indices. Please note that the stream decoding and identification process
// is not part of the ~SimpleIeee8021qciFilter.
//
// In the second step, the default policing process continues with a packet meter
// module, the ~DualRateThreeColorMeter by default, that labels the packets either
// as green, yellow or red based on the committed and excess information rate,
// and the committed and excess burst size parameters.
//
// The most commonly used packet meters for per-stream filtering and policing
// are:
//
//  - ~SingleRateTwoColorMeter labels packets as green or red based on CIR and
//    CBS parameters
//  - ~SingleRateThreeColorMeter labels packets as green, yellow or red based
//    on CIR, CBS and EBS parameters
//  - ~DualRateThreeColorMeter labels packets as green, yellow or red based on
//    CIR, CBS, EIR and EBS parameters
//
// The above modules are based on the following generic token bucket meter
// modules:
//
//  - ~TokenBucketMeter contains a single token bucket and labels packets one
//    of 2 labels
//  - ~MultiTokenBucketMeter contains an overflowing chain of N token buckets
//    and labels packets with one of N+1 labels
//
// Different packet meter modules can also be used by replacing the default
// packet meter submodules of ~SimpleIeee8021qciFilter. See the inet.queueing.meter
// NED package for alternatives.
//
// In the third step, the default per-stream filtering and policing process
// continues with a packet filter module, the ~LabelFilter by default, that drops
// the red packets and lets through the green and yellow ones by default. Of
// course, different packet filter modules can also be used by replacing the
// default filter submodules of ~SimpleIeee8021qciFilter. See the inet.queueing.filter
// NED package for alternatives.
//
// Finally, the default policing process finishes by merging the per-stream
// filtering and policing paths into a single output gate by using the generic
// ~PacketMultiplexer module. There's no need to prioritize between the per-stream
// paths here, because the packets pass through in zero simulation time.
//
// Different per-stream filtering and policing compound modules can also be
// created by combining the existing queueing and protocol element modules
// of the INET Framework. For example, instead of the packet meter modules,
// the token bucket based packet classifier modules give more freedom in terms
// of the module structure. See the inet.queueing NED package for more modules.
//
// The most commonly used packet classifiers for per-stream filtering and
// policing are:
//
//  - ~SingleRateTwoColorClassifier classifies packets to 2 ouptput gates based
//    on CIR and CBS parameters
//  - ~SingleRateThreeColorClassifier classifies packets to 3 ouptput gates based
//    on CIR, CBS and EBS parameters
//  - ~DualRateThreeColorClassifier classifies packets to 3 output gates based
//    on CIR, CBS, EIR and EBS parameters
//
// The above modules are derived from the generic token bucket classifier modules.
// These modules can also be used on their own and combined in many different
// ways with all the other queueing modules to achieve the desired per-stream
// filtering and policing.
//
//  - ~TokenBucketClassifier contains a single token bucket and classifies
//    packets to 2 output gates
//  - ~MultiTokenBucketClassifier contains an overflowing chain of N token buckets
//    and classifies packets to the N+1 output gates
//
// There is also a more complex per-stream filtering and policing module, called
// the ~Ieee8021qciFilter. This module is more similar to the architecture that
// is present in the IEEE 802.1Q standard. The ~Ieee8021qciFilter also combines
// several submodules but in a slightly different way than the ~SimpleIeee8021qciFilter.
// The most important difference is that this module can be mostly configured
// through a single streamFilterTable parameter.
//
// TODO further refine this module according to meeting with Philipp
//

package inet;

// TODO
//
// <h2>Enhancements for Scheduled Traffic (IEEE 802.1Qbv-2015)</h2>
//
// Credit based shaping:
//  - ~GatingPriorityQueue
//  - ~PeriodicGate
//  - ~CreditBasedGate
//
// Static configurators:
//  - ~TSNschedGateSchedulingConfigurator
//  - ~Z3GateSchedulingConfigurator
//  - ~SimpleGateSchedulingConfigurator
//
// <h2>Frame Replication and Elimination for Reliability (IEEE 802.1CB-2017)</h2>
//
// Stream handling modules:
//  - ~StreamIdentifier
//  - ~StreamSplitter
//  - ~StreamEncoder
//  - ~StreamDecoder
//  - ~StreamMerger
//  - ~StreamLayer
//
// Static configurators:
//  - ~StreamRedundancyConfigurator
//  - ~TsnConfigurator
//
// <h2>Timing and Synchronization for Time-Sensitive Applications (IEEE 802.1AS-2020)</h2>
//
// Clock time modeling:
//  - ~SettableClock
//  - ~ConstantDriftOscillator
//  - ~RandomDriftOscillator
//
// Time synchronization protocol:
//  - ~Gptp
//  - ~GptpBridge
//  - ~GptpEndstation
//  - ~GptpMaster
//  - ~GptpSlave
//
// <h2>Examples</h2>
//
// Some examples:
//  - ~ClockSynchronizationExampleNetwork
//  - ~EthernetCreditBasedShaperExample
//  - ~EthernetFilteringExample
//  - ~EthernetRedundancyExample
//  - ~RedundancyExample1
//  - ~RedundancyExample2
//  - ~TimeAwareShaperExample
//  - ~EthernetPreemptionExample
//  - ~EthernetCutthroughExample
//

package inet.linklayer.ethernet;
