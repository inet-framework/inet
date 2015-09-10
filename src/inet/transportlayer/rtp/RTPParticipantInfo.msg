//
// Copyright (C) 2001 Matthias Oppitz
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
}}

namespace inet::rtp;

class noncobject IPv4Address;

//
// This class is a super class for classes intended for storing information
// about ~RTP end systems.
// It has two subclasses: RTPReceiverInformation which is used for storing
// information about other system participating in an ~RTP session.
// RTPSenderInformation is used by an ~RTP endsystem for storing information
// about itself.
// sa RTPReceiverInformation
// sa RTPSenderInformation
//
class RTPParticipantInfo extends cNamedObject
{
    @customize(true);  // see the generated C++ header for more info

    abstract uint32 ssrc;

    // Used for storing the ip address of this endsystem.
    IPv4Address address = IPv4Address::UNSPECIFIED_ADDRESS;

    // Used for storing the port for ~RTP by this endsystem.
    int RTPPort = PORT_UNDEF;

    // Used for storing the port for ~RTCP by this endsystem.
    int RTCPPort = PORT_UNDEF;
};
