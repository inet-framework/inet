//
// Copyright (C) 2010 OpenSim Ltd.
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
