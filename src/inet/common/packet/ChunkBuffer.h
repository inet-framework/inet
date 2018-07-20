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

#ifndef __INET_CHUNKBUFFER_H_
#define __INET_CHUNKBUFFER_H_

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class provides basic functionality for merging large data chunks from
 * out of order smaller data chunks.
 *
 * Internally, buffers stores the data in different kind of chunks. See the
 * Chunk class and its subclasses for details. All chunks are immutable in a
 * buffer. Chunks are automatically merged as they are replaced in the buffer,
 * and they are also shared among buffers when duplicating.
 *
 * In general, this class supports the following operations:
 *  - push at the tail and pop at the head
 *  - query the length
 *  - serialize to and deserialize from a sequence of bits or bytes
 *  - copy to a new queue
 *  - convert to a human readable string
 */
class INET_API ChunkBuffer : public cNamedObject
{
  friend class ChunkBufferDescriptor;
  friend class ChunkBuffer__RegionDescriptor;

  protected:
    class INET_API Region {
      friend class ChunkBuffer__RegionDescriptor;

      public:
        b offset;
        /**
         * This chunk is always immutable to allow arbitrary peeking. Nevertheless
         * it's reused if possible to allow efficient merging with newly added chunks.
         */
        Ptr<const Chunk> data;

      protected:
        const Chunk *getData() const { return data.get(); } // only for class descriptor

      public:
        Region(b offset, const Ptr<const Chunk>& data) : offset(offset), data(data) { }
        Region(const Region& other) : offset(other.offset), data(other.data) { }

        b getStartOffset() const { return offset; }
        b getEndOffset() const { return offset + data->getChunkLength(); }

        static bool compareStartOffset(const Region& a, const Region& b) { return a.getStartOffset() < b.getStartOffset(); }
        static bool compareEndOffset(const Region& a, const Region& b) { return a.getEndOffset() < b.getEndOffset(); }
    };

  protected:
    /**
     * The list of non-overlapping, non-connecting but continuous regions.
     */
    std::vector<Region> regions;

  protected:
    Region *getRegion(int i) const { return const_cast<Region *>(&regions[i]); } // only for class descriptor

    void eraseEmptyRegions(std::vector<Region>::iterator begin, std::vector<Region>::iterator end);
    void sliceRegions(Region& newRegion);
    void mergeRegions(Region& previousRegion, Region& nextRegion);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    ChunkBuffer(const char *name = nullptr);
    ChunkBuffer(const ChunkBuffer& other);

    virtual ChunkBuffer *dup() const override { return new ChunkBuffer(*this); }
    //@}

    /** @name Content querying functions */
    //@{
    /**
     * Returns true if the buffer is completely empty.
     */
    bool isEmpty() const { return regions.empty(); }

    /**
     * Returns the number non-overlapping, non-connecting but continuous regions
     */
    int getNumRegions() const { return regions.size(); }

    /**
     * Returns the length of the given region.
     */
    b getRegionLength(int index) const { return regions.at(index).data->getChunkLength(); }

    /**
     * Returns the start offset of the given region.
     */
    b getRegionStartOffset(int index) const { return regions.at(index).offset; }

    /**
     * Returns the end offset of the given region.
     */
    b getRegionEndOffset(int index) const { return regions.at(index).getEndOffset(); }

    /**
     * Returns the data of the given region in its current representation.
     */
    const Ptr<const Chunk>& getRegionData(int index) const { return regions.at(index).data; }
    //@}

    /**
     * Replaces the stored data at the provided offset with the data in the
     * chunk. Already existing data gets overwritten, and connecting data gets
     * merged with the provided chunk.
     */
    // TODO: add flag to decide between keeping or overwriting old data when replacing with new data
    void replace(b offset, const Ptr<const Chunk>& chunk);

    /**
     * Erases the stored data at the provided offset and length.
     */
    void clear(b offset, b length);

    /**
     * Erases all of the stored data.
     */
    void clear() { regions.clear(); }

    /**
     * Returns a human readable string representation.
     */
    virtual std::string str() const override;
};

inline std::ostream& operator<<(std::ostream& os, const ChunkBuffer *buffer) { return os << buffer->str(); }

inline std::ostream& operator<<(std::ostream& os, const ChunkBuffer& buffer) { return os << buffer.str(); }

} // namespace

#endif // #ifndef __INET_CHUNKBUFFER_H_

