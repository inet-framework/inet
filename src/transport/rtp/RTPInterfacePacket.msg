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

cplusplus {{
#include "INETDefs.h"
#include "IPAddress.h"
#include "RTPSenderControlMessage_m.h"
#include "RTPSenderStatusMessage_m.h"
}}

class noncobject IPAddress;

enum RTPSenderControlMessageCommands;

enum RTPSenderStatus;

//
// An enumeration to distinguish the different functions of the
// RTPInterfacePacket.
//
enum RTP_IFP_TYPE
{
    RTP_IFP_UNDEF = 0;
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
// The next classes are used for communication between an RTPApplication
// and an RTPLayer module. Its offers functionality for starting and stopping of an
// rtp session, transmission of files and feedback about the success of the
// operations.

message RTPControlMsg
{
};

class RTPControlInfo
{
    short type @enum(RTP_IFP_TYPE);
};

class RTPCIEnterSession extends RTPControlInfo
{
    type = RTP_IFP_ENTER_SESSION;
    string commonName;
    string profileName;
    int bandwidth;
    IPAddress destinationAddress;
    int port;
};

class RTPCISessionEntered extends RTPControlInfo
{
    type = RTP_IFP_SESSION_ENTERED;
    uint32 ssrc;
};

class RTPCISenderControl extends RTPControlInfo
{
    type = RTP_IFP_SENDER_CONTROL;
    uint32 ssrc;
    // from RTPSenderControlMessage:
    short command @enum(RTPSenderControlMessageCommands);
    float commandParameter1;
    float commandParameter2;
}

class RTPCISenderStatus extends RTPControlInfo
{
    type = RTP_IFP_SENDER_STATUS;
    uint32 ssrc;
    // from RTPSenderStatusMessage:
    short status @enum(RTPSenderStatus);
    uint32 timeStamp;
};

class RTPCILeaveSession extends RTPControlInfo
{
    type = RTP_IFP_LEAVE_SESSION;
    // TODO do I need any sesson identifier?
};

class RTPCISessionLeft extends RTPControlInfo
{
    type = RTP_IFP_SESSION_LEFT;
    // TODO do I need any sesson identifier?
};

class RTPCICreateSenderModule extends RTPControlInfo
{
    type = RTP_IFP_CREATE_SENDER_MODULE;
    uint32 ssrc;
    int payloadType;
    string fileName;
};

class RTPCISenderModuleCreated extends RTPControlInfo
{
    type = RTP_IFP_SENDER_MODULE_CREATED;
    uint32 ssrc;
};

class RTPCIDeleteSenderModule extends RTPControlInfo
{
    type = RTP_IFP_DELETE_SENDER_MODULE;
    uint32 ssrc;
};

class RTPCISenderModuleDeleted extends RTPControlInfo
{
    type = RTP_IFP_SENDER_MODULE_DELETED;
    uint32 ssrc;
};
