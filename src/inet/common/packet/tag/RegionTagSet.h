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

#ifndef __INET_REGIONTAGSET_H_
#define __INET_REGIONTAGSET_H_

#include "inet/common/INETDefs.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

/**
 * This class maintains a set of tags each referring to a specific region.
 * Regions are identified by their offset and length, and they are not allowed
 * to overlap. Tags are usually small data strcutures that hold some relevant
 * information. Tags are identified by their type, which means that this class
 * supports adding the same tag type for a specific region only once. Added tags
 * are exclusively owned by this class and they get deleted with it.
 */
class INET_API RegionTagSet : public cObject
{
  public:
    /**
     * Region tags keep a tag for a specific range identified by offset and length.
     */
    template <typename T>
    class INET_API RegionTag
    {
      protected:
        b offset;
        b length;
        // TODO: use shared pointer Ptr<T>
        T* tag;

      public:
        RegionTag(b offset, b length, T* tag) : offset(offset), length(length), tag(tag) { }
        RegionTag(const RegionTag& other) : offset(other.offset), length(other.length), tag(other.tag->dup()) { }
        RegionTag(RegionTag&& other) : offset(other.offset), length(other.length), tag(other.tag) { other.tag = nullptr; }
        ~RegionTag() { delete tag; }

        RegionTag& operator=(const RegionTag& other) {
            if (this != &other) {
                delete tag;
                offset = other.offset; length = other.length; tag = other.tag->dup();
            }
            return *this;
        }
        RegionTag& operator=(RegionTag&& other) {
            if (this != &other) {
                delete tag;
                offset = other.offset; length = other.length; tag = other.tag; other.tag = nullptr;
            }
            return *this;
        }

        b getOffset() const { return offset; }
        void setOffset(b offset) { this->offset = offset; }

        b getLength() const { return length; }
        void setLength(b length) { this->length = length; }

        b getStartOffset() const { return offset; }
        b getEndOffset() const { return offset + length; }

        T* getTag() const { return tag; }
    };

    typedef RegionTag<cObject> cObjectRegionTag;

  protected:
    std::vector<RegionTag<cObject>> *regionTags;

  protected:
    void ensureAllocated();

    void addTag(b offset, b length, cObject *tag);
    cObject *removeTag(int index);
    void clearAllTags();

    int getTagIndex(const std::type_info& typeInfo, b offset, b length) const;
    template <typename T> int getTagIndex(b offset, b length) const;

  public:
    RegionTagSet();
    RegionTagSet(const RegionTagSet& other);
    RegionTagSet(RegionTagSet&& other);
    ~RegionTagSet();

    RegionTagSet& operator=(const RegionTagSet& other);
    RegionTagSet& operator=(RegionTagSet&& other);

    /**
     * Returns the number of tags.
     */
    inline int getNumTags() const;

    /**
     * Returns the tag at the given index.
     */
    inline cObject *getTag(int index) const;

    /**
     * Returns the region tag at the given index.
     */
    inline const RegionTag<cObject>& getRegionTag(int index) const;

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
    void copyTags(const RegionTagSet& source, b sourceOffset, b offset, b length);

    /**
     * Returns the tag for the provided type and range, or returns nullptr if no such tag is found.
     */
    template <typename T> T *findTag(b offset, b length) const;

    /**
     * Returns the tag for the provided type and range, or throws an exception if no such tag is found.
     */
    template <typename T> T *getTag(b offset, b length) const;

    /**
     * Returns all tags for the provided type and range.
     */
    template <typename T> std::vector<RegionTag<T>> getAllTags(b offset, b length) const;

    /**
     * Returns a newly added tag for the provided type and range, or throws an exception if such a tag is already present.
     */
    template <typename T> T *addTag(b offset, b length);

    /**
     * Returns a newly added tag for the provided type and range if absent, or returns the tag that is already present.
     */
    template <typename T> T *addTagIfAbsent(b offset, b length);

    /**
     * Removes the tag for the provided type and range, or throws an exception if no such tag is found.
     */
    template <typename T> T *removeTag(b offset, b length);

    /**
     * Removes the tag for the provided type and range if present, or returns nullptr if no such tag is found.
     */
    template <typename T> T *removeTagIfPresent(b offset, b length);

