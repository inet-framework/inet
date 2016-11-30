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

#ifndef __INET_REASSEMBLYBUFFER_H_
#define __INET_REASSEMBLYBUFFER_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

/**
 * This class provides functionality for reassembling large data chunks from
 * out of order smaller data chunks. This can be useful for reassembling data
 * segments in connection oriented transport layer protocols or reassembling
 * fragmented datagrams in network layer protocols.
 */
// TODO: add keep old or overwrite with new flag
class INET_API ReassemblyBuffer : public cNamedObject
{
  protected:
    class INET_API Region {
      public:
        int64_t offset;
        std::shared_ptr<Chunk> data;

      public:
        Region(int64_t offset, const std::shared_ptr<Chunk>& data);

        int64_t getStartOffset() const { return offset; }
        int64_t getEndOffset() const { return offset + data->getChunkLength(); }

        static bool compareStartOffset(const Region& a, const Region& b) { return a.getStartOffset() < b.getStartOffset(); }
        static bool compareEndOffset(const Region& a, const Region& b) { return a.getEndOffset() < b.getEndOffset(); }
    };

  protected:
    /**
     * The list of non-overlapping, non-connecting but contiguous regions.
     */
    std::vector<Region> regions;

  protected:
    void eraseEmptyRegions(std::vector<Region>::iterator begin, std::vector<Region>::iterator end);
    void sliceRegions(Region& newRegion);
    void mergeRegions(Region& previousRegion, Region& nextRegion);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    ReassemblyBuffer(const char *name = nullptr);
    ReassemblyBuffer(const ReassemblyBuffer& other);

    virtual ReassemblyBuffer *dup() const override { return new ReassemblyBuffer(*this); }
    //@}

    /** @name Content accessor functions */
    //@{
    int getNumRegions() const { return regions.size(); }
    int64_t getRegionOffset(int index) const { return regions[index].offset; }
    int64_t getRegionLength(int index) const { return regions[index].data->getChunkLength(); }
    const std::shared_ptr<Chunk>& getRegionData(int index) const { return regions[index].data; }
    //@}

    /**
     * Replaces the stored data at the offset with the provided data in the
     * chunk. Already existing data gets overwritten, and connecting data gets
     * merged with the provided chunk.
     */
    void replace(int64_t offset, const std::shared_ptr<Chunk>& chunk);

    /**
     * Removed the stored data at the provided offset and length.
     */
    void clear(int64_t offset, int64_t length);

    virtual std::string str() const override { return ""; }
};

// TODO: move to the network layer
class INET_API DatagramReassemblyBuffer : public ReassemblyBuffer
{
  protected:
    int64_t expectedLength;

  public:
    DatagramReassemblyBuffer(int64_t expectedLength = -1) : expectedLength() { }
    DatagramReassemblyBuffer(const DatagramReassemblyBuffer& other) :
        ReassemblyBuffer(other),
        expectedLength(other.expectedLength)
    { }

    int64_t getExpectedLength() const { return expectedLength; }
    void setExpectedLength(int64_t expectedLength) { this->expectedLength = expectedLength; }

    bool isComplete() const {
        return regions.size() == 1 && regions[0].offset == 0 && regions[0].data->getChunkLength() == expectedLength;
    }

    std::shared_ptr<Chunk> getData() const {
        return isComplete() ? regions[0].data : nullptr;
    }
};

// TODO: move to the transport layer
class INET_API SegmentReassemblyBuffer : public ReassemblyBuffer
{
  protected:
    int64_t expectedOffset;

  public:
    SegmentReassemblyBuffer(int64_t expectedOffset = -1) : expectedOffset(expectedOffset) { }
    SegmentReassemblyBuffer(const SegmentReassemblyBuffer& other) :
        ReassemblyBuffer(other),
        expectedOffset(other.expectedOffset)
    { }

    int64_t getExpectedOffset() const { return expectedOffset; }
    void setExpectedOffset(int64_t expectedOffset) { this->expectedOffset = expectedOffset; }

    std::shared_ptr<Chunk> popData() {
        if (regions.size() > 0 && regions[0].offset == expectedOffset) {
            const auto& data = regions[0].data;
            int64_t length = data->getChunkLength();
            clear(expectedOffset, length);
            expectedOffset += length;
            return data;
        }
        else
            return nullptr;
    }
};

inline std::ostream& operator<<(std::ostream& os, const ReassemblyBuffer *buffer) { return os << buffer->str(); }

inline std::ostream& operator<<(std::ostream& os, const ReassemblyBuffer& buffer) { return os << buffer.str(); }

} // namespace

#endif // #ifndef __INET_REASSEMBLYBUFFER_H_

