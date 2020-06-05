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

#ifndef __INET_SHARINGREGIONTAGSET_H_
#define __INET_SHARINGREGIONTAGSET_H_

#include <algorithm>
#include <functional>
#include <vector>
#include "inet/common/Ptr.h"
#include "inet/common/TagBase.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

/**
 * This class maintains a set of tags each referring to a specific region.
 * Regions are identified by their offset and length, and they are not allowed
 * to overlap. Tags are usually small data strcutures that hold some relevant
 * information. Tags are identified by their type, which means that this class
 * supports adding the same tag type for a specific region only once. Tags are
 * shared between other instances of this class. Tags can be changed with a
 * copy-on-write mechanism.
 */
class INET_API SharingRegionTagSet : public cObject
{
  public:
    /**
     * Region tags keep a tag for a specific range identified by offset and length.
     */
    template <typename T>
    class INET_API RegionTag : public cObject
    {
      friend class SharingRegionTagSet__TagBaseRegionTagDescriptor;

      protected:
        b offset;
        b length;
        Ptr<const T> tag;

      protected:
        const T *_getTag() const { return tag.get(); }

      public:
        RegionTag(b offset, b length, const Ptr<const T>& tag) : offset(offset), length(length), tag(tag) { }
        RegionTag(const RegionTag<T>& other) : offset(other.offset), length(other.length), tag(other.tag) { }
        RegionTag(RegionTag&& other) : offset(other.offset), length(other.length), tag(other.tag) { other.tag = nullptr; }

        RegionTag& operator=(const RegionTag& other) {
            if (this != &other) {
                offset = other.offset; length = other.length; tag = other.tag;
            }
            return *this;
        }
        RegionTag& operator=(RegionTag&& other) {
            if (this != &other) {
                offset = other.offset; length = other.length; tag = other.tag; other.tag = nullptr;
            }
            return *this;
        }

        bool operator<(const SharingRegionTagSet::RegionTag<T>& other) const { return offset < other.offset; }

        b getOffset() const { return offset; }
        void setOffset(b offset) { this->offset = offset; }

        b getLength() const { return length; }
        void setLength(b length) { this->length = length; }

        b getStartOffset() const { return offset; }
        b getEndOffset() const { return offset + length; }

        const Ptr<const T>& getTag() const { return tag; }
        void setTag(const Ptr<const T>& tag) { this->tag = tag; }

        virtual std::string str() const override {
            using std::operator<<; // KLUDGE: but whyyyyyyyy?
            std::stringstream stream;
            stream << "(" << getStartOffset() << ", " << getEndOffset() << ") " << (tag != nullptr ? tag->str() : "<nullptr>");
            return stream.str();
        }
    };

    typedef RegionTag<TagBase> TagBaseRegionTag;

  protected:
    Ptr<SharedVector<RegionTag<TagBase>>> regionTags;

  protected:
    void setTag(int index, const Ptr<const TagBase>& tag);
    void addTag(b offset, b length, const Ptr<const TagBase>& tag);
    std::vector<SharingRegionTagSet::RegionTag<TagBase>> addTagsWhereAbsent(const std::type_info& typeInfo, b offset, b length, const Ptr<const TagBase>& tag);
    const Ptr<TagBase> removeTag(int index);

    void mapAllTags(b offset, b length, std::function<void (b, b, const Ptr<const TagBase>&)> f) const;
    void mapAllTagsForUpdate(b offset, b length, std::function<void (b, b, const Ptr<TagBase>&)> f);

    void splitTags(const std::type_info& typeInfo, b offset);
    template <typename T> void splitTags(b offset);

    int getTagIndex(const std::type_info& typeInfo, b offset, b length) const;
    template <typename T> int getTagIndex(b offset, b length) const;

    void ensureTagsVectorAllocated();
    void prepareTagsVectorForUpdate();
    void sortTagsVector();

  public:
    /** @name Constructors and operators */
    //@{
    SharingRegionTagSet() : regionTags(nullptr) { }
    SharingRegionTagSet(const SharingRegionTagSet& other) : regionTags(other.regionTags) { }
    SharingRegionTagSet(SharingRegionTagSet&& other) : regionTags(other.regionTags) { other.regionTags = nullptr; }

    SharingRegionTagSet& operator=(const SharingRegionTagSet& other);
    SharingRegionTagSet& operator=(SharingRegionTagSet&& other);
    //@}

    /** @name Type independent functions */
    //@{
    /**
     * Returns the number of tags.
     */
    int getNumTags() const;

    /**
     * Returns the shared tag at the given index.
     */
    const Ptr<const TagBase>& getTag(int index) const;

    /**
     * Returns the exclusively owned tag at the given index for update.
     */
    const Ptr<TagBase> getTagForUpdate(int index);

    /**
     * Returns the shared region tag at the given index.
     */
    const RegionTag<TagBase>& getRegionTag(int index) const;

    /**
     * Returns the exclusively owned region tag at the given index.
     */
    const RegionTag<TagBase> getRegionTagForUpdate(int index);

