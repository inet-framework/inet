//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_MPLSPACKET_H
#define __INET_MPLSPACKET_H

#include <stack>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Represents a packet with MPLS headers
 */
class INET_API MPLSPacket : public cPacket
{
  protected:
    typedef std::vector<int> LabelStack;    // note: last element is the top of stack
    LabelStack labels;

  private:
    void copy(const MPLSPacket& other) { labels = other.labels; }

  public:
    /* constructors*/
    MPLSPacket(const char *name = nullptr);
    MPLSPacket(const MPLSPacket& p);

    /* assignment operator*/
    virtual MPLSPacket& operator=(const MPLSPacket& p);

    /**
     * cloning function
     */
    virtual MPLSPacket *dup() const override { return new MPLSPacket(*this); }

    /**
     * Returns a string with the labels, starting with the top of stack.
     */
    virtual std::string info() const override;

    /**
     * Swap Label operation
     */
    inline void swapLabel(int newLabel) { labels.back() = newLabel; }

    /**
     * Pushes new label on the label stack
     */
    inline void pushLabel(int newLabel) { labels.push_back(newLabel); addBitLength(32); }

    /**
     * Pops the top label
     */
    inline void popLabel() { labels.pop_back(); addBitLength(-32); }

    /**
     * Returns true if the label stack is not empty
     */
    inline bool hasLabel() { return !labels.empty(); }

    /**
     * Returns the top label
     */
    inline int getTopLabel() { return labels.back(); }
};

} // namespace inet

#endif // ifndef __INET_MPLSPACKET_H

