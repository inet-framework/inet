//
// Copyright (C) 2008-2010, Anna Foerster, NetLab, SUPSI, Switzerland.
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.bmac;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;


//
// Implements the B-MAC (Berkeley MAC) protocol, also known as Low Power Listening
// (LPL), designed for energy-efficient communication in wireless sensor networks.
// B-MAC uses a preamble sampling approach where nodes periodically wake up to
// check for channel activity.
//
// Key features of B-MAC include:
// - Long preamble transmission to ensure receiver detection during periodic wake-ups
// - Clear Channel Assessment (CCA) to detect ongoing transmissions
// - Configurable duty cycling for energy conservation
// - Optional MAC-level acknowledgments for reliability
//
// The protocol operates as follows:
// - Nodes sleep for a configurable duration (slotDuration)
// - Upon waking, nodes perform CCA to check for transmissions
// - If a transmission is detected, nodes stay awake to receive the data packet
// - When sending, nodes first transmit preambles for at least slotDuration to
//   wake up potential receivers, then send the actual data packet
// - If MAC acknowledgments are enabled, receivers send an immediate ACK after
//   receiving a data packet
//
// B-MAC is widely used in wireless sensor networks, including in the TinyOS
// operating system, due to its simplicity and energy efficiency.
//
// The protocol's finite state machine is illustrated in the BMAC-FSM.png file.
//
module BMac extends MacProtocolBase like IMacProtocol
{
    parameters:
        string address @mutable = default("auto"); // MAC address as a hex string (12 hex digits), or
                                                   // "auto". "auto" values will be replaced
                                                   // by a generated MAC address in init stage 0.
        // BMAC specific parameters
        // how long is one slot?
        double slotDuration @unit(s) = default(0.1s);

        int headerLength @unit(b) = default(10b);
        int ctrlFrameLength @unit(b) = default(headerLength);
        int mtu @unit(B) = default(0B);

        // how long is the check interval (CCA)?
        double checkInterval @unit(s) = default(0.01s);

        // should we animate the nodes with colors depending on their state or
        // not?
        bool animation = default(true);

        // bit rate
        double bitrate @unit(bps) = default(19200 bps);

        // should the MAC send and expect acknowledgments?
        bool useMACAcks = default(false);

        string radioModule = default("^.radio");   // The path to the Radio module  // FIXME remove default value

        int maxTxAttempts = default(2);     // Maximum transmission attempts per data packet, when ACKs are used

        @class(BMac);
        @signal[linkBroken](type=inet::Packet);
        @statistic[linkBroken](title="link break"; source=linkBroken; record=count; interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drop: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);

        @selfMessageKinds(inet::BMacType);
}
