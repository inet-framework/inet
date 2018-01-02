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

#ifndef __INET_COMMAND_H_
#define __INET_COMMAND_H_

#include "inet/common/packet/tag/TagSet.h"

namespace inet {

class INET_API Command : public cMessage
{
  friend class CommandDescriptor;

  protected:
    TagSet tags; // TODO: replace omnet API with this one

  public:
    explicit Command(const char *name = nullptr, short kind = 0);
    Command(const Command& other);

    virtual Command *dup() const override { return new Command(*this); }

    /** @name Tag related functions */
    //@{
    /**
     * Returns the number of command tags.
     */
    int getNumTags() const {
        return tags.getNumTags();
    }

    /**
     * Returns the command tag at the given index.
     */
    cObject *getTag(int index) const {
        return tags.getTag(index);
    }

    /**
     * Clears the set of command tags.
     */
    void clearTags() {
        tags.clearTags();
    }

    /**
     * Copies the set of command tags from the other command.
     */
    void copyTags(const Command& source) {
        tags.copyTags(source.tags);
    }

    /**
     * Returns the command tag for the provided type or returns nullptr if no such command tag is found.
     */
    template<typename T> T *findTag() const {
        return tags.findTag<T>();
    }

    /**
     * Returns the command tag for the provided type or throws an exception if no such command tag is found.
     */
    template<typename T> T *getTag() const {
        return tags.getTag<T>();
    }

    /**
     * Returns a newly added command tag for the provided type, or throws an exception if such a command tag is already present.
     */
    template<typename T> T *addTag() {
        return tags.addTag<T>();
    }

    /**
     * Returns a newly added command tag for the provided type if absent, or returns the command tag that is already present.
     */
    template<typename T> T *addTagIfAbsent() {
        return tags.addTagIfAbsent<T>();
    }

    /**
     * Removes the command tag for the provided type, or throws an exception if no such command tag is found.
     */
    template<typename T> T *removeTag() {
        return tags.removeTag<T>();
    }

    /**
     * Removes the command tag for the provided type if present, or returns nullptr if no such command tag is found.
     */
    template<typename T> T *removeTagIfPresent() {
        return tags.removeTagIfPresent<T>();
    }
    //@}
};

inline std::ostream& operator<<(std::ostream& os, const Command *command) { return os << command->str(); }

inline std::ostream& operator<<(std::ostream& os, const Command& command) { return os << command.str(); }

} // namespace

#endif // #ifndef __INET_COMMAND_H_