    /**
     * Clears the set of tags in the given region.
     */
    void clearTags(b offset, b length);

    /**
     * Moves all tags with the provided shift.
     */
    void moveTags(b shift);

    /**
     * Copies the set of tags from the source region to the provided region.
     */
    void copyTags(const SharingRegionTagSet& source, b sourceOffset, b offset, b length);
    //@}

    /** @name Type dependent functions */
    //@{
    /**
     * Returns the shared tag of the provided type and range, or returns nullptr if no such tag is found.
     */
    template <typename T> const Ptr<const T> findTag(b offset, b length) const;

    /**
     * Returns the exclusively owned tag of the provided type and range for update, or returns nullptr if no such tag is found.
     */
    template <typename T> const Ptr<T> findTagForUpdate(b offset, b length);

    /**
     * Returns the shared tag of the provided type and range, or throws an exception if no such tag is found.
     */
    template <typename T> const Ptr<const T> getTag(b offset, b length) const;

    /**
     * Returns the exclusively owned tag of the provided type and range for update, or throws an exception if no such tag is found.
     */
    template <typename T> const Ptr<T> getTagForUpdate(b offset, b length);

    /**
     * Calls the given function with all the shared tags of the provided type and range.
     */
    template <typename T> void mapAllTags(b offset, b length, std::function<void (b, b, const Ptr<const T>&)> f) const;

    /**
     * Calls the given function with all the exclusively owned tags of the provided type and range for update.
     */
    template <typename T> void mapAllTagsForUpdate(b offset, b length, std::function<void (b, b, const Ptr<T>&)> f);

    /**
     * Returns all the shared tags of the provided type and range.
     */
    template <typename T> std::vector<RegionTag<T>> getAllTags(b offset, b length) const;

    /**
     * Returns all the exclusively owned tags of the provided type and range for update.
     */
    template <typename T> std::vector<RegionTag<T>> getAllTagsForUpdate(b offset, b length);

    /**
     * Returns a newly added exclusively owned tag of the provided type and range for update, or throws an exception if such a tag is already present.
     */
    template <typename T> const Ptr<T> addTag(b offset, b length);

    /**
     * Returns a newly added exclusively owned tag of the provided type and range if absent, or returns the tag that is already present for update.
     */
    template <typename T> const Ptr<T> addTagIfAbsent(b offset, b length);

    /**
     * Returns the newly added exclusively owned tags of the provided type and range for update where the tag is absent.
     */
    template <typename T> std::vector<RegionTag<T>> addTagsWhereAbsent(b offset, b length);

    /**
     * Removes the tag of the provided type and range and returns it for update, or throws an exception if no such tag is found.
     */
    template <typename T> const Ptr<T> removeTag(b offset, b length);

    /**
     * Removes the tag of the provided type and range if present and returns it for update, or returns nullptr if no such tag is found.
     */
    template <typename T> const Ptr<T> removeTagIfPresent(b offset, b length);

    /**
     * Removes and returns all tags of the provided type and range and returns them for update.
     */
    template <typename T> std::vector<RegionTag<T>> removeTagsWherePresent(b offset, b length);
    //@}
};

inline SharingRegionTagSet& SharingRegionTagSet::operator=(const SharingRegionTagSet& other)
{
    if (this != &other)
        regionTags = other.regionTags;
    return *this;
}

inline SharingRegionTagSet& SharingRegionTagSet::operator=(SharingRegionTagSet&& other)
{
    if (this != &other) {
        regionTags = other.regionTags;
        other.regionTags = nullptr;
    }
    return *this;
}

inline int SharingRegionTagSet::getNumTags() const
{
    return regionTags == nullptr ? 0 : regionTags->size();
}

inline void SharingRegionTagSet::setTag(int index, const Ptr<const TagBase>& tag)
{
    (*regionTags)[index].setTag(tag);
}

inline const Ptr<const TagBase>& SharingRegionTagSet::getTag(int index) const
{
    return getRegionTag(index).getTag();
}

inline const Ptr<TagBase> SharingRegionTagSet::getTagForUpdate(int index)
{
    return constPtrCast<TagBase>(getRegionTagForUpdate(index).getTag());
}

inline const SharingRegionTagSet::RegionTag<TagBase>& SharingRegionTagSet::getRegionTag(int index) const
{
    return (*regionTags)[index];
}

inline const SharingRegionTagSet::RegionTag<TagBase> SharingRegionTagSet::getRegionTagForUpdate(int index)
{
    const auto& regionTag = getRegionTag(index);
    if (regionTag.getTag().use_count() != 1) {
        prepareTagsVectorForUpdate();
        setTag(index, regionTag.getTag()->dupShared());
    }
    return regionTag;
}

inline void SharingRegionTagSet::sortTagsVector()
{
    std::sort(regionTags->begin(), regionTags->end());
}

template <typename T>
inline int SharingRegionTagSet::getTagIndex(b offset, b length) const
{
    return getTagIndex(typeid(T), offset, length);
}

