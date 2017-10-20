//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;

cplusplus {{
#ifndef HAVE_FFMPEG
#error Please install libavcodec, libavformat, libavutil or disable 'VoIPStream' feature.
#endif

#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/common/ByteArray.h"
}};

namespace inet;

class FieldsChunk extends cObject;

//
// For the type field of VoIPPacket
//
enum VoIPStreamPacketType
{
    VOICE = 1;
    SILENCE = 2;
}

class noncobject ByteArray;

//
// Packet type used by VoIPSourceApp and VoIPSinkApp
//
class VoIPStreamPacket extends FieldsChunk
{
    int type @enum(VoIPStreamPacketType);

    int codec;  // FFmpeg identifier of the codec
    short sampleBits;
    int sampleRate;
    int transmitBitrate;
    int samplesPerPacket;
    ByteArray bytes;

    uint16_t seqNo; // packet sequence number
    uint32_t timeStamp; // increased with every packet
    uint32_t ssrc;  // currently unused
}
