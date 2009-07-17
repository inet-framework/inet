//***************************************************************************
//* file:        Mac80211.ned
//*
//* author:      Marc Löbbers
//*
//* copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//***************************************************************************
//* part of:     framework implementation developed by tkn
//* description: MAC layer class for 802.11
//**************************************************************************/


package inet.linklayer.mf80211.macLayer;

//
// Implementation of the 802.11b MAC protocol. This module is intended
// to be used in combination with the SnrEval80211 and Decider80211
// modules as physical layer.
//
// This module supports ad-hoc mode only (does not generate or handle
// management frames), and does not support fragmentation, RTS threshold,
// duplication detection and recovery.
//
// This component has been taken over from Mobility Framework 1.0a5.
//
simple Mac80211
{
    parameters:
        string address = default("auto");       // MAC address as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by
                                // a generated MAC address in init stage 0.
        int maxQueueSize;
        bool rtsCts = default(false);
        double bitrate @unit("bps");
        int broadcastBackoff;
        int mtu = default(1500);
        @display("i=block/layer");
    gates:
        input uppergateIn @labels(Ieee802Ctrl);
        output uppergateOut;
        input lowergateIn @labels(Mac80211Pkt);
        output lowergateOut @labels(Mac80211Pkt);
}
