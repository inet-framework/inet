//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_PIMPACKET_H
#define __INET_PIMPACKET_H

#include "inet/routing/pim/PimPacket_m.h"

namespace inet {

class INET_API PimHello : public PimHello_Base
{
  protected:
    HelloOption **options_var;    // array ptr
    unsigned int options_arraysize;

  private:
    void copy(const PimHello& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const PimHello&);

  public:
    PimHello();
    PimHello(const PimHello& other);
    virtual ~PimHello();
    PimHello& operator=(const PimHello& other);
    virtual PimHello *dup() const override { return new PimHello(*this); }
    virtual void parsimPack(cCommBuffer *b) const override;
    virtual void parsimUnpack(cCommBuffer *b) override;

    // field getter/setter methods
    virtual void setOptionsArraySize(unsigned int size) override;
    virtual unsigned int getOptionsArraySize() const override;
    virtual HelloOption *getMutableOptions(unsigned int k) override;
    virtual const HelloOption *getOptions(unsigned int k) const override;
    virtual void setOptions(unsigned int k, HelloOption *options) override;
};

}    // namespace inet

#endif    // _PIMPACKET_H_

