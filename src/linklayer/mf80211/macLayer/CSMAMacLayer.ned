//***************************************************************************
//* file:        CSMAMacLayer.ned
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
//* description: MAC layer class for the CSMAMac
//**************************************************************************/


package inet.linklayer.mf80211.macLayer;

//
// CSMA MAC protocol
//
// This component has been taken over from Mobility Framework 1.0a5.
//
simple CSMAMacLayer
{
    parameters:
        bool debug = default(false); // debug switch
        int headerLength @unit(b);
        int queueLength;
        @display("i=prot2");
    gates:
        input uppergateIn;
        output uppergateOut;
        input lowergateIn;
        output lowergateOut;
}
