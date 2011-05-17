//**************************************************************************
// * file:        AirFrame.msg
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

cplusplus {{
#include "INETDefs.h"
#include "Coord.h"
#include "ModulationType.h"
}}


class noncobject Coord;
class noncobject ModulationType;

//
// Format of the messages that are sent to the channel
//
// subclass if you want to create your own ~AirFrame message class
//
// This message format is used to send a packet from a snrEval module
// to the channel. All other snrEval modules that can 'hear' this
// message can evaluate noise, interference etc. from the information
// contained in this message:
//
// If you need more fields for whatever reason, please do NOT create
// your own message! Just extend (subclass) this message format (see Omnet
// manual,chapter Messages - Inheritance among message classes).
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @todo As soon as opp_msgc also supports pointers we should replace
// the id with a pointer to the nodes coordinates itself.
// @author Marc Loebbers
//
packet AirFrame
{
    double pSend; // Power with which this packet is transmitted
    int channelNumber; // Channel on which the packet is sent
    simtime_t duration; // Time it takes to transmit the packet, in seconds
    double bitrate;
    double snr;
    double lossRate;
    double powRec; // Power in the receiver
    Coord senderPos;
    // multi gate support
    double carrierFrequency; // 
    double bandwidth;
    ModulationType modulationType;
}
//
// Support of multiples gates
//
//packet AirFrameExtended extends AirFrame
//{
//   double carrierFrequency; // 
//   double bandwidth;
//   ModulationType modulationType;
//}
