//***************************************************************************
//* file:        WirelessMacBase.ned
//*
//* derived by Andras Varga from BasicMacLayer.ned, which had the following
//* copyright:
//*
//* author:      Daniel Willkomm
//*
//* copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU Lesser General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//***************************************************************************
//* part of:     framework implementation developed by tkn
//* description: basic MAC layer class
//*              subclass to create your own MAC layer
//***************************************************************************

package inet.linklayer.mfcore;

//
// This is not an actual MAC implementation, just a prototype for wireless MAC's
// that use a MF (Mobility Framework)-based physical layer.
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @author Daniel Willkomm, Andras Varga
//
simple WirelessMac
{
    @display("i=block/ifcard");
    gates:
        input uppergateIn; // from the network layer
        output uppergateOut; // to the network layer
        input lowergateIn; // from the physical layer
        output lowergateOut; // to the physical layer
}

