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

#include "inet/common/packet/tag/TagSet.h"

namespace inet {

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

