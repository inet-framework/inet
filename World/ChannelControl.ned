//***************************************************************************
// * file:        ChannelControl.ned
// *
// * author:      Andras Varga, Steffen Sroka, Daniel Willkomm
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/


//
// ChannelControl has exactly one instance in every network model that 
// contains mobile or wireless nodes. This module gets informed about the 
// location and movement of nodes, and determines which nodes are within
// communication or interference distance. This info is then used by the 
// radio interfaces of nodes at transmissions.
//
// Side effect: updates the containing compound module's display string
// according to the given playground size (sets <tt>"p=0,0;b=$playgroundSizeX,
// $playgroundSizeY"</tt>).
//
// This ChannelControl is a different implementation from the one in 
// Mobility Framework 1.0a5: here we use sendDirect(), while the MF version 
// used normal send() and dynamic connections.
//
// @author Andras Varga (based on MF's ChannelControl by Steffen Sroka and Daniel Willkomm)
// @see BasicMobility
//
simple ChannelControl
    parameters:
        coreDebug: numeric const, // debug switch for core framework
        playgroundSizeX: numeric const, // x size of the playground (in meters)
        playgroundSizeY: numeric const, // y size of the playground (in meters)
        pMax: numeric const, // maximum sending power used for this network (in mW)
        sat: numeric const, // signal attenuation threshold (in dBm)
        alpha: numeric const, // path loss coefficient
        carrierFrequency: numeric const, // carrier frequency of the channel (in Hz)
        numChannels: numeric const; // number of radio channels (frequencies)
endsimple

