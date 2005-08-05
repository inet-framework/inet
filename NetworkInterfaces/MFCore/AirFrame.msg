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
#include "SnrList.h"
#include "Coord.h"
}}

class noncobject SnrList;
class noncobject Coord;

//
// Format of the messages that are sent to the channel
//
// subclass if you want to create your own AirFrame message class
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
// @todo As soon as opp_msgc also supports pointers we should replace
// the id with a pointer to the nodes coordinates itself.
// @author Marc Loebbers
//
message AirFrame
{
    fields:
        double pSend; // Power with which this packet is transmitted
        int channelId=1; // Channel on which the packet is sent
        double duration; // Time it takes to transmit the packet, in seconds!
        Coord senderPos;
};

//
// Control Info class that is used to send SNR information from
// snrEval to decider
//
// This class is used by the snrEval to send SNR information
// to the decider. After recieving a packet from the channel the
// snrEval calculates SNR information, writes this into the SnrControlInfo
// and attaches the ControlInfo to the Airframe.
//
// This control info can be used for complex information,
// i.e. different SNR levels over the transmission time of this
// packet. The parameter snrList is a dynamic list (see
// http://www.sgi.com/tech/stl/List.html). The list entries are
// defined by the struct SnrListEntry, which only contains two
// parameters of type double, time and SNR. These values are a certain
// SNR level and the time at which this SNR level started. The thing
// is that you can't manipulate the list directly in the class.
// Instead you have to create your own list with 'SnrList
// listName;'. Furthermore you have to create a new object of the struct
// SnrListEntry everytime you add something to the list.
//
// So, every list entry should look something like this:
//
// SnrListEntry listEntry;<br>
// listEntry.time = snrStartingTime;<br>
// listEntry.snr  = snrLevel;<br>
// listName.push_back( listEntry );
//
// When the list is complete and you want to send the message, you
// have to give the list to the control info  before attaching it to
// the AirFrame.
//
// SnrControlInfo* cInfo = new SnrControlInfo;<br>
// cInfo->setSnrList( listName );<br>
// msg->setControlInfo( cInfo );
//
// When receiving an AirFrame (in the deceider module) you probably want to
// read the list. Therefore you have to get a copy of that list:
//
// SnrControlInfo* cInfo = new SnrControlInfo;<br>
// cInfo = static_cast<SnrControlInfo*>(msg->removeControlInfo());<br>
// SnrList receivedList* = new SnrList;<br>
// receivedList = cInfo->getSnrList();<br>
// delete cInfo;
//
// @author Marc Loebbers
//
class SnrControlInfo
{
    fields:
        SnrList snrList;
};
