//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/tag/SharingTagSet.h"

namespace inet {

#ifdef INET_WITH_SELFDOC
void SharingTagSet::selfDoc(const char * tagAction, const char *typeName)
{
    if (SelfDoc::generateSelfdoc) {
        auto contextModuleTypeName = cSimulation::getActiveSimulation()->getContextModule()->getComponentType()->getFullName();
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", contextModuleTypeName)
               << ", " << SelfDoc::keyVal("action", "STAG")
               << ", \"details\" : { "
               << SelfDoc::keyVal("tagAction", tagAction)
               << ", " << SelfDoc::keyVal("tagType", typeName)
               << " } }";
            globalSelfDoc.insert(os.str());
        }
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("class", typeName)
               << ", " << SelfDoc::keyVal("action", "STAG_USAGE")
               << ", \"details\" : { "
               << SelfDoc::keyVal("tagAction", tagAction)
               << ", " << SelfDoc::keyVal("module", contextModuleTypeName)
               << " } }";
            globalSelfDoc.insert(os.str());
        }
    }
}
#endif // INET_WITH_SELFDOC

void SharingTagSet::addTag(const Ptr<const TagBase>& tag)
{
    ensureTagsVectorAllocated();
    prepareTagsVectorForUpdate();
    tags->push_back(tag);
}

const Ptr<TagBase> SharingTagSet::removeTag(int index)
{
    prepareTagsVectorForUpdate();
    const Ptr<TagBase> tag = getTagForUpdate(index);
    tags->erase(tags->begin() + index);
    if (tags->size() == 0)
        tags = nullptr;
    return constPtrCast<TagBase>(tag);
}

int SharingTagSet::getTagIndex(const std::type_info& typeInfo) const
{
    if (tags == nullptr)
        return -1;
    else {
        int numTags = tags->size();
        for (int index = 0; index < numTags; index++) {
            const Ptr<const TagBase>& tag = (*tags)[index];
            const TagBase *tagObject = tag.get();
            if (typeInfo == typeid(*tagObject))
                return index;
        }
        return -1;
    }
}

void SharingTagSet::ensureTagsVectorAllocated()
{
    if (tags == nullptr) {
        tags = makeShared<SharedVector<Ptr<const TagBase>>>();
        tags->reserve(16);
    }
}

void SharingTagSet::prepareTagsVectorForUpdate()
{
    if (tags.use_count() != 1) {
        const auto& newTags = makeShared<SharedVector<Ptr<const TagBase>>>();
        newTags->insert(newTags->begin(), tags->begin(), tags->end());
        tags = newTags;
    }
}

void SharingTagSet::parsimPack(cCommBuffer *buffer) const
{
    buffer->pack(getNumTags());
    if (tags != nullptr)
        for (auto tag : *tags)
            buffer->packObject(const_cast<TagBase *>(tag.get()));
}

void SharingTagSet::parsimUnpack(cCommBuffer *buffer)
{
    clearTags();
    int size;
    buffer->unpack(size);
    for (int i = 0; i < size; i++) {
        auto tag = check_and_cast<TagBase *>(buffer->unpackObject());
        addTag(tag->shared_from_this());
    }
}

} // namespace

