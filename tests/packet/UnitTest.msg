//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.packet.chunk.Chunk;

namespace inet;

enum Protocol
{
    Tcp = 0;
    Ip = 1;
    Ethernet = 2;
}

enum CrcMode
{
    CRC_DECLARED_CORRECT = 0;
    CRC_DECLARED_INCORRECT  = 1;
    CRC_COMPUTED = 2;
}

class ApplicationHeader extends FieldsChunk
{
    @str;
    chunkLength = B(10);
    int16_t someData = -1;
}

class TcpHeader extends FieldsChunk
{
    @str;
    int16_t lengthField = -1;
    int16_t srcPort = -1;
    int16_t destPort = -1;
    int16_t crc = 0;
    int crcMode @enum(CrcMode) = -1;
}

class IpHeader extends FieldsChunk
{
    @str;
    chunkLength = B(20);
    int16_t protocol @enum(Protocol) = -1;
}

class EthernetHeader extends FieldsChunk
{
    @str;
    chunkLength = B(14);
    int16_t protocol @enum(Protocol) = -1;
}

class EthernetTrailer extends FieldsChunk
{
    @str;
    chunkLength = B(2);
    int16_t crc = 0;
    int crcMode @enum(CrcMode) = -1;
}

class CompoundHeader extends SequenceChunk
{
    @customize;
}

class TlvHeader extends FieldsChunk
{
    int8_t type = -1;
}

class TlvHeaderBool extends TlvHeader
{
    type = 1;
    chunkLength = B(3);
    bool boolValue = false;
}

class TlvHeaderInt extends TlvHeader
{
    type = 2;
    chunkLength = B(4);
    int16_t int16Value = -1;
}

class HeaderWithoutSerializer extends FieldsChunk
{
    @str;
    chunkLength = B(8);
    int64_t int64Value  = -1;
}