    /**
     * Removes and returns all tags for the provided type and range.
     */
    template <typename T> std::vector<RegionTag<T>> removeAllTags(b offset, b length);
};

inline int RegionTagSet::getNumTags() const
{
    return regionTags == nullptr ? 0 : regionTags->size();
}

inline cObject *RegionTagSet::getTag(int index) const
{
    return getRegionTag(index).getTag();
}

inline const RegionTagSet::RegionTag<cObject>& RegionTagSet::getRegionTag(int index) const
{
    return regionTags->at(index);
}

template <typename T>
inline int RegionTagSet::getTagIndex(b offset, b length) const
{
    return getTagIndex(typeid(T), offset, length);
}

template <typename T>
inline T *RegionTagSet::findTag(b offset, b length) const
{
    int index = getTagIndex<T>(offset, length);
    return index == -1 ? nullptr : static_cast<T *>((*regionTags)[index].getTag());
}

template <typename T>
inline T *RegionTagSet::getTag(b offset, b length) const
{
    int index = getTagIndex<T>(offset, length);
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return static_cast<T *>((*regionTags)[index].getTag());
}

template <typename T>
inline T *RegionTagSet::addTag(b offset, b length)
{
    int index = getTagIndex<T>(offset, length);
    if (index != -1)
        throw cRuntimeError("Tag '%s' is present", opp_typename(typeid(T)));
    T *tag = new T();
    addTag(offset, length, tag);
    return tag;
}

template <typename T>
inline T *RegionTagSet::addTagIfAbsent(b offset, b length)
{
    T *tag = findTag<T>(offset, length);
    if (tag == nullptr)
        addTag(offset, length, tag = new T());
    return tag;
}

template <typename T>
inline T *RegionTagSet::removeTag(b offset, b length)
{
    int index = getTagIndex<T>(offset, length);
    if (index == -1)
        throw cRuntimeError("Tag '%s' is absent", opp_typename(typeid(T)));
    return static_cast<T *>(removeTag(index));
}

template <typename T>
inline T *RegionTagSet::removeTagIfPresent(b offset, b length)
{
    int index = getTagIndex<T>(offset, length);
    return index == -1 ? nullptr : static_cast<T *>(removeTag(index));
}

template <typename T>
inline std::vector<RegionTagSet::RegionTag<T>> RegionTagSet::getAllTags(b offset, b length) const
{
    if (regionTags == nullptr)
        return std::vector<RegionTagSet::RegionTag<T>>();
    else {
        std::vector<RegionTagSet::RegionTag<T>> result;
        b getStartOffset = offset;
        b getEndOffset = offset + length;
        for (auto& regionTag : *regionTags) {
            if (auto tag = dynamic_cast<T *>(regionTag.getTag())) {
                if (getEndOffset <= regionTag.getStartOffset() || regionTag.getEndOffset() <= getStartOffset)
                    // no intersection
                    continue;
                else if (getStartOffset <= regionTag.getStartOffset() && regionTag.getEndOffset() <= getEndOffset)
                    // get totally covers region
                    result.push_back(RegionTag<T>(regionTag.getOffset(), regionTag.getLength(), tag->dup()));
                else if (regionTag.getStartOffset() < getStartOffset && getEndOffset < regionTag.getEndOffset())
                    // get splits region into two parts
                    result.push_back(RegionTag<T>(getStartOffset, getEndOffset - getStartOffset, tag->dup()));
                else if (regionTag.getEndOffset() <= getEndOffset)
                    // get cuts end of region
                    result.push_back(RegionTag<T>(getStartOffset, regionTag.getEndOffset() - getStartOffset, tag->dup()));
                else if (getStartOffset <= regionTag.getStartOffset())
                    // get cuts beginning of region
                    result.push_back(RegionTag<T>(regionTag.getStartOffset(), getEndOffset - regionTag.getStartOffset(), tag->dup()));
                else
                    ASSERT(false);
            }
        }
        return result;
    }
}

template <typename T>
inline std::vector<RegionTagSet::RegionTag<T>> RegionTagSet::removeAllTags(b offset, b length)
{
    auto result = getAllTags<T>(offset, length);
    clearTags(offset, length);
    return result;
}

} // namespace

#endif // #ifndef __INET_REGIONTAGSET_H_

