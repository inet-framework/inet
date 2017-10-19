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
#include "inet/networklayer/mpls/MplsPacket_m.h"

namespace inet {

/**
 * Represents a packet with MPLS headers
 */
class INET_API MplsHeader : public MplsHeader_Base
{
  protected:
    typedef std::vector<MplsLabel> LabelStack;    // note: last element is the top of stack
    LabelStack labels;

  private:
    void copy(const MplsHeader& other) { labels = other.labels; }

  public:
    /* constructors*/
    MplsHeader() : MplsHeader_Base() {}

    /* assignment operator*/
    MplsHeader& operator=(const MplsHeader& other);

    /**
     * cloning function
     */
    virtual MplsHeader *dup() const override { return new MplsHeader(*this); }

    /**
     * Returns a string with the labels, starting with the top of stack.
     */
    virtual std::string info() const override;

    /**
     * Swap Label operation
     */
    inline void swapLabel(MplsLabel newLabel) { labels.back() = newLabel; }

    /**
     * Pushes new label on the label stack
     */
    inline void pushLabel(MplsLabel newLabel) { labels.push_back(newLabel); }

    /**
     * Pops the top label
     */
    inline void popLabel() { labels.pop_back(); }

    /**
     * Returns true if the label stack is not empty
     */
    inline bool hasLabel() { return !labels.empty(); }

    /**
     * Returns the top label
     */
    inline MplsLabel getTopLabel() { return labels.back(); }

    virtual b getChunkLength() const override { return b(32) * labels.size(); }

    virtual void setLabelsArraySize(unsigned int size) override { throw cRuntimeError("do not use it"); }
    virtual unsigned int getLabelsArraySize() const override { return labels.size(); }
    virtual MplsLabel& getMutableLabels(unsigned int k) override { throw cRuntimeError("do not use it"); }
    virtual const MplsLabel& getLabels(unsigned int k) const override { throw cRuntimeError("do not use it"); }
    virtual void setLabels(unsigned int k, const MplsLabel& labels) override { throw cRuntimeError("do not use it"); }
};

} // namespace inet

#endif // ifndef __INET_MPLSPACKET_H

