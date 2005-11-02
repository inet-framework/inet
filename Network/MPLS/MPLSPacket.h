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

#ifndef __MPLSPACKET_H
#define __MPLSPACKET_H

#include <stack>
#include <omnetpp.h>
#include "INETDefs.h"

/**
 * Represents a packet with MPLS headers
 */
class INET_API MPLSPacket: public cMessage
{
  private:
    typedef std::stack<int> LabelStack;
    LabelStack labels;

  public:
    /* constructors*/
    MPLSPacket(const char *name=NULL);
    MPLSPacket(const MPLSPacket &p);

    /* assignment operator*/
    virtual MPLSPacket& operator=(const MPLSPacket& p);

    /**
     * cloning function
     */
    virtual cObject *dup() const {return new MPLSPacket(*this);}

    /**
     * Swap Label operation
     */
    inline void swapLabel(int newLabel)  {labels.top()=newLabel;}

    /**
     * Pushes new label on the label stack
     */
    inline void pushLabel(int newLabel)  {labels.push(newLabel);addLength(32);}

    /**
     * Pops the top label
     */
    inline void popLabel()  {labels.pop();addLength(-32);}

    /**
     * Returns true if the label stack is not empty
     */
    inline bool hasLabel()  {return !labels.empty();}

    /**
     * Returns the top label
     */
    inline int topLabel()  {return labels.top();}
};

#endif


