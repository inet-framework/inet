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

#include "inet/common/packet/Chunk.h"

namespace inet {

/**
 * This class provides basic functionality for merging large data chunks from
 * out of order smaller data chunks.
 */
class INET_API ChunkBuffer : public cNamedObject
{
  protected:
    class INET_API Region {
      public:
        int64_t offset;
        /**
         * This chunk is always immutable to allow arbitrary peeking. Nevertheless
         * it's reused if possible to allow efficient merging with newly added chunks.
         */
        std::shared_ptr<Chunk> data;

      public:
        Region(int64_t offset, const std::shared_ptr<Chunk>& data) : offset(offset), data(data) { }
        Region(const Region& other) : offset(other.offset), data(other.data) { }

        int64_t getStartOffset() const { return offset; }
        int64_t getEndOffset() const { return offset + data->getChunkLength(); }

        static bool compareStartOffset(const Region& a, const Region& b) { return a.getStartOffset() < b.getStartOffset(); }
        static bool compareEndOffset(const Region& a, const Region& b) { return a.getEndOffset() < b.getEndOffset(); }
    };

  protected:
    /**
     * The list of non-overlapping, non-connecting but continuous regions.
     */
    std::vector<Region> regions;

  protected:
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
    bool isEmpty() const { return regions.empty(); }
    int getNumRegions() const { return regions.size(); }
    int64_t getRegionLength(int index) const { assert(0 <= index && index < regions.size()); return regions[index].data->getChunkLength(); }
    int64_t getRegionStartOffset(int index) const { assert(0 <= index && index < regions.size()); return regions[index].offset; }
    int64_t getRegionEndOffset(int index) const { assert(0 <= index && index < regions.size()); return regions[index].getEndOffset(); }
    const std::shared_ptr<Chunk>& getRegionData(int index) const { assert(0 <= index && index < regions.size()); return regions[index].data; }
    //@}

    /**
     * Replaces the stored data at the provided offset with the data in the
     * chunk. Already existing data gets overwritten, and connecting data gets
     * merged with the provided chunk.
     */
    // TODO: add flag to decide between keeping or overwriting old data when replacing with new data
    void replace(int64_t offset, const std::shared_ptr<Chunk>& chunk);

    /**
     * Erases the stored data at the provided offset and length.
     */
    void clear(int64_t offset, int64_t length);

    /**
     * Erases all of the stored data.
     */
    void clear() { regions.clear(); }

    virtual std::string str() const override;
};

inline std::ostream& operator<<(std::ostream& os, const ChunkBuffer *buffer) { return os << buffer->str(); }

inline std::ostream& operator<<(std::ostream& os, const ChunkBuffer& buffer) { return os << buffer.str(); }

} // namespace

#endif // #ifndef __INET_CHUNKBUFFER_H_

