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

#ifndef __INET_SHARINGTAGSET_H_
#define __INET_SHARINGTAGSET_H_

#include <vector>
#include <memory>
#include "inet/common/Ptr.h"
#include "inet/common/TagBase.h"

namespace inet {

/**
 * This class maintains a set of tags. Tags are usually small data strcutures
 * that hold some relevant information. Tags are identified by their type,
 * which means that this class supports adding the same tag type only once.
 * Tags are shared between other instances of this class. Tags can be changed
 * with a copy-on-write mechanism.
 */
class INET_API SharingTagSet : public cObject
{
  protected:
    /**
     * Both the vector and the individual tags can be shared among different instances of this class.
     */
    Ptr<SharedVector<Ptr<const TagBase>>> tags;

  protected:
    void setTag(int index, const Ptr<const TagBase>& tag);
    void addTag(const Ptr<const TagBase>& tag);
    const Ptr<TagBase> removeTag(int index);

    int getTagIndex(const std::type_info& typeInfo) const;
    template <typename T> int getTagIndex() const;

    void ensureTagsVectorAllocated();
    void prepareTagsVectorForUpdate();

  public:
    /** @name Constructors and operators */
    //@{
    SharingTagSet() : tags(nullptr) { }
    SharingTagSet(const SharingTagSet& other) : tags(other.tags) { }
    SharingTagSet(SharingTagSet&& other) : tags(other.tags) { other.tags = nullptr; }

    SharingTagSet& operator=(const SharingTagSet& other);
    SharingTagSet& operator=(SharingTagSet&& other);

    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Type independent functions */
    //@{
    /**
     * Returns the number of tags.
     */
    int getNumTags() const;

    /**
     * Returns the shared tag at the given index. The index must be in the range [0, getNumTags()).
     */
    const Ptr<const TagBase>& getTag(int index) const;

    /**
     * Returns the exclusively owned tag at the given index for update. The index must be in the range [0, getNumTags()).
     */
    const Ptr<TagBase> getTagForUpdate(int index);

    /**
     * Clears the set of tags.
     */
    void clearTags();

    /**
     * Copies the set of tags from the other set.
     */
    void copyTags(const SharingTagSet& other);
    //@}

    /** @name Type dependent functions */
    //@{
    /**
     * Returns the shared tag of the provided type, or returns nullptr if no such tag is present.
     */
    template <typename T> const Ptr<const T> findTag() const;

    /**
     * Returns the exclusively owned tag of the provided type for update, or returns nullptr if no such tag is present.
     */
    template <typename T> const Ptr<T> findTagForUpdate();

    /**
     * Returns the shared tag of the provided type, or throws an exception if no such tag is present.
     */
    template <typename T> const Ptr<const T> getTag() const;

    /**
     * Returns the exclusively owned tag of the provided type for update, or throws an exception if no such tag is present.
     */
    template <typename T> const Ptr<T> getTagForUpdate();

    /**
     * Returns a newly added exclusively owned tag of the provided type for update, or throws an exception if such a tag is already present.
     */
    template <typename T> const Ptr<T> addTag();

    /**
     * Returns a newly added exclusively owned tag of the provided type if absent, or returns the exclusively owned tag that is already present for update.
     */
    template <typename T> const Ptr<T> addTagIfAbsent();

    /**
     * Removes the tag of the provided type and returns it for update, or throws an exception if no such tag is present.
     */
    template <typename T> const Ptr<T> removeTag();

    /**
     * Removes the tag of the provided type if present and returns it for update, or returns nullptr if no such tag is present.
     */
    template <typename T> const Ptr<T> removeTagIfPresent();
    //@}
};

inline SharingTagSet& SharingTagSet::operator=(const SharingTagSet& other)
{
    if (this != &other)
        tags = other.tags;
    return *this;
}

inline SharingTagSet& SharingTagSet::operator=(SharingTagSet&& other)
{
    if (this != &other) {
        tags = other.tags;
        other.tags = nullptr;
    }
    return *this;
}

inline int SharingTagSet::getNumTags() const
{
    return tags == nullptr ? 0 : tags->size();
}

inline void SharingTagSet::setTag(int index, const Ptr<const TagBase>& tag)
{
    (*tags)[index] = tag;
}

inline const Ptr<const TagBase>& SharingTagSet::getTag(int index) const
{
    return (*tags)[index];
}

inline const Ptr<TagBase> SharingTagSet::getTagForUpdate(int index)
{
    const Ptr<const TagBase>& tag = getTag(index);
    prepareTagsVectorForUpdate();
    if (tag.use_count() != 1)
        setTag(index, tag->dupShared());
    return constPtrCast<TagBase>(getTag(index));
}

inline void SharingTagSet::clearTags()
{
    tags = nullptr;
}

inline void SharingTagSet::copyTags(const SharingTagSet& source)
{
    operator=(source);
}

template <typename T>
inline int SharingTagSet::getTagIndex() const
{
    return getTagIndex(typeid(T));
}

template <typename T>
inline const Ptr<const T> SharingTagSet::findTag() const
{
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : staticPtrCast<const T>(getTag(index));
}

template <typename T>
inline const Ptr<T> SharingTagSet::findTagForUpdate()
{
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : staticPtrCast<T>(getTagForUpdate(index));
}

template <typename T>
inline const Ptr<const T> SharingTagSet::getTag() const
{
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<const T>(getTag(index));
}

template <typename T>
inline const Ptr<T> SharingTagSet::getTagForUpdate()
{
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<T>(getTagForUpdate(index));
}

template <typename T>
inline const Ptr<T> SharingTagSet::addTag()
{
    int index = getTagIndex<T>();
    if (index != -1)
        throw cRuntimeError("Tag '%s' is present", opp_typename(typeid(T)));
    const Ptr<T>& tag = makeShared<T>();
    addTag(tag);
    return tag;
}

template <typename T>
inline const Ptr<T> SharingTagSet::addTagIfAbsent()
{
    const Ptr<T>& tag = findTagForUpdate<T>();
    if (tag != nullptr)
        return tag;
    else {
        const Ptr<T>& tag = makeShared<T>();
        addTag(tag);
        return tag;
    }
}

template <typename T>
inline const Ptr<T> SharingTagSet::removeTag()
{
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<T>(removeTag(index));
}

template <typename T>
inline const Ptr<T> SharingTagSet::removeTagIfPresent()
{
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : staticPtrCast<T>(removeTag(index));
}

} // namespace

#endif // #ifndef __INET_SHARINGTAGSET_H_

