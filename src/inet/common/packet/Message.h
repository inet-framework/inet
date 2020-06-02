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

#ifndef __INET_MESSAGE_H_
#define __INET_MESSAGE_H_

#include "inet/common/packet/tag/SharingTagSet.h"
#include "inet/common/TagBase.h"

namespace inet {

class INET_API Message : public cMessage
{
  friend class MessageDescriptor;

  protected:
    SharingTagSet tags;

  protected:
    /** @name Class descriptor functions */
    //@{
    const TagBase *_getTag(int index) { return tags.getTag(index).get(); }
    //@}

  public:
    explicit Message(const char *name = nullptr, short kind = 0);
    Message(const Message& other);

    virtual Message *dup() const override { return new Message(*this); }

    /** @name Tag related functions */
    //@{
    /**
     * Returns all tags.
     */
    SharingTagSet& getTags() { return tags; }

    /**
     * Returns the number of message tags.
     */
    int getNumTags() const {
        return tags.getNumTags();
    }

    /**
     * Returns the message tag at the given index.
     */
    const Ptr<const TagBase> getTag(int index) const {
        return tags.getTag(index);
    }

    /**
     * Clears the set of message tags.
     */
    void clearTags() {
        tags.clearTags();
    }

    /**
     * Copies the set of message tags from the other message.
     */
    void copyTags(const Message& source) {
        tags.copyTags(source.tags);
    }

    /**
     * Returns the message tag for the provided type or returns nullptr if no such message tag is found.
     */
    template<typename T> const Ptr<const T> findTag() const {
        return tags.findTag<T>();
    }

    /**
     * Returns the message tag for the provided type or returns nullptr if no such message tag is found.
     */
    template<typename T> const Ptr<T> findTagForUpdate() {
        return tags.findTagForUpdate<T>();
    }

    /**
     * Returns the message tag for the provided type or throws an exception if no such message tag is found.
     */
    template<typename T> const Ptr<const T> getTag() const {
        return tags.getTag<T>();
    }

    /**
     * Returns the message tag for the provided type or throws an exception if no such message tag is found.
     */
    template<typename T> const Ptr<T> getTagForUpdate() {
        return tags.getTagForUpdate<T>();
    }

    /**
     * Returns a newly added message tag for the provided type, or throws an exception if such a message tag is already present.
     */
    template<typename T> const Ptr<T> addTag() {
        return tags.addTag<T>();
    }

    /**
     * Returns a newly added message tag for the provided type if absent, or returns the message tag that is already present.
     */
    template<typename T> const Ptr<T> addTagIfAbsent() {
        return tags.addTagIfAbsent<T>();
    }

    /**
     * Removes the message tag for the provided type, or throws an exception if no such message tag is found.
     */
    template<typename T> const Ptr<T> removeTag() {
        return tags.removeTag<T>();
    }

    /**
     * Removes the message tag for the provided type if present, or returns nullptr if no such message tag is found.
     */
    template<typename T> const Ptr<T> removeTagIfPresent() {
        return tags.removeTagIfPresent<T>();
    }
    //@}
};

class INET_API Request : public Message
{
  public:
    explicit Request(const char *name = "Req", short kind = 0);
    Request(const Request& other);

    virtual Request *dup() const override { return new Request(*this); }
};

class INET_API Indication : public Message
{
  public:
    explicit Indication(const char *name = "Ind", short kind = 0);
    Indication(const Indication& other);

    virtual Indication *dup() const override { return new Indication(*this); }
};

inline std::ostream& operator<<(std::ostream& os, const Message *message) { return os << message->str(); }

inline std::ostream& operator<<(std::ostream& os, const Message& message) { return os << message.str(); }

} // namespace

#endif // #ifndef __INET_MESSAGE_H_

