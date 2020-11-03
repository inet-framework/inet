//
// Copyright (C) 2001 Matthias Oppitz
// Copyright (C) 2010 OpenSim Ltd.
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//


import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet::rtp;

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
class RtpParticipantInfo extends cNamedObject
{
    @customize;  //FIXME need revise, several custommized functions should be moved to a processing module

    abstract uint32_t ssrc;

    // Used for storing the ip address of this endsystem.
    Ipv4Address address = Ipv4Address::UNSPECIFIED_ADDRESS;

    // Used for storing the port for ~RTP by this endsystem.
    int RTPPort = PORT_UNDEF;

    // Used for storing the port for ~RTCP by this endsystem.
    int RTCPPort = PORT_UNDEF;
};
