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

#ifndef __INET_TAGSET_H_
#define __INET_TAGSET_H_

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class maintains a set of tags. Tags are usually small data strcutures
 * that hold some relevant information. Tags are identified by their type,
 * which means that this class supports adding the same tag type only once.
 * Added tags are exclusively owned by this class and they get deleted with it.
 */
class INET_API TagSet : public cObject
{
  protected:
    std::vector<cObject *> *tags;

  protected:
    void ensureAllocated();

    void addTag(cObject *tag);
    cObject *removeTag(int index);

    int getTagIndex(const std::type_info& typeInfo) const;
    template <typename T> int getTagIndex() const;

  public:
    TagSet();
    TagSet(const TagSet& other);
    TagSet(TagSet&& other);
    ~TagSet();

    TagSet& operator=(const TagSet& other);
    TagSet& operator=(TagSet&& other);

    /**
     * Returns the number of tags.
     */
    inline int getNumTags() const;

    /**
     * Returns the tag at the given index. The index must be in the range [0, getNumTags()).
     */
    inline cObject *getTag(int index) const;

    /**
     * Clears the set of tags.
     */
    void clearTags();

    /**
     * Copies the set of tags from the other set.
     */
    void copyTags(const TagSet& other);

    /**
     * Returns the tag for the provided type, or returns nullptr if no such tag is present.
     */
    template <typename T> T *findTag() const;

    /**
     * Returns the tag for the provided type, or throws an exception if no such tag is present.
     */
    template <typename T> T *getTag() const;

    /**
     * Returns a newly added tag for the provided type, or throws an exception if such a tag is already present.
     */
    template <typename T> T *addTag();

    /**
     * Returns a newly added tag for the provided type if absent, or returns the tag that is already present.
     */
    template <typename T> T *addTagIfAbsent();

    /**
     * Removes the tag for the provided type, or throws an exception if no such tag is present.
     */
    template <typename T> T *removeTag();

    /**
     * Removes the tag for the provided type if present, or returns nullptr if no such tag is present.
     */
    template <typename T> T *removeTagIfPresent();
};

inline int TagSet::getNumTags() const
{
    return tags == nullptr ? 0 : tags->size();
}

inline cObject *TagSet::getTag(int index) const
{
    return tags->at(index);
}

template <typename T>
inline int TagSet::getTagIndex() const
{
    return getTagIndex(typeid(T));
}

template <typename T>
inline T *TagSet::findTag() const
{
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : static_cast<T *>((*tags)[index]);
}

template <typename T>
inline T *TagSet::getTag() const
{
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return static_cast<T *>((*tags)[index]);
}

template <typename T>
inline T *TagSet::addTag()
{
    int index = getTagIndex<T>();
    if (index != -1)
        throw cRuntimeError("Tag '%s' is present", opp_typename(typeid(T)));
    T *tag = new T();
    addTag(tag);
    return tag;
}

template <typename T>
inline T *TagSet::addTagIfAbsent()
{
    T *tag = findTag<T>();
    if (tag == nullptr)
        addTag(tag = new T());
    return tag;
}

template <typename T>
inline T *TagSet::removeTag()
{
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return static_cast<T *>(removeTag(index));
}

template <typename T>
inline T *TagSet::removeTagIfPresent()
{
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : static_cast<T *>(removeTag(index));
}

} // namespace

#endif // #ifndef __INET_TAGSET_H_

