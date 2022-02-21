//
// Copyright (C) 2008 OpenSim Ltd.
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet::rtp;

//
// A message of the type ~RtpSenderControlMessage is created by an application to control
// the behavior of an rtp sender module. With this class the application can send a command
// string to the sender module like "PLAY" or "STOP". A message can have up to two float
// parameters.
//
// Following commands are evaluated in ~RtpPayloadSender (although correct behavior must
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
enum RtpSenderControlMessageCommands
{
    RTP_CONTROL_PLAY = 1;
    RTP_CONTROL_PLAY_UNTIL_TIME = 2;
    RTP_CONTROL_PLAY_UNTIL_BYTE = 3;
    RTP_CONTROL_PAUSE = 4;
    RTP_CONTROL_STOP = 5;
    RTP_CONTROL_SEEK_TIME = 6;
    RTP_CONTROL_SEEK_BYTE = 7;
};

packet RtpSenderControlMessage
{
    RtpSenderControlMessageCommands command;
    float commandParameter1;
    float commandParameter2;
};
