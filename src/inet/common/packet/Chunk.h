//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __CHUNK_H_
#define __CHUNK_H_

#include <memory>
#include "ByteStream.h"

// TODO: flag for disabling serialization completely
// TODO: flag for flattening
// TODO: erroneous flag for deserialized chunks which can't be correctly represented

class Chunk : public cObject
{
  protected:
    bool isImmutable_ = false;
    bool isIncomplete_ = false;

  public:
    Chunk() { }
    Chunk(const Chunk &other) :
        isImmutable_(other.isImmutable_),
        isIncomplete_(other.isIncomplete_)
    {
    }
    virtual ~Chunk() { }

    bool isImmutable() const { return isImmutable_; }
    bool isMutable() const { return !isImmutable_; }
    void assertMutable() const { assert(!isImmutable_); }
    void assertImmutable() const { assert(isImmutable_); }
    void makeImmutable() { isImmutable_ = true; }

    bool isIncomplete() const { return isIncomplete_; }
    bool isComplete() const { return !isIncomplete_; }
    void assertComplete() const { assert(!isIncomplete_); }
    void assertIncomplete() const { assert(isIncomplete_); }
    void makeIncomplete() { isIncomplete_ = true; }

    virtual int64_t getByteLength() const = 0;

    // TODO: is it justified to have a separate replace? why not deserialize directly?
    virtual void replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength);
    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const { return nullptr; }

    virtual const char *getSerializerClassName() const { return nullptr; }

    virtual void serialize(ByteOutputStream& stream) const;
    virtual void deserialize(ByteInputStream& stream);

    virtual std::string str() const override {
        std::ostringstream os;
        os << "Chunk, byteLength = " << getByteLength();
        return os.str();
    }
};

inline std::ostream& operator<<(std::ostream& os, const Chunk *chunk) { return os << chunk->str(); }

inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk) { return os << chunk.str(); }

class cPacketChunk : public Chunk
{
  protected:
    cPacket *packet = nullptr;

  public:
    cPacketChunk() { }

    virtual int64_t getByteLength() const override { return packet->getByteLength(); }

    virtual std::string str() const override {
        std::ostringstream os;
        os << "cPacketChunk, packet = {" << packet->str() << "}";
        return os.str();
    }
};

class ByteArrayChunk : public Chunk
{
  protected:
    std::vector<uint8_t> bytes;

  public:
    ByteArrayChunk() { }

    ByteArrayChunk(const std::vector<uint8_t>& bytes) :
        bytes(bytes)
    {
    }

    void setBytes(const std::vector<uint8_t>& bytes) {
        assertMutable();
        this->bytes = bytes;
    }

    const std::vector<uint8_t>& getBytes() const { return bytes; }

    virtual int64_t getByteLength() const override { return bytes.size(); }

    virtual void replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength) override;

    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const override;

    virtual const char *getSerializerClassName() const override { return "ByteArrayChunkSerializer"; }

    virtual std::string str() const override {
        std::ostringstream os;
        os << "ByteArrayChunk, byteLength = " << getByteLength() << ", bytes = {";
        bool first = true;
        for (auto byte : bytes) {
            if (!first)
                os << ", ";
            else
                first = false;
            os << (int)byte;
        }
        os << "}";
        return os.str();
    }
};

class ByteLengthChunk : public Chunk
{
  protected:
    int64_t byteLength = -1;

  public:
    ByteLengthChunk() { }

    ByteLengthChunk(int64_t byteLength) :
        byteLength(byteLength)
    {
        assert(byteLength >= 0);
    }

    void setByteLength(int64_t byteLength) {
        assertMutable();
        assert(byteLength >= 0);
        this->byteLength = byteLength;
    }

    virtual int64_t getByteLength() const override { return byteLength; }

    virtual void replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength) override;

    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const override;

    virtual const char *getSerializerClassName() const override { return "ByteLengthChunkSerializer"; }

    virtual std::string str() const override {
        std::ostringstream os;
        os << "ByteLengthChunk, byteLength = " << byteLength;
        return os.str();
    }
};

class SliceChunk : public Chunk
{
  protected:
    std::shared_ptr<Chunk> chunk = nullptr;
    int64_t byteOffset = -1;
    int64_t byteLength = -1;

  public:
    SliceChunk() { }

    SliceChunk(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset = -1, int64_t byteLength = -1) :
        chunk(chunk),
        byteOffset(byteOffset == -1 ? 0 : byteOffset),
        byteLength(byteLength == -1 ? chunk->getByteLength() - this->byteOffset : byteLength)
    {
        chunk->assertImmutable();
        assert(this->byteOffset >= 0);
        assert(this->byteLength >= 0);
    }

    const std::shared_ptr<Chunk>& getChunk() const { return chunk; }

    void setByteOffset(int64_t byteOffset) {
        assertMutable();
        assert(byteOffset >= 0);
        this->byteOffset = byteOffset;
    }

    int64_t getByteOffset() const { return byteOffset; }

    void setByteLength(int64_t byteLength) {
        assertMutable();
        assert(byteLength >= 0);
        this->byteLength = byteLength;
    }

    virtual int64_t getByteLength() const override { return byteLength; }

    virtual void replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset = -1, int64_t byteLength = -1) override;

    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const override;

    virtual const char *getSerializerClassName() const override { return "SliceChunkSerializer"; }

    virtual std::string str() const override {
        std::ostringstream os;
        os << "SliceChunk, chunk = " << chunk << ", byteOffset = " << byteOffset << ", byteLength = " << byteLength;
        return os.str();
    }
};

class SequenceChunk : public Chunk, public std::enable_shared_from_this<SequenceChunk>
{
  public:
    class Iterator
    {
      protected:
        const std::shared_ptr<const SequenceChunk> chunk = nullptr;
        int64_t position = 0;
        int index = 0;

