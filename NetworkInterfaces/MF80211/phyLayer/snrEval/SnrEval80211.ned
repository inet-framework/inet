//***************************************************************************
// * file:        SnrEval80211.ned
// *
// * author:      Marc Loebbers
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
// This component has been taken over from Mobility Framework 1.0a5.
//
simple SnrEval80211
    parameters:
        channelNumber: numeric const, // channel identifier
        debug: bool, // debug switch
        transmitterPower : numeric const,
        bitrate: numeric const,
        headerLength: numeric const,
        carrierFrequency: numeric const,
        thermalNoise: numeric const,
        pathLossAlpha: numeric const,
        sensitivity: numeric const;
    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: radioIn; // to receive AirFrames
endsimple
