//
// Copyright (C) 2017 Jan Peter Drees
// Copyright (C) 2015 Joaquim Oller
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.xmac;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;


//
// Implements the X-MAC protocol, an energy-efficient MAC protocol designed for
// wireless sensor networks. X-MAC uses a preamble sampling approach to reduce
// power consumption in duty-cycled networks where nodes periodically wake up
// to check for transmissions.
//
// Key features of X-MAC include:
// - Short preamble bursts with target address instead of a long continuous preamble
// - Early acknowledgment mechanism that allows receivers to interrupt the preamble
//   once they detect it, reducing energy consumption and latency
// - Duty cycling with configurable sleep and wake periods
// - Optional MAC-level acknowledgments for reliability
//
// The protocol operates through a state machine that transitions between sleep,
// channel assessment, preamble transmission/reception, and data transmission/reception
// states. Parameters allow fine-tuning of timing, power consumption, and reliability.
//
// For detailed protocol description, see the paper:
// "X-MAC: A Short Preamble MAC Protocol for Duty-Cycled Wireless Sensor Networks"
// http://www.cs.cmu.edu/~andersoe/papers/xmac-sensys.pdf
//
module XMac extends MacProtocolBase like IMacProtocol
{
    parameters:
        string address @mutable = default("auto"); // MAC address as a hex string (12 hex digits), or
                                                   // "auto". "auto" values will be replaced by
                                                   // a generated MAC address in init stage 0.

        // debug switch
        bool debug = default(false);
        // collect statistics?
        bool stats = default(true);

        // BMAC specific parameters
        // how long is one slot?
        double slotDuration @unit(s) = default(1s);

        // how long is the check interval (CCA)?
        double checkInterval @unit(s) = default(0.01s);

        // should we animate the nodes with colors depending on their state or
        // not?
        bool animation = default(true);

        // bit rate
        double bitrate @unit(bps) = default(19200 bps);

        // tx power
        double txPower @unit(W) = default(50 mW);

        // should mac send and expect acknowledgments?
        bool useMACAcks = default(false);

        // maximum number of frame retransmission
        // only used when the usage of MAC acks is enabled
        int macMaxFrameRetries = default(3);

        // length of MAC headers
        int headerLength @unit(b) = default(80b);
        int ctrlFrameLength @unit(b) = default(headerLength);

        // mtu
        int mtu @unit(B) = default(0B);

        // Maximum transmission attempts per data packet, when ACKs are used
        int maxTxAttempts = default(2);

        string radioModule = default("^.radio");   // The path to the Radio module  // FIXME remove the default value

        @class(XMac);
        @signal[packetDropped](type=inet::Packet);
        @selfMessageKinds(inet::XMacTypes);
}
