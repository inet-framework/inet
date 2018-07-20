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

#include "inet/networklayer/mpls/MplsPacket_m.h"

namespace inet {

std::string MplsHeader::str() const
{
    std::stringstream out;
    for (int i = (int)labels_arraysize - 1; i >= 0; i--)
        out << labels[i].getLabel() << (i == 0 ? "" : " ");
    return out.str();
}

} // namespace inet