      public:
        Iterator(const std::shared_ptr<const SequenceChunk>& chunk) : chunk(chunk) { }
        Iterator(const Iterator &other) :
            chunk(other.chunk),
            position(other.position),
            index(other.index)
        {
        }

        int64_t getPosition() const { return position; }
        int getIndex() const { return index; }

        void move(int64_t byteLength);
        void seek(int64_t byteOffset);

        virtual int getStartIndex() const = 0;
        virtual int getEndIndex() const = 0;
        virtual int getIndexIncrement() const = 0;
        virtual const std::shared_ptr<Chunk>& getChunk() const = 0;
    };

    class ForwardIterator : public Iterator
    {
      public:
        ForwardIterator(const std::shared_ptr<const SequenceChunk>& chunk) : Iterator(chunk) { }
        ForwardIterator(const ForwardIterator &other) : Iterator(other) { }

        virtual int getStartIndex() const override { return 0; }
        virtual int getEndIndex() const override { return chunk->chunks.size() - 1; }
        virtual int getIndexIncrement() const override { return 1; }
        virtual const std::shared_ptr<Chunk>& getChunk() const override { return chunk->chunks[index]; }
    };

    class BackwardIterator : public Iterator
    {
      public:
        BackwardIterator(const std::shared_ptr<const SequenceChunk>& chunk) : Iterator(chunk) { }
        BackwardIterator(const ForwardIterator &other) : Iterator(other) { }

        virtual int getStartIndex() const override { return chunk->chunks.size() - 1; }
        virtual int getEndIndex() const override { return 0; }
        virtual int getIndexIncrement() const override { return -1; }
        virtual const std::shared_ptr<Chunk>& getChunk() const override { return chunk->chunks[chunk->chunks.size() - index - 1]; }
    };

  protected:
    std::vector<std::shared_ptr<Chunk>> chunks;

    void prependChunk(const std::shared_ptr<Chunk>& chunk);
    void prependChunk(const std::shared_ptr<SequenceChunk>& chunk);

    void appendChunk(const std::shared_ptr<Chunk>& chunk);
    void appendChunk(const std::shared_ptr<SliceChunk>& chunk);
    void appendChunk(const std::shared_ptr<SequenceChunk>& chunk);

  public:
    SequenceChunk() { }
    SequenceChunk(const SequenceChunk &other) :
        Chunk(other),
        chunks(other.chunks)
    {
    }

    void makeImmutable();

    const std::vector<std::shared_ptr<Chunk>>& getChunks() const { return chunks; }
    ForwardIterator createForwardIterator() const { return ForwardIterator(this->shared_from_this()); }
    BackwardIterator createBackwardIterator() const { return BackwardIterator(this->shared_from_this()); }

    // TODO: should we support polymorphic peek? what about current cPacket class hierarchies?
    template <typename T>
    std::shared_ptr<T> peek(const Iterator& iterator, int64_t byteLength = -1) const {
        return peekAt<T>(iterator, iterator.getPosition(), byteLength);
    }
    std::shared_ptr<SliceChunk> peek(const Iterator& iterator, int64_t byteLength = -1) const {
        return peekAt<SliceChunk>(iterator, iterator.getPosition(), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekAt(const Iterator& iterator, int64_t byteOffset, int64_t byteLength = -1) const {
        // fast path
        if (iterator.getIndex() != -1 && iterator.getIndex() != chunks.size() && byteOffset == iterator.getPosition()) {
            const auto& chunk = iterator.getChunk();
            if (byteLength == -1 || chunk->getByteLength() == byteLength) {
                if (auto castChunk = std::dynamic_pointer_cast<T>(chunk))
                    return castChunk;
            }
        }
        // linear search path
        int position = 0;
        int startIndex = iterator.getStartIndex();
        int endIndex = iterator.getEndIndex();
        int increment = iterator.getIndexIncrement();
        for (int i = startIndex; i != endIndex + increment; i += increment) {
            auto& chunk = chunks[i];
            if (byteOffset == position && (byteLength == -1 || byteLength == chunk->getByteLength())) {
                if (auto castChunk = std::dynamic_pointer_cast<T>(chunk))
                    return castChunk;
            }
            position += chunk->getByteLength();
        }
        // slow path
        // TODO: prevents easy access for application buffer assertImmutable();
        assert(increment == 1 || byteLength != -1);
        // TODO: move make_shared into deserialize to allow polymorphism?
        auto chunk = std::make_shared<T>();
        // TODO: eliminate const_cast
        chunk->replace(const_cast<SequenceChunk *>(this)->shared_from_this(), increment == 1 ? byteOffset : getByteLength() - byteOffset - byteLength, byteLength);
        chunk->makeImmutable();
        if ((chunk->isComplete() && byteLength == -1) || byteLength == chunk->getByteLength())
            return chunk;
        else
            return nullptr;
    }
    std::shared_ptr<SliceChunk> peekAt(const Iterator& iterator, int64_t byteOffset, int64_t byteLength) const {
        return peekAt<SliceChunk>(iterator, byteOffset, byteLength);
    }

    // TODO: implement these with insert()
    void prepend(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    void append(const std::shared_ptr<Chunk>& chunk, bool flatten = true);

    virtual int64_t getByteLength() const override;

    virtual const char *getSerializerClassName() const override { return "SequenceChunkSerializer"; }

    virtual std::string str() const override
    {
        std::ostringstream os;
        os << "[";
        bool first = true;
        for (auto& chunk : chunks) {
            if (!first)
                os << " | ";
            else
                first = false;
            os << chunk->str();
        }
        os << "]";
        return os.str();
    }
};

#endif // #ifndef __CHUNK_H_

