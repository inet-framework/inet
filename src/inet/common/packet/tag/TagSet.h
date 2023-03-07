//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TAGSET_H
#define __INET_TAGSET_H

#include "inet/common/INETDefs.h"

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
    template<typename T> int getTagIndex() const;

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
    template<typename T> const T *findTag() const;

    /**
     * Returns the tag of the provided type for update, or returns nullptr if no such tag is present.
     */
    template<typename T> T *findTagForUpdate();

    /**
     * Returns the tag for the provided type, or throws an exception if no such tag is present.
     */
    template<typename T> const T *getTag() const;

    /**
     * Returns the tag of the provided type for update, or throws an exception if no such tag is present.
     */
    template<typename T> T *getTagForUpdate();

    /**
     * Returns a newly added tag for the provided type, or throws an exception if such a tag is already present.
     */
    template<typename T> T *addTag();

    /**
     * Returns a newly added tag for the provided type if absent, or returns the tag that is already present.
     */
    template<typename T> T *addTagIfAbsent();

    /**
     * Removes the tag for the provided type, or throws an exception if no such tag is present.
     */
    template<typename T> T *removeTag();

    /**
     * Removes the tag for the provided type if present, or returns nullptr if no such tag is present.
     */
    template<typename T> T *removeTagIfPresent();

#ifdef INET_WITH_SELFDOC
    static void selfDoc(const char * tagAction, const char *typeName);
#endif
};

inline int TagSet::getNumTags() const
{
    return tags == nullptr ? 0 : tags->size();
}

inline cObject *TagSet::getTag(int index) const
{
    return tags->at(index);
}

template<typename T>
inline int TagSet::getTagIndex() const
{
    return getTagIndex(typeid(T));
}

template<typename T>
inline const T *TagSet::findTag() const
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : static_cast<T *>((*tags)[index]);
}

template<typename T>
inline T *TagSet::findTagForUpdate()
{
    SELFDOC_FUNCTION_T;
    return const_cast<T *>(findTag<T>());
}

template<typename T>
inline const T *TagSet::getTag() const
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return static_cast<T *>((*tags)[index]);
}

template<typename T>
inline T *TagSet::getTagForUpdate()
{
    SELFDOC_FUNCTION_T;
    return const_cast<T *>(getTag<T>());
}

template<typename T>
inline T *TagSet::addTag()
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    if (index != -1)
        throw cRuntimeError("Tag '%s' is present", opp_typename(typeid(T)));
    T *tag = new T();
    addTag(tag);
    return tag;
}

template<typename T>
inline T *TagSet::addTagIfAbsent()
{
    SELFDOC_FUNCTION_T;
    T *tag = findTagForUpdate<T>();
    if (tag == nullptr)
        addTag(tag = new T());
    return tag;
}

template<typename T>
inline T *TagSet::removeTag()
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return static_cast<T *>(removeTag(index));
}

template<typename T>
inline T *TagSet::removeTagIfPresent()
{
    SELFDOC_FUNCTION_T;
    int index = getTagIndex<T>();
    return index == -1 ? nullptr : static_cast<T *>(removeTag(index));
}

#undef SELFDOC_FUNCTION_T

} // namespace

#endif

