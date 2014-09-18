//
// Copyright (C) 2008 Andras Varga
// Copyright (C) 2001 Matthias Oppitz, Arndt Buschmann
//           <matthias.oppitz@gmx.de>, <a.buschmann@gmx.de>
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
// Messages of type ~RTPSenderStatusMessage are used to send information
// from an rtp sender module to the application. Within this class a status
// string is defined in which the information is stored. This can be "PLAYING",
// "STOPPED" or "FINISHED".
// If a message must provide more information than just a string, a new class
// defining this parameter can derived.
//
enum RTPSenderStatus
{
    RTP_SENDER_STATUS_PLAYING = 1;
    RTP_SENDER_STATUS_FINISHED = 2;
    RTP_SENDER_STATUS_STOPPED = 3;
    RTP_SENDER_STATUS_PAUSED = 4;
}

packet RTPSenderStatusMessage
{
    short status @enum(RTPSenderStatus);
    uint32 timeStamp;
};
