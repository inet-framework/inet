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
import inet.common.packet.chunk.Chunk;

cplusplus {{
#ifndef HAVE_FFMPEG
#error Please install libavcodec, libavformat, libavutil or disable 'VoIPStream' feature.
#endif
}};

namespace inet;

//
// For the type field of VoIPPacket
//
enum VoipStreamPacketType
{
    VOICE = 1;
    SILENCE = 2;
}

//
// Packet type used by VoIPSourceApp and VoIPSinkApp
//
class VoipStreamPacket extends FieldsChunk
{
    short headerLength = 0;     // from voipHeaderSize parameter
    VoipStreamPacketType type;
    int codec;  // FFmpeg identifier of the codec
    short sampleBits;
    int sampleRate;
    int transmitBitrate;
    int samplesPerPacket;

    uint16_t seqNo; // packet sequence number
    uint32_t timeStamp; // increased with every packet
    uint32_t ssrc;  // currently unused

    uint16_t dataLength;
}

