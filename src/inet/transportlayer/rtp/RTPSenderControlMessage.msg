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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

cplusplus {{
#include "inet/common/INETDefs.h"
}}

namespace inet::rtp;

//
// A message of the type ~RTPSenderControlMessage is created by an application to control
// the behavior of an rtp sender module. With this class the application can send a command
// string to the sender module like "PLAY" or "STOP". A message can have up to two float
// parameters.
//
// Following commands are evaluated in ~RTPPayloadSender (although correct behavior must
// be implemented in modules for certain payload types):
//    - PLAY : start playing from current position
//    - PAUSE : pause playing, stay at current position
//    - STOP : stop playing, go to beginning
//    - PLAY_UNTIL_TIME : start playing from current position and play until given temporal position (relative to
//      start of file is reached)
//    - PLAY_UNTIL_BYTE : start playing from current position and play until this data byte is reached
//    - SEEK_TIME : go to temporal position within the file (not allowed while playing)
//    - SEEK_BYTE : go to data byte (not allowed while playing)
//
enum RTPSenderControlMessageCommands
{
    RTP_CONTROL_PLAY = 1;
    RTP_CONTROL_PLAY_UNTIL_TIME = 2;
    RTP_CONTROL_PLAY_UNTIL_BYTE = 3;
    RTP_CONTROL_PAUSE = 4;
    RTP_CONTROL_STOP = 5;
    RTP_CONTROL_SEEK_TIME = 6;
    RTP_CONTROL_SEEK_BYTE = 7;
};

packet RTPSenderControlMessage
{
    short command @enum(RTPSenderControlMessageCommands);
    float commandParameter1;
    float commandParameter2;
};

