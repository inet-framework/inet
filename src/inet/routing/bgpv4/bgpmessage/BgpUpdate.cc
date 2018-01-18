//
// Copyright (C) 2010 Helene Lageber
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

#include "inet/routing/bgpv4/bgpmessage/BgpUpdate.h"

namespace inet {

namespace bgp {

Register_Class(BgpUpdateMessage)

void BgpUpdateMessage::setWithdrawnRoutesArraySize(size_t size)
{
    unsigned short delta_size = size - getWithdrawnRoutesArraySize();
    unsigned short delta_bytes = delta_size * 5;    // 5 = Withdrawn Route length
    BgpUpdateMessage_Base::setWithdrawnRoutesArraySize(size);
    setChunkLength(getChunkLength() + B(delta_bytes));
}

unsigned short BgpUpdateMessage::computePathAttributesBytes(const BgpUpdatePathAttributeList& pathAttrs)
{
    unsigned short nb_path_attr = 2 + pathAttrs.getAsPathArraySize()
        + pathAttrs.getLocalPrefArraySize()
        + pathAttrs.getAtomicAggregateArraySize();

    // BgpUpdatePathAttributes (4)
    unsigned short contentBytes = nb_path_attr * 4;
    // BgpUpdatePathAttributesOrigin (1)
    contentBytes += 1;
    // BgpUpdatePathAttributesAsPath
    for (size_t i = 0; i < pathAttrs.getAsPathArraySize(); i++)
        contentBytes += 2 + pathAttrs.getAsPath(i).getLength(); // type (1) + length (1) + value
    // BgpUpdatePathAttributesNextHop (4)
    contentBytes += 4;
    // BgpUpdatePathAttributesLocalPref (4)
    contentBytes = 4 * pathAttrs.getLocalPrefArraySize();
    return contentBytes;
}

void BgpUpdateMessage::setPathAttributeList(const BgpUpdatePathAttributeList& pathAttrs)
{
    unsigned int old_bytes = getPathAttributeListArraySize() == 0 ? 0 : computePathAttributesBytes(getPathAttributeList(0));
    unsigned int delta_bytes = computePathAttributesBytes(pathAttrs) - old_bytes;

    setPathAttributeListArraySize(1);
    BgpUpdateMessage_Base::setPathAttributeList(0, pathAttrs);

    setChunkLength(getChunkLength() + B(delta_bytes));
}

void BgpUpdateMessage::setNLRI(const BgpUpdateNlri& NLRI_var)
{
    //FIXME bug: the length always incremented
    setChunkLength(getChunkLength() + B(5));    //5 = NLRI (length (1) + Ipv4Address (4))
    BgpUpdateMessage_Base::NLRI = NLRI_var;
}

} // namespace bgp

} // namespace inet

