//TODO copyright header


cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/common/packet/FlatPacket.h"

}}

namespace inet;

packet FlatPacket;

class FlatChunk extends cOwnedObject
{
    @existingClass(true);
    @descriptor(readonly);
    int64_t chunkBitLength;
    int64_t chunkByteLength;
    FlatPacket* ownerPacket;     //FIXME readonly
}

class PacketChunk extends FlatChunk
{
    @existingClass(true);
    @descriptor(readonly);
    cPacket* _packet @getter(getPacket) @getval(.getPacket());
}

//class PacketSliceChunk : public Chunk
//{
//    cPacket *packet;        // shared ptr, counted
//    int64_t bitOffset;         // [bits]
//    int64_t bitLength;         // [bits]
//  public:
//    cPacket * getPacket();
//    void setSlice(cPacket *, int64_t byteOffset, int64_t byteLength);
//    void setSliceBits(cPacket *, int64_t bitOffset, int64_t bitLength);
//    int64_t getChunkBitLength() const override;
//    bool isComplete();
//};

packet FlatPacket       //TODO rename to Packet?
{
    @existingClass(true);
    @descriptor(readonly);
    FlatChunk* chunks[] @getter(getChunk) @sizeGetter(getNumChunks);
}

