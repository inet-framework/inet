//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.common.TagBase;
import inet.common.ProtocolTag;
import inet.common.packet.chunk.Chunk;

namespace inet::inetmanet;

cplusplus{{
typedef Ptr<const Chunk> ChunkPtr;
}}

class ChunkPtr
{
    @existingClass;
    @descriptor(readonly);
    Chunk *content @getter(get);
}

class DsrProtocolInd extends ProtocolTagBase
{
    ChunkPtr dsrProtocolHeader = nullptr;
}

