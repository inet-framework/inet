//***************************************************************************
// * file:        ChannelControl.ned
// *
// * author:      Andras Varga, Steffen Sroka, Daniel Willkomm,
// *              Juan-Carlos Maureira, Alfonso Ariza
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

package inet.world;

//
// ChannelControlExtended has exactly one instance in every network model that
// contains mobile or wireless nodes. This module behaves exactly like ChannelControl
// except this supports multiple radio interfaces per host.
//
// @see IMobility
// @see ChannelControl
//
simple ChannelControlExtended
{
    parameters:
       bool coreDebug = default(false); // debug switch for core framework
        double playgroundSizeX = default(600); // x size of the playground (in meters)
        double playgroundSizeY = default(400); // y size of the playground (in meters)
        double pMax @unit("mW") = default(20mW); // maximum sending power used for this network (in mW)
        double sat @unit("dBm") = default(-110dBm); // signal attenuation threshold (in dBm)
        double alpha = default(2); // path loss coefficient
        double carrierFrequency @unit("Hz") = default(2.4GHz); // carrier frequency of the channel (in Hz)
        double percentage = default(0.1);
        int numChannels = default(1); // number of radio channels (frequencies)
        @display("i=misc/cloud");
        @labels(node);
}

