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


#include <omnetpp.h>
#include "MPLSPacket.h"

// constructors
MPLSPacket::MPLSPacket(const char *name) : cMessage(name)
{
}

MPLSPacket::MPLSPacket(const MPLSPacket & p)
{
    setName(p.name());
    operator=(p);
}

// assignment operator
MPLSPacket & MPLSPacket::operator=(const MPLSPacket & p)
{
    cMessage::operator=(p);
    return *this;
}


