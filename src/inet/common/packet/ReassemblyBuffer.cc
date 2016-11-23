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

#include <algorithm>
#include "inet/common/packet/ReassemblyBuffer.h"
#include "inet/common/packet/SequenceChunk.h"

namespace inet {

NewReassemblyBuffer::Region::Region(int64_t offset, const std::shared_ptr<Chunk>& data) :
    offset(offset),
    data(data)
{
}

NewReassemblyBuffer::NewReassemblyBuffer(const NewReassemblyBuffer& other) :
    regions(other.regions)
{
}

int64_t NewReassemblyBuffer::getLength() const
{
    int64_t startOffset = 0;
    int64_t endOffset = 0;
    for (const auto& region : regions) {
        startOffset = std::max(startOffset, region.getStartOffset());
        endOffset = std::max(endOffset, region.getEndOffset());
    }
    return endOffset - startOffset;
}

void NewReassemblyBuffer::eraseEmptyRegions(std::vector<Region>::iterator begin, std::vector<Region>::iterator end)
{
    regions.erase(std::remove_if(begin, end, [](const Region& region) { return region.data == nullptr; }), regions.end());
}

void NewReassemblyBuffer::sliceRegions(Region& newRegion)
{
    for (auto it = regions.begin(); it != regions.end(); it++) {
        auto& oldRegion = *it;
        if (newRegion.getEndOffset() <= oldRegion.getStartOffset() || oldRegion.getEndOffset() <= newRegion.getStartOffset())
            // no intersection
            continue;
        else if (newRegion.getStartOffset() <= oldRegion.getStartOffset() && oldRegion.getEndOffset() <= newRegion.getEndOffset()) {
            // new totally covers old
            oldRegion.data = nullptr;
            regions.erase(it--);
        }
        else if (oldRegion.getStartOffset() < newRegion.getStartOffset() && newRegion.getEndOffset() < oldRegion.getEndOffset()) {
            // new splits old into two parts
            Region previousRegin(oldRegion.getStartOffset(), oldRegion.data->peek(0, newRegion.getStartOffset() - oldRegion.getStartOffset()));
            Region nextRegion(newRegion.getEndOffset(), oldRegion.data->peek(newRegion.getEndOffset() - oldRegion.getStartOffset(), oldRegion.getEndOffset() - newRegion.getEndOffset()));
            oldRegion.offset = nextRegion.offset;
            oldRegion.data = nextRegion.data;
            regions.insert(it, previousRegin);
            return;
        }
        else {
            // new cuts beginning or end of old
            int64_t peekStartOffset = std::min(newRegion.getStartOffset(), oldRegion.getStartOffset());
            int64_t peekEndOffset = std::min(newRegion.getEndOffset(), oldRegion.getEndOffset());
            oldRegion.data = oldRegion.data->peek(peekStartOffset, peekEndOffset - peekStartOffset);
        }
    }
}

void NewReassemblyBuffer::mergeRegions(Region& previousRegion, Region& nextRegion)
{
    if (previousRegion.getEndOffset() == nextRegion.getStartOffset()) {
        // consecutive regions
        if (previousRegion.data->isMutable() && previousRegion.data->insertToEnd(nextRegion.data))
            // merge into previous
            nextRegion.data = nullptr;
        else if (nextRegion.data->isMutable() && nextRegion.data->insertToBeginning(previousRegion.data))
            // merge into next
            previousRegion.data = nullptr;
        else {
            // merge as a sequence
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->append(previousRegion.data);
            sequenceChunk->append(nextRegion.data);
            previousRegion.data = sequenceChunk;
            nextRegion.data = nullptr;
        }
    }
}

void NewReassemblyBuffer::setData(int64_t offset, const std::shared_ptr<Chunk>& chunk)
{
    Region newRegion(offset, chunk);
    sliceRegions(newRegion);
    if (regions.empty())
        regions.push_back(newRegion);
    else if (regions.back().getEndOffset() <= offset) {
        regions.push_back(newRegion);
        mergeRegions(regions[regions.size() - 2], regions[regions.size() - 1]);
        eraseEmptyRegions(regions.end() - 2, regions.end());
    }
    else if (offset + chunk->getChunkLength() <= regions.front().getStartOffset()) {
        regions.insert(regions.begin(), newRegion);
        mergeRegions(regions[0], regions[1]);
        eraseEmptyRegions(regions.begin(), regions.begin() + 2);
    }
    else {
        auto it = std::upper_bound(regions.begin(), regions.end(), newRegion, Region::compareStartOffset);
        it = regions.insert(it, newRegion);
        auto& previousRegion = *(it - 1);
        auto& region = *it;
        auto& nextRegion = *(it + 1);
        mergeRegions(previousRegion, region);
        mergeRegions(region.data != nullptr ? region : previousRegion, nextRegion);
        eraseEmptyRegions(it - 1, it + 1);
    }
}

std::shared_ptr<Chunk> NewReassemblyBuffer::getData()
{
    return isComplete() ? regions[0].data : nullptr;
}

} // namespace
