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

#ifndef __INET_NEWREASSEMBLYBUFFER_H_
#define __INET_NEWREASSEMBLYBUFFER_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

class NewReassemblyBuffer : public cObject
{
  protected:
    class Region {
      public:
        int64_t byteOffset;
        std::shared_ptr<Chunk> data;

      public:
        Region(int64_t byteOffset, const std::shared_ptr<Chunk>& data);

        int64_t getStartOffset() const { return byteOffset; }
        int64_t getEndOffset() const { return byteOffset + data->getByteLength(); }

        static bool compareStartOffset(const Region& a, const Region& b) { return a.getStartOffset() < b.getStartOffset(); }
        static bool compareEndOffset(const Region& a, const Region& b) { return a.getEndOffset() < b.getEndOffset(); }
    };

  protected:
    std::vector<Region> regions;

  protected:
    void eraseEmptyRegions(std::vector<Region>::iterator begin, std::vector<Region>::iterator end);
    void sliceRegions(Region& newRegion);
    void mergeRegions(Region& previousRegion, Region& nextRegion);

  public:
    NewReassemblyBuffer() { }
    NewReassemblyBuffer(const NewReassemblyBuffer& other);

    virtual NewReassemblyBuffer *dup() const override { return new NewReassemblyBuffer(*this); }

    bool isComplete() const { return regions.size() == 1; }

    int64_t getByteLength() const;
    int64_t getByteOffset() const { return regions.empty() ? -1 : regions[0].byteOffset; }

    void setData(int64_t byteOffset, const std::shared_ptr<Chunk>& chunk);
    std::shared_ptr<Chunk> getData();

    virtual std::string str() const override { return ""; }
};

inline std::ostream& operator<<(std::ostream& os, const NewReassemblyBuffer *buffer) { return os << buffer->str(); }

inline std::ostream& operator<<(std::ostream& os, const NewReassemblyBuffer& buffer) { return os << buffer.str(); }

} // namespace

#endif // #ifndef __INET_NEWREASSEMBLYBUFFER_H_

