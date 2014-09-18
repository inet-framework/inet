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

#include "inet/common/INETDefs.h"

#include "inet/networklayer/mpls/MPLSPacket.h"

namespace inet {

// constructors
MPLSPacket::MPLSPacket(const char *name) : cPacket(name)
{
}

MPLSPacket::MPLSPacket(const MPLSPacket& p)
    : cPacket(p)
{
    copy(p);
}

// assignment operator
MPLSPacket& MPLSPacket::operator=(const MPLSPacket& p)
{
    if (this == &p)
        return *this;
    cPacket::operator=(p);
    copy(p);
    return *this;
}

std::string MPLSPacket::info() const
{
    std::stringstream out;
    for (int i = (int)labels.size() - 1; i >= 0; i--)
        out << labels[i] << (i == 0 ? "" : " ");
    return out.str();
}

} // namespace inet

