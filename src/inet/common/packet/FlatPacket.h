//TODO header

#ifndef __INET_COMMON_FLATPACKET_H
#define __INET_COMMON_FLATPACKET_H

#include "inet/common/INETDefs.h"

namespace inet {

class FlatPacket;

class Chunk : public cOwnedObject
{
    int64_t chunkBitLength = -1; //KLUDGE need for MSG: byteLength = x;
    void copy(const Chunk& other);
  protected:
    void setChunkBitLength(int64_t x);     //TODO error when owned by FlatPacket?
    void setChunkByteLength(int64_t x) { setChunkBitLength(x<<3); }
  public:
    explicit Chunk(const char *name=nullptr, bool namepooling=true);
    Chunk(const Chunk& other);
    virtual ~Chunk();
    virtual Chunk *dup() const override { return new Chunk(*this); }

    virtual int64_t getChunkBitLength() const { return chunkBitLength; }
    int64_t getChunkByteLength() const { return (getChunkBitLength()+7)>>3; }
    FlatPacket *getOwnerPacket() const;
};

class PacketChunk : public Chunk
{
    cPacket *packet = nullptr;
    void copy(const PacketChunk& other);
  public:
    PacketChunk(cPacket *);
    PacketChunk(const PacketChunk& other);
    virtual ~PacketChunk();
    virtual PacketChunk *dup() const override { return new PacketChunk(*this); }
    cPacket *getPacket() { return packet; }
    const cPacket *getPacket() const  { return packet; }
    cPacket *removePacket();           // throw error when PacketChunk owned by a FlatPacket
    void setPacket(cPacket *);           // throw error when PacketChunk owned by a FlatPacket or chunk already own another packet
    int64_t getChunkBitLength() const override;
};

#if 0   //FIXME
class PacketSliceChunk : public Chunk
{
    cPacket *packet;        // shared ptr, counted
    int64_t bitOffset;         // [bits]
    int64_t bitLength;         // [bits]
  public:
    cPacket *getPacket();
    void setSlice(cPacket *, int64_t byteOffset, int64_t byteLength);
    void setSliceBits(cPacket *, int64_t bitOffset, int64_t bitLength);
    int64_t getChunkBitLength() const override;
    bool isComplete();
};
#endif

class FlatPacket : public cPacket       //TODO rename to Packet?
{
    std::vector <Chunk *> chunks;
    void copy(const FlatPacket& other);
  public:
    explicit FlatPacket(const char *name=nullptr, short kind=0, int64_t bitLength=0);
    FlatPacket(const FlatPacket& other);
    virtual ~FlatPacket();
    virtual FlatPacket *dup() const override { return new FlatPacket(*this); }
    void pushHeader(Chunk *);
    void pushTrailer(Chunk *);
    Chunk *peekHeader();
    Chunk *peekTrailer();
    const Chunk *peekHeader() const;
    const Chunk *peekTrailer() const;
    Chunk *popHeader();
    Chunk *popTrailer();
    int getNumChunks() const;
    Chunk *getChunk(int i);
    const Chunk *getChunk(int i) const;
    int getChunkIndex(const Chunk *) const;     // -1 means: chunk not in



    // overrides
  private:
    virtual void encapsulate(cPacket *) override { throw 1; }
    virtual cPacket *getEncapsulatedPacket() const override { return nullptr; }
    virtual cPacket *decapsulate() override { throw 1; }
    virtual void setBitLength(int64_t) override { throw 1; }
    virtual void addBitLength(int64_t delta) override { throw 1; }
  public:
    virtual int64_t getBitLength() const override;
};


//class IPv4Header : public Chunk ...

}

#endif // __INET_COMMON_FLATPACKET_H
