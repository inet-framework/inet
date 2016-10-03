//TODO header

#ifndef __INET_COMMON_FLATPACKET_H
#define __INET_COMMON_FLATPACKET_H

#include "inet/common/INETDefs.h"

namespace inet {

class FlatPacket;

class Chunk : public cOwnedObject
{
    int64_t chunkBitLength = -1; //KLUDGE need for MSG: byteLength = x;
protected:
    void setChunkBitLength(int64_t x) const;     //TODO error when owned by FlatPacket?
    void setChunkByteLength(int64_t x) const { setChunkBitLength(x<<3); }
public:
    virtual int64_t getChunkBitLength() const;
    int64_t getChunkByteLength() const { return (getChunkBitLength() +7)>>3; }
    FlatPacket *getOwnerPacket() const;
};

class PacketChunk : public Chunk
{
    cPacket *packet = nullptr;        // shared ptr, counted
  public:
    PacketChunk(cPacket *);
    cPacket * getPacket();
    void setPacket(cPacket *);
    int64_t getChunkBitLength() const override;
};

class PacketSliceChunk : public Chunk
{
    cPacket *packet;        // shared ptr, counted
    int64_t bitOffset;         // [bits]
    int64_t bitLength;         // [bits]
  public:
    cPacket * getPacket();
    void setSlice(cPacket *, int64_t byteOffset, int64_t byteLength);
    void setSliceBits(cPacket *, int64_t bitOffset, int64_t bitLength);
    int64_t getChunkBitLength() const override;
    bool isComplete();
};

class FlatPacket : public cPacket       //TODO rename to Packet?
{
    std::vector <Chunk *> chunks;
  public:
    void pushHeader(Chunk *);
    void pushTrailer(Chunk *);
    Chunk *peekHeader();
    Chunk *peekTrailer();
    Chunk *popHeader();
    Chunk *popTrailer();
    int getNumChunks() const;
    Chunk *getChunk(int i);
    int getChunkIndex(const Chunk *) const;     // -1



    // overrides
  private:
    virtual void encapsulate(cPacket *) override { throw 1; }
    virtual cPacket *getEncapsulatedPacket() const override { throw 1; }
    virtual cPacket *decapsulate() override { throw 1; }
    void setByteLength(int64_t) { throw 1; }
    virtual void setBitLength(int64_t) override { throw 1; }
    virtual void addBitLength(int64_t delta) override { throw 1; }
  public:
    virtual int64_t getBitLength() const override;
};


//class IPv4Header : public Chunk ...

}

#endif // __INET_COMMON_FLATPACKET_H
