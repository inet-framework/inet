//***************************************************************************
// * file:        SnrEval.ned
// *
// * author:      Marc Loebbers, Andras Varga, Levente Meszaros
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


package inet.linklayer.mf80211.phyLayer.snrEval;

//
// Represents the radio. Frames received from higher layers (typically the
// MAC protocol) are transmitted on the radio channel, using the global
// ChannelControl module. Frames received on the radio channel will have
// their signal-noise ratio evaluated and passed up to the decider.
// (It is the decider's task to determine whether the frame was received
// correctly, and pass it up to the MAC layer).
//
// The receive/transmit channel (frequency) can be changed by sending
// SnrEval a blank cMessage with PHY_C_CONFIGURERADIO as message kind, and
// a PhyControlInfo object attached to it as control info, to hold the
// new channel number. If the radio is currently transmitting, the channel
// switch will actually take place after the transmission is completed.
//
// This component has been taken over from Mobility Framework 1.0a5.
//
//# FIXME merge docu with BasicSnrEval's
simple SnrEval
{
    parameters:
        double channelNumber = default(0); // channel identifier
        bool debug = default(false); // debug switch
        double transmitterPower @unit("mW");
        double bitrate @unit("bps");
        int headerLength @unit(b);
        double thermalNoise @unit("dBm");
        double pathLossAlpha;
        double sensitivity @unit(mW);
        @display("i=block/wrxtx");
    gates:
        input uppergateIn;   // from higher layer protocol (MAC)
        output uppergateOut; // to decider (decider connects to higher layer protocol, i.e. the MAC)
        input radioIn @labels(AirFrame); // to receive frames (AirFrame) on the radio channel
}

