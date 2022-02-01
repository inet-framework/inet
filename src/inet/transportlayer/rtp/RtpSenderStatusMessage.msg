//
// Copyright (C) 2001 Matthias Oppitz, Arndt Buschmann
//           <matthias.oppitz@gmx.de>, <a.buschmann@gmx.de>
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet::rtp;

//
// Messages of type ~RtpSenderStatusMessage are used to send information
// from an rtp sender module to the application. Within this class a status
// string is defined in which the information is stored. This can be "PLAYING",
// "STOPPED" or "FINISHED".
// If a message must provide more information than just a string, a new class
// defining this parameter can derived.
//
enum RtpSenderStatus
{
    RTP_SENDER_STATUS_PLAYING = 1;
    RTP_SENDER_STATUS_FINISHED = 2;
    RTP_SENDER_STATUS_STOPPED = 3;
    RTP_SENDER_STATUS_PAUSED = 4;
}

packet RtpSenderStatusMessage
{
    RtpSenderStatus status;
    uint32_t timeStamp;
};
