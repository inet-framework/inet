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

#include "inet/common/packet/tag/RegionTagSet.h"

namespace inet {

void RegionTagSet::addTag(b offset, b length, const Ptr<const TagBase>& tag)
{
    ensureTagsVectorAllocated();
    prepareTagsVectorForUpdate();
    regionTags->push_back(RegionTag<TagBase>(offset, length, tag));
}

std::vector<RegionTagSet::RegionTag<TagBase>> RegionTagSet::addTagsWhereAbsent(const std::type_info& typeInfo, b offset, b length, const Ptr<const TagBase>& tag)
{
    std::vector<RegionTagSet::RegionTag<TagBase>> result;
    b endOffset = offset + length;
    b o = offset;
    if (regionTags != nullptr) {
        for (auto& regionTag : *regionTags) {
            auto tag = regionTag.getTag();
            auto tagObject = tag.get();
            if (typeInfo == typeid(*tagObject)) {
                b l = regionTag.getStartOffset() - o;
                if (l > b(0)) {
                    if (l > endOffset - o)
                        l = endOffset - o;
                    result.push_back(RegionTag<TagBase>(o, l, tag->dupShared()));
                    o += l;
                }
                if (regionTag.getEndOffset() > o)
                    o = regionTag.getEndOffset();
                if (o >= endOffset)
                    break;
            }
        }
    }
    b l = endOffset - o;
    if (l > b(0))
        result.push_back(RegionTag<TagBase>(o, l, tag->dupShared()));
    if (!result.empty()) {
        ensureTagsVectorAllocated();
        prepareTagsVectorForUpdate();
        for (auto& regionTag : result)
            regionTags->push_back(RegionTag<TagBase>(regionTag.getOffset(), regionTag.getLength(), regionTag.getTag()));
        sortTagsVector();
    }
    return result;
}

const Ptr<TagBase> RegionTagSet::removeTag(int index)
{
    prepareTagsVectorForUpdate();
    const Ptr<TagBase> tag = getTagForUpdate(index);
    regionTags->erase(regionTags->begin() + index);
    if (regionTags->size() == 0)
        regionTags = nullptr;
    return constPtrCast<TagBase>(tag);
}

void RegionTagSet::mapAllTags(b offset, b length, std::function<void (b, b, const Ptr<const TagBase>&)> f) const
{
    if (regionTags != nullptr) {
        b startOffset = offset;
        b endOffset = offset + length;
        for (auto& regionTag : *regionTags) {
            if (endOffset <= regionTag.getStartOffset() || regionTag.getEndOffset() <= startOffset)
                // no intersection
                continue;
            else if (startOffset <= regionTag.getStartOffset() && regionTag.getEndOffset() <= endOffset)
                // totally covers region
                f(regionTag.getOffset(), regionTag.getLength(), regionTag.getTag());
            else if (regionTag.getStartOffset() < startOffset && endOffset < regionTag.getEndOffset())
                // splits region into two parts
                f(startOffset, endOffset - startOffset, regionTag.getTag());
            else if (regionTag.getEndOffset() <= endOffset)
                // cuts end of region
                f(startOffset, regionTag.getEndOffset() - startOffset, regionTag.getTag());
            else if (startOffset <= regionTag.getStartOffset())
                // cuts beginning of region
                f(regionTag.getStartOffset(), endOffset - regionTag.getStartOffset(), regionTag.getTag());
            else
                ASSERT(false);
        }
    }
}

void RegionTagSet::mapAllTagsForUpdate(b offset, b length, std::function<void (b, b, const Ptr<TagBase>&)> f)
{
    if (regionTags != nullptr) {
        prepareTagsVectorForUpdate();
        b startOffset = offset;
        b endOffset = offset + length;
        for (int i = 0; i < regionTags->size(); i++) {
            const RegionTag<TagBase>& regionTag = (*regionTags)[i];
            if (endOffset <= regionTag.getStartOffset() || regionTag.getEndOffset() <= startOffset)
                // no intersection
                continue;
            else if (startOffset <= regionTag.getStartOffset() && regionTag.getEndOffset() <= endOffset)
                // totally covers region
                f(regionTag.getOffset(), regionTag.getLength(), getTagForUpdate(i));
            else if (regionTag.getStartOffset() < startOffset && endOffset < regionTag.getEndOffset())
                // splits region into two parts
                f(startOffset, endOffset - startOffset, getTagForUpdate(i));
            else if (regionTag.getEndOffset() <= endOffset)
                // cuts end of region
                f(startOffset, regionTag.getEndOffset() - startOffset, getTagForUpdate(i));
            else if (startOffset <= regionTag.getStartOffset())
                // cuts beginning of region
                f(regionTag.getStartOffset(), endOffset - regionTag.getStartOffset(), getTagForUpdate(i));
            else
                ASSERT(false);
        }
    }
}

void RegionTagSet::clearTags(b offset, b length)
{
    if (regionTags != nullptr) {
        bool changed = false;
        b clearStartOffset = offset;
        b clearEndOffset = offset + length;
        for (int i = 0; i < (int)regionTags->size(); i++) {
            auto& regionTag = (*regionTags)[i];
            if (clearEndOffset <= regionTag.getStartOffset() || regionTag.getEndOffset() <= clearStartOffset)
                // no intersection
                continue;
            else if (clearStartOffset <= regionTag.getStartOffset() && regionTag.getEndOffset() <= clearEndOffset) {
                // clear totally covers region
                prepareTagsVectorForUpdate();
                regionTags->erase(regionTags->begin() + i--);
                changed = true;
            }
            else if (regionTag.getStartOffset() < clearStartOffset && clearEndOffset < regionTag.getEndOffset()) {
                // clear splits region into two parts
                prepareTagsVectorForUpdate();
                RegionTag<TagBase> previousRegion(regionTag.getStartOffset(), clearStartOffset - regionTag.getStartOffset(), regionTag.getTag());
                regionTags->insert(regionTags->begin() + i++, previousRegion);
                regionTag.setLength(regionTag.getEndOffset() - clearEndOffset);
                regionTag.setOffset(clearEndOffset);
                changed = true;
            }
            else if (regionTag.getEndOffset() <= clearEndOffset) {
                // clear cuts end of region
                prepareTagsVectorForUpdate();
                regionTag.setLength(clearStartOffset - regionTag.getStartOffset());
                changed = true;
            }
            else if (clearStartOffset <= regionTag.getStartOffset()) {
                // clear cuts beginning of region
                prepareTagsVectorForUpdate();
                regionTag.setLength(regionTag.getEndOffset() - clearEndOffset);
                regionTag.setOffset(clearEndOffset);
                changed = true;
            }
            else
                ASSERT(false);
        }
        if (changed)
            sortTagsVector();
    }
}

void RegionTagSet::copyTags(const RegionTagSet& source, b sourceOffset, b offset, b length)
{
    auto shift = offset - sourceOffset;
    clearTags(offset, length);
    source.mapAllTags(sourceOffset, length, [&] (b o, b l, const Ptr<const TagBase>& tag) {
        b startOffset = o < sourceOffset ? sourceOffset : o;
        b endOffset = o + l > sourceOffset + length ? sourceOffset + length : o + l;
        addTag(startOffset + shift, endOffset - startOffset, tag);
    });
}

void RegionTagSet::moveTags(b shift)
{
    if (regionTags != nullptr)
        for (auto& regionTag : *regionTags)
            regionTag.setOffset(regionTag.getOffset() + shift);
}

void RegionTagSet::splitTags(const std::type_info& typeInfo, b offset)
{
    if (regionTags != nullptr) {
        std::vector<RegionTag<TagBase>> insertedRegionTags;
        for (auto& regionTag : *regionTags) {
            auto tag = regionTag.getTag();
            auto tagObject = tag.get();
            if (typeInfo == typeid(*tagObject) && regionTag.getStartOffset() < offset && offset < regionTag.getEndOffset()) {
                insertedRegionTags.push_back(RegionTag<TagBase>(offset, regionTag.getEndOffset() - offset, tag));
                regionTag.setLength(offset - regionTag.getStartOffset());
            }
        }
        if (!insertedRegionTags.empty()) {
            prepareTagsVectorForUpdate();
            for (auto regionTag : insertedRegionTags)
                regionTags->push_back(RegionTag<TagBase>(regionTag.getOffset(), regionTag.getLength(), regionTag.getTag()));
            sortTagsVector();
        }
    }
}

int RegionTagSet::getTagIndex(const std::type_info& typeInfo, b offset, b length) const
{
    if (regionTags == nullptr)
        return -1;
    else {
        int numRegionTags = regionTags->size();
        b getStartOffset = offset;
        b getEndOffset = offset + length;
        for (int index = 0; index < numRegionTags; index++) {
            auto& regionTag = (*regionTags)[index];
            const auto& tag = regionTag.getTag();
            auto tagObject = tag.get();
            if (typeInfo != typeid(*tagObject) || getEndOffset <= regionTag.getStartOffset() || regionTag.getEndOffset() <= getStartOffset)
                continue;
            else if (regionTag.getOffset() == offset && regionTag.getLength() == length)
                return index;
            else
                throw cRuntimeError("Overlapping tag is present");
        }
        return -1;
    }
}

void RegionTagSet::ensureTagsVectorAllocated()
{
    if (regionTags == nullptr) {
        regionTags = makeShared<SharedVector<RegionTag<TagBase>>>();
        regionTags->reserve(16);
    }
}

void RegionTagSet::prepareTagsVectorForUpdate()
{
    if (regionTags.use_count() != 1) {
        const auto& newRegionTags = makeShared<SharedVector<RegionTag<TagBase>>>();
        newRegionTags->insert(newRegionTags->begin(), regionTags->begin(), regionTags->end());
        regionTags = newRegionTags;
    }
}

} // namespace

