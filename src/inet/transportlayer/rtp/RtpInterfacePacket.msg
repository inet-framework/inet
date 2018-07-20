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

namespace inet::rtp;

//
// An enumeration to distinguish the different functions of the
// RtpInterfacePacket.
//
enum RtpIfpType
{
    RTP_IFP_ENTER_SESSION = 1;
    RTP_IFP_SESSION_ENTERED = 2;
    RTP_IFP_CREATE_SENDER_MODULE = 3;
    RTP_IFP_SENDER_MODULE_CREATED = 4;
    RTP_IFP_DELETE_SENDER_MODULE = 5;
    RTP_IFP_SENDER_MODULE_DELETED = 6;
    RTP_IFP_SENDER_CONTROL = 7;
    RTP_IFP_SENDER_STATUS = 8;
    RTP_IFP_LEAVE_SESSION = 9;
    RTP_IFP_SESSION_LEFT = 10;
};

//
// The next classes are used for communication between an ~RtpApplication
// and an RTPLayer module. Its offers functionality for starting and stopping of an
// rtp session, transmission of files and feedback about the success of the
// operations.

message RtpControlMsg
{
};

// base class
class RtpControlInfo extends cObject
{
    RtpIfpType type;
};

// base class
class RtpSessionControlInfo extends RtpControlInfo
{
    uint32 ssrc;
};


// App to ~Rtp
class RtpCiEnterSession extends RtpControlInfo
{
    type = RTP_IFP_ENTER_SESSION;
    string commonName;
    string profileName;
    int bandwidth;
    Ipv4Address destinationAddress;
    int port;
};

// ~Rtp to App
class RtpCiSessionEntered extends RtpSessionControlInfo
{
    type = RTP_IFP_SESSION_ENTERED;
};

// App to ~Rtp
class RtpCiSenderControl extends RtpSessionControlInfo
{
    type = RTP_IFP_SENDER_CONTROL;
    // from ~RtpSenderControlMessage:
    RtpSenderControlMessageCommands command;
    float commandParameter1;
    float commandParameter2;
}

// ~Rtp to App
class RtpCiSenderStatus extends RtpSessionControlInfo
{
    type = RTP_IFP_SENDER_STATUS;
    // from ~RtpSenderStatusMessage:
    RtpSenderStatus status;
    uint32 timeStamp;
};

// App to ~Rtp
class RtpCiLeaveSession extends RtpControlInfo
{
    type = RTP_IFP_LEAVE_SESSION;
    // TODO do I need any sesson identifier?
};

// ~Rtp to App
class RtpCiSessionLeft extends RtpControlInfo
{
    type = RTP_IFP_SESSION_LEFT;
    // TODO do I need any sesson identifier?
};

// App to ~Rtp
class RtpCiCreateSenderModule extends RtpSessionControlInfo
{
    type = RTP_IFP_CREATE_SENDER_MODULE;
    int payloadType;
    string fileName;
};

// ~Rtp to App
class RtpCiSenderModuleCreated extends RtpSessionControlInfo
{
    type = RTP_IFP_SENDER_MODULE_CREATED;
};

// App to ~Rtp
class RtpCiDeleteSenderModule extends RtpSessionControlInfo
{
    type = RTP_IFP_DELETE_SENDER_MODULE;
};

// ~Rtp to App
class RtpCiSenderModuleDeleted extends RtpSessionControlInfo
{
    type = RTP_IFP_SENDER_MODULE_DELETED;
};
