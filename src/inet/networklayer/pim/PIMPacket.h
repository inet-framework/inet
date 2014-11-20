//
// Copyright (C) 2014 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef _PIMPACKET_H_
#define _PIMPACKET_H_

#include "inet/networklayer/pim/PIMPacket_m.h"

class INET_API PIMHello : public PIMHello_Base
{
  protected:
    HelloOptionPtr *options_var; // array ptr
    unsigned int options_arraysize;

  private:
    void copy(const PIMHello& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const PIMHello&);

  public:
    PIMHello(const char *name=NULL, int kind=0);
    PIMHello(const PIMHello& other);
    virtual ~PIMHello();
    PIMHello& operator=(const PIMHello& other);
    virtual PIMHello *dup() const {return new PIMHello(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual void setOptionsArraySize(unsigned int size);
    virtual unsigned int getOptionsArraySize() const;
    virtual HelloOptionPtr& getOptions(unsigned int k);
    virtual const HelloOptionPtr& getOptions(unsigned int k) const {return const_cast<PIMHello*>(this)->getOptions(k);}
    virtual void setOptions(unsigned int k, const HelloOptionPtr& options);
};


#endif // _PIMPACKET_H_
