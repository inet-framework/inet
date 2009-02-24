//**************************************************************************
// * file:        MacPkt.msg
// *
// * author:      Daniel Willkomm
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
// **************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/

cplusplus {{
#include "MACAddress.h"
}}


class noncobject MACAddress;

//
// A basic MAC (Media Access Control) packet format definition
//
// subclass if you want to create your own MAC layer message class
//
// The basic MAC message only provides source and destination address
// and and the channel it should be send on
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @author Daniel Willkomm
//
//# FIXME just temporary here
//
packet MacPkt
{
    MACAddress destAddr; // destination mac address
    MACAddress srcAddr; // source mac address
    int channelId = 1; // channel to send / receive on
}
