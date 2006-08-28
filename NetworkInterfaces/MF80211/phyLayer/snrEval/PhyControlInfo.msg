//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


cplusplus {{
#include "INETDefs.h"
}}

//
// Command codes for controlling the physical layer (the radio). These constants
// should be set as message kind on a message sent to the TCP entity.
//
// @see TCPCommand, TCPOpenCommand, TCP
//
enum PhyCommandCode
{
    PHY_C_CHANGECHANNEL = 1; // change the channel
};

//
// Control info for controlling the physical layer (the radio).
//
class PhyControlInfo
{
    properties:
        omitGetVerb=true;
    fields:
        int channelNumber; // with PHY_C_CHANGECHANNEL: the channel to switch to
};


