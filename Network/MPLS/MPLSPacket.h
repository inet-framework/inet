/***************************************************************************
*
*    This library is free software, you can redistribute it and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
***************************************************************************/


#ifndef __MPLSPacket_H
#define __MPLSPacket_H

#include <omnetpp.h>
#include <list>
#include <queue>

using namespace std;

// label stack
typedef queue<int,list<int> > IQueue;

// FIXME length should be adjusted when length of label stack changes
class MPLSPacket: public cPacket
{
  private:
    IQueue label;

  public:
    /* constructors*/
    MPLSPacket(const char *name=NULL);
    MPLSPacket(const MPLSPacket &p);

    /* assignment operator*/
    virtual MPLSPacket& operator=(const MPLSPacket& p);

    /**
     * cloning function
     */
    virtual cObject *dup() const { return new MPLSPacket(*this); }

    /*swapLabel:    Swap Label operation
     *@param: newLabel - The new Label to use
     **/
    inline void swapLabel(int newLabel){label.pop();label.push(newLabel);}

    /*pushLabel:    Push new label
     **/
    inline void pushLabel(int newLabel){label.push(newLabel);}

    /*popLabel:    Pop out top label
     **/
    inline void popLabel(){label.pop();}

    /*noLabel:    Empty the label stack
     **/
    inline bool noLabel(){return label.empty(); }

    /*getLabel:    Get the top label
     *@param: none
     **/
    inline int getLabel(){return label.front();}
};

#endif


