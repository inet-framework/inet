//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/tag/TagSet.h"

namespace inet {

#ifdef INET_WITH_SELFDOC
void TagSet::selfDoc(const char * tagAction, const char *typeName)
{
    if (SelfDoc::generateSelfdoc) {
        auto contextModuleTypeName = cSimulation::getActiveSimulation()->getContextModule()->getComponentType()->getFullName();
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", contextModuleTypeName)
               << ", " << SelfDoc::keyVal("action", "TAG")
               << ", \"details\" : { "
               << SelfDoc::keyVal("tagAction", tagAction)
               << ", " << SelfDoc::keyVal("tagType", typeName)
               << " } }";
            globalSelfDoc.insert(os.str());
        }
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("class", typeName)
               << ", " << SelfDoc::keyVal("action", "TAG_USAGE")
               << ", \"details\" : { "
               << SelfDoc::keyVal("tagAction", tagAction)
               << ", " << SelfDoc::keyVal("module", contextModuleTypeName)
               << " } }";
            globalSelfDoc.insert(os.str());
        }
    }
}
#endif // INET_WITH_SELFDOC

TagSet::TagSet() :
    tags(nullptr)
{
}

TagSet::TagSet(const TagSet& other) :
    tags(nullptr)
{
    operator=(other);
}

TagSet::TagSet(TagSet&& other) :
    tags(other.tags)
{
    other.tags = nullptr;
}

TagSet::~TagSet()
{
    clearTags();
}

TagSet& TagSet::operator=(const TagSet& other)
{
    if (this != &other) {
        clearTags();
        if (other.tags == nullptr)
            tags = nullptr;
        else {
            ensureAllocated();
            for (auto tag : *other.tags)
                addTag(tag->dup());
        }
    }
    return *this;
}

TagSet& TagSet::operator=(TagSet&& other)
{
    if (this != &other) {
        clearTags();
        tags = other.tags;
        other.tags = nullptr;
    }
    return *this;
}

void TagSet::ensureAllocated()
{
    if (tags == nullptr) {
        tags = new std::vector<cObject *>();
        tags->reserve(16);
    }
}

void TagSet::addTag(cObject *tag)
{
    ensureAllocated();
    tags->push_back(tag);
    if (tag->isOwnedObject())
        take(static_cast<cOwnedObject *>(tag));
}

cObject *TagSet::removeTag(int index)
{
    cObject *tag = (*tags)[index];
    if (tag->isOwnedObject())
        drop(static_cast<cOwnedObject *>(tag));
    tags->erase(tags->begin() + index);
    if (tags->size() == 0) {
        delete tags;
        tags = nullptr;
    }
    return tag;
}

void TagSet::clearTags()
{
#ifdef INET_WITH_SELFDOC
    selfDoc(__FUNCTION__, "");
    SelfDocTempOff;
#endif // INET_WITH_SELFDOC
    if (tags != nullptr) {
        int numTags = tags->size();
        for (int index = 0; index < numTags; index++) {
            cObject *tag = (*tags)[index];
            if (tag->isOwnedObject())
                drop(static_cast<cOwnedObject *>(tag));
            delete tag;
        }
        delete tags;
        tags = nullptr;
    }
}

void TagSet::copyTags(const TagSet& source)
{
    clearTags();
    if (source.tags != nullptr) {
        int numTags = source.tags->size();
        tags = new std::vector<cObject *>(numTags);
        for (int index = 0; index < numTags; index++) {
            cObject *tag = (*source.tags)[index]->dup();
            if (tag->isOwnedObject())
                take(static_cast<cOwnedObject *>(tag));
            (*tags)[index] = tag;
        }
    }
}

int TagSet::getTagIndex(const std::type_info& typeInfo) const
{
    if (tags == nullptr)
        return -1;
    else {
        int numTags = tags->size();
        for (int index = 0; index < numTags; index++) {
            cObject *tag = (*tags)[index];
            if (typeInfo == typeid(*tag))
                return index;
        }
        return -1;
    }
}

} // namespace

