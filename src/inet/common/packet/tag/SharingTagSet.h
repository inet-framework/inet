//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SHARINGTAGSET_H
#define __INET_SHARINGTAGSET_H

#include <memory>
#include <vector>

#include "inet/common/Ptr.h"
#include "inet/common/TagBase.h"

namespace inet {

#ifdef INET_WITH_SELFDOC
#define SELFDOC_FUNCTION_T  \
        selfDoc(__FUNCTION__, opp_typename(typeid(T))); \
        SelfDocTempOff;
#else
#define SELFDOC_FUNCTION_T
#endif

/**
 * This class maintains a set of tags. Tags are usually small data structures
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
    inline void setTag(int index, const Ptr<const TagBase>& tag);
    void addTag(const Ptr<const TagBase>& tag);
    const Ptr<TagBase> removeTag(int index);

    int getTagIndex(const std::type_info& typeInfo) const;
    template<typename T> int getTagIndex() const;

    void ensureTagsVectorAllocated();
    void prepareTagsVectorForUpdate();

  public:
    /** @name Constructors and operators */
    //@{
    SharingTagSet() : tags(nullptr) {}
    SharingTagSet(const SharingTagSet& other) : tags(other.tags) {}
    SharingTagSet(SharingTagSet&& other) : tags(other.tags) { other.tags = nullptr; }

    inline SharingTagSet& operator=(const SharingTagSet& other);
    inline SharingTagSet& operator=(SharingTagSet&& other);

    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Type independent functions */
    //@{
    /**
     * Returns the number of tags.
     */
    inline int getNumTags() const;

    /**
     * Returns the shared tag at the given index. The index must be in the range [0, getNumTags()).
     */
    inline const Ptr<const TagBase>& getTag(int index) const;

    /**
     * Returns the exclusively owned tag at the given index for update. The index must be in the range [0, getNumTags()).
     */
    inline const Ptr<TagBase> getTagForUpdate(int index);

    /**
     * Clears the set of tags.
     */
    inline void clearTags();

    /**
     * Copies the set of tags from the other set.
     */
    inline void copyTags(const SharingTagSet& other);
    //@}

    /** @name Type dependent functions */
    //@{
    /**
     * Returns true if the tag with provided type is present.
     */
    template<typename T> bool hasTag() const;

    /**
     * Returns the shared tag of the provided type, or returns nullptr if no such tag is present.
     */
    template<typename T> const Ptr<const T> findTag() const;

    /**
     * Returns the exclusively owned tag of the provided type for update, or returns nullptr if no such tag is present.
     */
    template<typename T> const Ptr<T> findTagForUpdate();

    /**
     * Returns the shared tag of the provided type, or throws an exception if no such tag is present.
     */
    template<typename T> const Ptr<const T> getTag() const;

    /**
     * Returns the exclusively owned tag of the provided type for update, or throws an exception if no such tag is present.
     */
    template<typename T> const Ptr<T> getTagForUpdate();

    /**
     * Returns a newly added exclusively owned tag of the provided type for update, or throws an exception if such a tag is already present.
     */
    template<typename T> const Ptr<T> addTag();

    /**
     * Returns a newly added exclusively owned tag of the provided type if absent, or returns the exclusively owned tag that is already present for update.
     */
    template<typename T> const Ptr<T> addTagIfAbsent();

    /**
     * Removes the tag of the provided type and returns it for update, or throws an exception if no such tag is present.
     */
    template<typename T> const Ptr<T> removeTag();

    /**
     * Removes the tag of the provided type if present and returns it for update, or returns nullptr if no such tag is present.
     */
    template<typename T> const Ptr<T> removeTagIfPresent();
    //@}

#ifdef INET_WITH_SELFDOC
    static void selfDoc(const char * tagAction, const char *typeName);
#endif
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
#ifdef INET_WITH_SELFDOC
    selfDoc(__FUNCTION__, "");
#endif // INET_WITH_SELFDOC
    tags = nullptr;
}

inline void SharingTagSet::copyTags(const SharingTagSet& source)
{
    operator=(source);
}

template<typename T>
inline int SharingTagSet::getTagIndex() const
{
    return getTagIndex(typeid(T));
}

template<typename T>
inline bool SharingTagSet::hasTag() const
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    return index != -1;
}

template<typename T>
inline const Ptr<const T> SharingTagSet::findTag() const
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : staticPtrCast<const T>(getTag(index));
}

template<typename T>
inline const Ptr<T> SharingTagSet::findTagForUpdate()
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : staticPtrCast<T>(getTagForUpdate(index));
}

template<typename T>
inline const Ptr<const T> SharingTagSet::getTag() const
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<const T>(getTag(index));
}

template<typename T>
inline const Ptr<T> SharingTagSet::getTagForUpdate()
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<T>(getTagForUpdate(index));
}

template<typename T>
inline const Ptr<T> SharingTagSet::addTag()
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    if (index != -1)
        throw cRuntimeError("Tag '%s' is present", opp_typename(typeid(T)));
    const Ptr<T>& tag = makeShared<T>();
    addTag(tag);
    return tag;
}

template<typename T>
inline const Ptr<T> SharingTagSet::addTagIfAbsent()
{
    SELFDOC_FUNCTION_T;
    const Ptr<T>& tag = findTagForUpdate<T>();
    if (tag != nullptr)
        return tag;
    else {
        const Ptr<T>& tag = makeShared<T>();
        addTag(tag);
        return tag;
    }
}

template<typename T>
inline const Ptr<T> SharingTagSet::removeTag()
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<T>(removeTag(index));
}

template<typename T>
inline const Ptr<T> SharingTagSet::removeTagIfPresent()
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : staticPtrCast<T>(removeTag(index));
}

#undef SELFDOC_FUNCTION_T

} // namespace

#endif

