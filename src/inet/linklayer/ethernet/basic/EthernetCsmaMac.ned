//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.basic;

import inet.linklayer.base.MacProtocolBase;

simple EthernetCsmaMac extends MacProtocolBase
{
    parameters:
        bool sendRawBytes = default(false); // when true packets are serialized into a sequence of bytes before sending out
        bool promiscuous = default(false);  // if true, all packets are received, otherwise only the
                                            // ones with matching destination MAC address
        int mtu @unit(B) = default(1500B);
        string fcsMode @enum("declared","computed");
        @class(EthernetCsmaMac);
        @signal[carrierSenseChanged](type=int);
        @signal[collisionChanged](type=int);
        @signal[stateChanged](type=int);
        @statistic[carrierSense](title="carrier sense"; type=int; source=carrierSenseChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[collision](title="collision"; type=int; source=collisionChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[state](title="state"; type=enum; enum=IDLE, WAIT_IFG, TRANSMITTING, JAMMING, BACKOFF, RECEIVING; source=stateChanged; record=count,vector; interpolationmode=sample-hold);
}

