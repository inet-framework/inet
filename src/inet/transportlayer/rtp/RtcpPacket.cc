//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include <iostream>
#include <sstream>

#include "inet/transportlayer/rtp/RtcpPacket_m.h"

namespace inet {

namespace rtp {

void RtcpPacket::paddingAndSetLength()
{
    handleChange();
    int64_t chunkBits = getChunkLength().get();
    rtcpLength = (chunkBits + 31) / 32 - 1;
    setChunkLength(b((rtcpLength + 1) * 32));
}

void RtcpReceiverReportPacket::addReceptionReport(ReceptionReport *report)
{
    handleChange();
    receptionReports.add(report);
    count++;
    // an rtcp receiver report is 24 bytes long
    addChunkLength(B(24));
};

void RtcpSdesPacket::addSDESChunk(SdesChunk *sdesChunk)
{
    handleChange();
    sdesChunks.add(sdesChunk);
    count++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    addChunkLength(B(sdesChunk->getLength()));
};

} // namespace rtp

} // namespace inet

