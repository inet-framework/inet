//
// Copyright (C) 2016 OpenSim Limited
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.csmaca;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;


//
// Implements a configurable CSMA/CA (Carrier Sense Multiple Access with Collision
// Avoidance) MAC protocol with optional acknowledgements and a retry mechanism.
// This implementation can be used for general wireless network simulations and
// can approximate basic 802.11b ad-hoc mode operation with appropriate settings.
//
// The CSMA/CA protocol operates by sensing the medium before transmission (carrier
// sense) and backing off if the medium is busy (collision avoidance). This approach
// reduces the probability of collisions while maintaining good channel utilization.
//
// Key features and configuration parameters:
//
// - Medium access control:
//   * Carrier sensing before transmission
//   * Random backoff mechanism with configurable contention window
//   * DIFS (DCF Interframe Space) before data transmission
//   * SIFS (Short Interframe Space) before ACK transmission
//
// - Reliability mechanisms:
//   * Optional acknowledgements (can be enabled/disabled)
//   * Configurable retry limit for failed transmissions
//   * Exponential backoff for retransmissions (contention window doubles after each failure)
//
// - Timing and performance parameters:
//   * Configurable bit rate for both data and ACK frames
//   * Customizable protocol overhead (header length, ACK frame length)
//   * Adjustable timing parameters (SIFS, DIFS, slot time)
//
// When acknowledgements are disabled, the MAC assumes all transmissions are successful.
// When enabled, the receiver responds with an ACK after SIFS, and the sender retransmits
// if no ACK is received within the timeout period. After reaching the retry limit,
// the frame is discarded, and a link break signal is emitted.
//
// Note: This implementation does not include duplicate detection to maintain simplicity,
// so lost ACKs may cause duplicate packets to be delivered to higher layers.
//
module CsmaCaMac extends MacProtocolBase like IMacProtocol
{
    parameters:
        string radioModule = default("^.radio"); // The path to the Radio module  //FIXME remove the default value
        string address @mutable = default("auto"); // MAC address as a hex string (12 hex digits) or
                                                   // "auto". "auto" values will be replaced by
                                                   // a generated MAC address in init stage 0.
        string fcsMode @enum("declared", "computed") = default("declared");
        bool useAck = default(true);
        double bitrate @unit(bps);
        int headerLength @unit(B) = default(17B); // Maximum 255 bytes, minimum 17B for serializing, see CsmaCaMacDataHeader
        int ackLength @unit(B) = default(14B); // Maximum 255 bytes, minimum 14B for serializing, see CsmaCaMacAckHeader
        double sifsTime @unit(s) = default(10us);
        double slotTime @unit(s) = default(20us);
        double difsTime @unit(s) = default(sifsTime + 2 * slotTime);
        double ackTimeout @unit(s) = default(dropUnit(ackLength * 8) / dropUnit(bitrate) * 1s + sifsTime + slotTime); // Measured from the end of data transmission; includes sifs, preamble, physical header, mac ack duration, 2x propagation time; by default, assumes slot time > preamble + physical header + 2x propagation time
        int mtu = default(1500);
        int cwMin = default(31); // Minimum contention window
        int cwMax = default(1023); // Maximum contention window
        int cwMulticast = default(cwMin); // Multicast contention window
        int retryLimit = default(7); // Maximum number of retries
        @class(CsmaCaMac);
        @signal[linkBroken](type=inet::Packet);
        @statistic[linkBroken](title="link break"; source=linkBroken; record=count; interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drop: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropRetryLimitReached](title="packet drop: retry limit reached"; source=packetDropReasonIsRetryLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
