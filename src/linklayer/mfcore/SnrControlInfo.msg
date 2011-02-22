//**************************************************************************
// * file:        SnrControlInfo.msg
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
}}


class noncobject SnrList;

//
// Control Info class that is used to send SNR information from
// snrEval to decider
//
// This class is used by the snrEval to send SNR information
// to the decider. After recieving a packet from the channel the
// snrEval calculates SNR information, writes this into the ~SnrControlInfo
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
// <pre>
// SnrListEntry listEntry;
// listEntry.time = snrStartingTime;
// listEntry.snr  = snrLevel;<br>
// listName.push_back( listEntry );
// </pre>
//
// When the list is complete and you want to send the message, you
// have to give the list to the control info  before attaching it to
// the ~AirFrame.
//
// <pre>
// SnrControlInfo* cInfo = new SnrControlInfo;
// cInfo->setSnrList( listName );
// msg->setControlInfo( cInfo );
// </pre>
//
// When receiving an ~AirFrame (in the deceider module) you probably want to
// read the list. Therefore you have to get a copy of that list:
//
// <pre>
// SnrControlInfo* cInfo = new SnrControlInfo;<br>
// cInfo = static_cast<~SnrControlInfo*>(msg->removeControlInfo());<br>
// SnrList receivedList* = new SnrList;<br>
// receivedList = cInfo->getSnrList();<br>
// delete cInfo;
// </pre>
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @author Marc Loebbers
//
class SnrControlInfo
{
    SnrList snrList;
}
