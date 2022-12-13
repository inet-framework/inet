//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/tag/SharingRegionTagSet.h"

namespace inet {

#ifdef INET_WITH_SELFDOC
void SharingRegionTagSet::selfDoc(const char * tagAction, const char *typeName)
{
    auto contextModuleTypeName = cSimulation::getActiveSimulation()->getContextModule()->getComponentType()->getFullName();
    if (SelfDoc::generateSelfdoc) {
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", contextModuleTypeName)
               << ", " << SelfDoc::keyVal("action", "RTAG")
               << ", \"details\" : { "
               << SelfDoc::keyVal("tagAction", tagAction)
               << ", " << SelfDoc::keyVal("tagType", typeName)
               << " } }";
            globalSelfDoc.insert(os.str());
        }
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("class", typeName)
               << ", " << SelfDoc::keyVal("action", "RTAG_USAGE")
               << ", \"details\" : { "
               << SelfDoc::keyVal("tagAction", tagAction)
               << ", " << SelfDoc::keyVal("module", contextModuleTypeName)
               << " } }";
            globalSelfDoc.insert(os.str());
        }
    }
}
#endif // INET_WITH_SELFDOC

void SharingRegionTagSet::parsimPack(cCommBuffer *buffer) const
{
    buffer->pack(getNumTags());
    if (regionTags != nullptr) {
        for (auto regionTag : *regionTags) {
            buffer->pack(regionTag.getOffset().get());
            buffer->pack(regionTag.getLength().get());
            buffer->packObject(const_cast<TagBase *>(regionTag.getTag().get()));
        }
    }
}

void SharingRegionTagSet::parsimUnpack(cCommBuffer *buffer)
{
    if (regionTags != nullptr)
        regionTags->clear();
    int size;
    buffer->unpack(size);
    for (int i = 0; i < size; i++) {
        uint64_t offset;
        buffer->unpack(offset);
        uint64_t length;
        buffer->unpack(length);
        auto tag = check_and_cast<TagBase *>(buffer->unpackObject());
        addTag(b(offset), b(length), tag->shared_from_this());
    }
}

void SharingRegionTagSet::addTag(b offset, b length, const Ptr<const TagBase>& tag)
{
    ensureTagsVectorAllocated();
    prepareTagsVectorForUpdate();
    regionTags->push_back(RegionTag<TagBase>(offset, length, tag));
}

