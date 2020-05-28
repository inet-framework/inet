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

void TagSet::addTag(const Ptr<const TagBase>& tag)
{
    ensureTagsVectorAllocated();
    prepareTagsVectorForUpdate();
    tags->push_back(tag);
}

const Ptr<TagBase> TagSet::removeTag(int index)
{
    prepareTagsVectorForUpdate();
    const Ptr<TagBase> tag = getTagForUpdate(index);
    tags->erase(tags->begin() + index);
    if (tags->size() == 0)
        tags = nullptr;
    return constPtrCast<TagBase>(tag);
}

int TagSet::getTagIndex(const std::type_info& typeInfo) const
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

void TagSet::ensureTagsVectorAllocated()
{
    if (tags == nullptr) {
        tags = makeShared<SharedVector<Ptr<const TagBase>>>();
        tags->reserve(16);
    }
}

void TagSet::prepareTagsVectorForUpdate()
{
    if (tags.use_count() != 1) {
        const auto& newTags = makeShared<SharedVector<Ptr<const TagBase>>>();
        newTags->insert(newTags->begin(), tags->begin(), tags->end());
        tags = newTags;
    }
}

} // namespace

