//
// Copyright (C) 2008 Andras Varga
// Copyright (C) 2001 Matthias Oppitz
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

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.transportlayer.rtp.RtpSenderControlMessage;
import inet.transportlayer.rtp.RtpSenderStatusMessage;

cplusplus {{
#include "inet/transportlayer/rtp/RtpPacket.h"
}}

namespace inet::rtp;

//
// This enumeration is a list of all possibly types of
// an ~RtpInnerPacket.
//
enum RtpInpType
{
    RTP_INP_UNDEF = 0;
    RTP_INP_INITIALIZE_PROFILE = 1;
    RTP_INP_PROFILE_INITIALIZED = 2;
    RTP_INP_INITIALIZE_RTCP = 3;
    RTP_INP_RTCP_INITIALIZED = 4;
    RTP_INP_CREATE_SENDER_MODULE = 5;
    RTP_INP_SENDER_MODULE_CREATED = 6;
    RTP_INP_DELETE_SENDER_MODULE = 7;
    RTP_INP_SENDER_MODULE_DELETED = 8;
    RTP_INP_INITIALIZE_SENDER_MODULE = 9;
    RTP_INP_SENDER_MODULE_INITIALIZED = 10;
    RTP_INP_SENDER_MODULE_CONTROL = 11;
    RTP_INP_SENDER_MODULE_STATUS = 12;
    RTP_INP_LEAVE_SESSION = 13;
    RTP_INP_SESSION_LEFT = 14;
    RTP_INP_DATA_OUT = 15;
    RTP_INP_DATA_IN = 16;
};

//
// This class is used for communication between submodules of the RTP layer module.
//
packet RtpInnerPacket
{
    @customize;  // see the generated C++ header for more info

    // The type of this ~RtpInnerPacket.
    RtpInpType type = RTP_INP_UNDEF;

    // The CNAME
    string commonName;

    // The MTU
    int mtu;

    // The Bandwidth
    int bandwidth;

    // The RTCP percentage
    int rtcpPercentage;

    // The IPv4 address
    Ipv4Address address = Ipv4Address::UNSPECIFIED_ADDRESS;

    // The port number
    int port = PORT_UNDEF;

    // The SSRC identifier
    uint32 ssrc;

    // The payload type
    int payloadType;

    // The file name
    string fileName;

    // The clock rate
    int clockRate;

    // The RTP time stamp base
    int timeStampBase;

    // The RTP sequence number base
    int sequenceNumberBase;
};