std::vector<SharingRegionTagSet::RegionTag<TagBase>> SharingRegionTagSet::addTagsWhereAbsent(const std::type_info& typeInfo, b offset, b length, const Ptr<const TagBase>& tag)
{
    std::vector<SharingRegionTagSet::RegionTag<TagBase>> result;
    b endOffset = offset + length;
    b o = offset;
    if (regionTags != nullptr) {
        for (auto& regionTag : *regionTags) {
            const auto& existingTag = regionTag.getTag();
            auto tagObject = existingTag.get();
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

const Ptr<TagBase> SharingRegionTagSet::removeTag(int index)
{
    prepareTagsVectorForUpdate();
    const Ptr<TagBase> tag = getTagForUpdate(index);
    regionTags->erase(regionTags->begin() + index);
    if (regionTags->size() == 0)
        regionTags = nullptr;
    return constPtrCast<TagBase>(tag);
}

void SharingRegionTagSet::mapAllTags(b offset, b length, std::function<void(b, b, const Ptr<const TagBase>&)> f) const
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

void SharingRegionTagSet::mapAllTagsForUpdate(b offset, b length, std::function<void(b, b, const Ptr<TagBase>&)> f)
{
    if (regionTags != nullptr) {
        prepareTagsVectorForUpdate();
        b startOffset = offset;
        b endOffset = offset + length;
        for (int i = 0; i < (int)regionTags->size(); i++) {
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

void SharingRegionTagSet::clearTags(b offset, b length)
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
                auto& regionTag = (*regionTags)[i];
                auto startOffset = regionTag.getStartOffset();
                regionTag.setLength(regionTag.getEndOffset() - clearEndOffset);
                regionTag.setOffset(clearEndOffset);
                RegionTag<TagBase> previousRegionTag(regionTag);
                previousRegionTag.setOffset(startOffset);
                previousRegionTag.setLength(clearStartOffset - startOffset);
                regionTags->insert(regionTags->begin() + i++, previousRegionTag);
                changed = true;
            }
            else if (regionTag.getEndOffset() <= clearEndOffset) {
                // clear cuts end of region
                prepareTagsVectorForUpdate();
                auto& regionTag = (*regionTags)[i];
                regionTag.setLength(clearStartOffset - regionTag.getStartOffset());
                changed = true;
            }
            else if (clearStartOffset <= regionTag.getStartOffset()) {
                // clear cuts beginning of region
                prepareTagsVectorForUpdate();
                auto& regionTag = (*regionTags)[i];
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

void SharingRegionTagSet::copyTags(const SharingRegionTagSet& source, b sourceOffset, b offset, b length)
{
    auto shift = offset - sourceOffset;
    clearTags(offset, length);
    source.mapAllTags(sourceOffset, length, [&] (b o, b l, const Ptr<const TagBase>& tag) {
        b startOffset = o < sourceOffset ? sourceOffset : o;
        b endOffset = o + l > sourceOffset + length ? sourceOffset + length : o + l;
        addTag(startOffset + shift, endOffset - startOffset, tag);
    });
}

void SharingRegionTagSet::moveTags(b shift)
{
    if (regionTags != nullptr) {
        prepareTagsVectorForUpdate();
        for (auto& regionTag : *regionTags)
            regionTag.setOffset(regionTag.getOffset() + shift);
    }
}

void SharingRegionTagSet::moveTags(b offset, b length, b shift)
{
    if (regionTags != nullptr) {
        prepareTagsVectorForUpdate();
        if (shift > b(0)) {
            splitTags(offset);
            clearTags(offset + length, offset + length + shift);
        }
        else {
            splitTags(offset + length);
            clearTags(offset - shift, offset);
        }
        for (auto& regionTag : *regionTags)
            if (offset <= regionTag.getStartOffset() && regionTag.getEndOffset() <= offset + length)
                regionTag.setOffset(regionTag.getOffset() + shift);
    }
}

void SharingRegionTagSet::splitTags(b offset, std::function<bool(const TagBase *)> f)
{
    if (regionTags != nullptr) {
        std::vector<RegionTag<TagBase>> insertedRegionTags;
        for (auto& regionTag : *regionTags) {
            auto tag = regionTag.getTag();
            auto tagObject = tag.get();
            if (f(tagObject) && regionTag.getStartOffset() < offset && offset < regionTag.getEndOffset()) {
                prepareTagsVectorForUpdate();
                RegionTag<TagBase> otherRegionTag(regionTag);
                otherRegionTag.setOffset(offset);
                otherRegionTag.setLength(regionTag.getEndOffset() - offset);
                insertedRegionTags.push_back(otherRegionTag);
                regionTag.setLength(offset - regionTag.getStartOffset());
            }
        }
        if (!insertedRegionTags.empty()) {
            for (auto regionTag : insertedRegionTags)
                regionTags->push_back(regionTag);
            sortTagsVector();
        }
    }
}

int SharingRegionTagSet::getTagIndex(const std::type_info& typeInfo, b offset, b length) const
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

void SharingRegionTagSet::ensureTagsVectorAllocated()
{
    if (regionTags == nullptr) {
        regionTags = makeShared<SharedVector<RegionTag<TagBase>>>();
        regionTags->reserve(16);
    }
}

void SharingRegionTagSet::prepareTagsVectorForUpdate()
{
    if (regionTags.use_count() != 1) {
        const auto& newRegionTags = makeShared<SharedVector<RegionTag<TagBase>>>();
        newRegionTags->insert(newRegionTags->begin(), regionTags->begin(), regionTags->end());
        regionTags = newRegionTags;
    }
}

} // namespace