template <typename T>
inline void SharingRegionTagSet::splitTags(b offset)
{
    splitTags(typeid(T), offset);
}

template <typename T>
inline const Ptr<const T> SharingRegionTagSet::findTag(b offset, b length) const
{
    int index = getTagIndex<T>(offset, length);
    return index == -1 ? nullptr : staticPtrCast<const T>(getTag(index));
}

template <typename T>
inline const Ptr<T> SharingRegionTagSet::findTagForUpdate(b offset, b length)
{
    int index = getTagIndex<T>(offset, length);
    return index == -1 ? nullptr : staticPtrCast<T>(getTagForUpdate(index));
}

template <typename T>
inline const Ptr<const T> SharingRegionTagSet::getTag(b offset, b length) const
{
    int index = getTagIndex<T>(offset, length);
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<const T>(getTag(index));
}

template <typename T>
inline const Ptr<T> SharingRegionTagSet::getTagForUpdate(b offset, b length)
{
    int index = getTagIndex<T>(offset, length);
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<T>(getTagForUpdate(index));
}

template <typename T>
inline const Ptr<T> SharingRegionTagSet::addTag(b offset, b length)
{
    int index = getTagIndex<T>(offset, length);
    if (index != -1)
        throw cRuntimeError("Tag '%s' is present", opp_typename(typeid(T)));
    Ptr<T> tag = makeShared<T>();
    addTag(offset, length, tag);
    return tag;
}

template <typename T>
inline const Ptr<T> SharingRegionTagSet::addTagIfAbsent(b offset, b length)
{
    const Ptr<T>& tag = findTagForUpdate<T>(offset, length);
    if (tag != nullptr)
        return tag;
    else {
        const Ptr<T>& tag = makeShared<T>();
        addTag(offset, length, tag);
        return tag;
    }
}

template <typename T>
inline std::vector<SharingRegionTagSet::RegionTag<T>> SharingRegionTagSet::addTagsWhereAbsent(b offset, b length)
{
    splitTags<T>(offset);
    splitTags<T>(offset + length);
    std::vector<SharingRegionTagSet::RegionTag<T>> result;
    for (auto& region : addTagsWhereAbsent(typeid(T), offset, length, makeShared<T>()))
        result.push_back(RegionTag<T>(region.getOffset(), region.getLength(), staticPtrCast<const T>(region.getTag())));
    return result;
}

template <typename T>
inline const Ptr<T> SharingRegionTagSet::removeTag(b offset, b length)
{
    int index = getTagIndex<T>(offset, length);
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return staticPtrCast<T>(removeTag(index));
}

template <typename T>
inline const Ptr<T> SharingRegionTagSet::removeTagIfPresent(b offset, b length)
{
    int index = getTagIndex<T>(offset, length);
    return index == -1 ? nullptr : staticPtrCast<T>(removeTag(index));
}

template <typename T>
inline void SharingRegionTagSet::mapAllTags(b offset, b length, std::function<void (b, b, const Ptr<const T>&)> f) const
{
    mapAllTags(offset, length, [&] (b o, b l, const Ptr<const TagBase>& tag) {
        auto tagObject = tag.get();
        if (typeid(*tagObject) == typeid(T))
            f(o, l, staticPtrCast<const T>(tag));
    });
}

template <typename T>
inline void SharingRegionTagSet::mapAllTagsForUpdate(b offset, b length, std::function<void (b, b, const Ptr<T>&)> f)
{
    splitTags<T>(offset);
    splitTags<T>(offset + length);
    mapAllTagsForUpdate(offset, length, [&] (b o, b l, const Ptr<TagBase>& tag) {
        auto tagObject = tag.get();
        if (typeid(*tagObject) == typeid(T))
            f(o, l, staticPtrCast<T>(tag));
    });
}

template <typename T>
inline std::vector<SharingRegionTagSet::RegionTag<T>> SharingRegionTagSet::getAllTags(b offset, b length) const
{
    std::vector<SharingRegionTagSet::RegionTag<T>> result;
    mapAllTags<T>(offset, length, [&] (b o, b l, const Ptr<const T>& tag) {
        result.push_back(RegionTag<T>(o, l, staticPtrCast<const T>(tag)));
    });
    return result;
}

template <typename T>
inline std::vector<SharingRegionTagSet::RegionTag<T>> SharingRegionTagSet::getAllTagsForUpdate(b offset, b length)
{
    std::vector<SharingRegionTagSet::RegionTag<T>> result;
    mapAllTagsForUpdate<T>(offset, length, [&] (b o, b l, const Ptr<T>& tag) {
        result.push_back(RegionTag<T>(o, l, staticPtrCast<T>(tag)));
    });
    return result;
}

template <typename T>
inline std::vector<SharingRegionTagSet::RegionTag<T>> SharingRegionTagSet::removeTagsWherePresent(b offset, b length)
{
    auto result = getAllTags<T>(offset, length);
    clearTags(offset, length);
    return result;
}

} // namespace

#endif // #ifndef __INET_SHARINGREGIONTAGSET_H_

