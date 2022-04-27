//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

cplusplus {{
#ifndef HAVE_FFMPEG
#error Please install libavcodec, libavformat, libavutil or disable 'VoipStream' feature.
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

