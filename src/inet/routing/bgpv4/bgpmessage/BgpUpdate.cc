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

unsigned short BgpUpdateMessage::computePathAttributesBytes(const BgpUpdatePathAttributeList& pathAttrs)
{
    unsigned short contentBytes = 0;
    // BgpUpdatePathAttributesOrigin
    contentBytes += pathAttrs.getOrigin().getType().flags.estendedLengthBit ? 4 : 3;
    ASSERT(pathAttrs.getOrigin().getLength() == 1);
    contentBytes += 1;

    // BgpUpdatePathAttributesAsPath
    for (size_t i = 0; i < pathAttrs.getAsPathArraySize(); i++) {
        auto& attr = pathAttrs.getAsPath(i);
        contentBytes += attr.getType().flags.estendedLengthBit ? 4 : 3;
#ifndef NDEBUG
        {
            unsigned short s = 0;
            for (size_t j = 0; j < attr.getValueArraySize(); j++) {
                ASSERT(attr.getValue(j).getLength() == attr.getValue(j).getAsValueArraySize());
                s += 2 + 2 * attr.getValue(j).getAsValueArraySize();
            }
            ASSERT(s == attr.getLength());
        }
#endif
        contentBytes += attr.getLength(); // type (1) + length (1) + value
    }

    // BgpUpdatePathAttributesNextHop
    contentBytes += pathAttrs.getNextHop().getType().flags.estendedLengthBit ? 4 : 3;
    ASSERT(pathAttrs.getNextHop().getLength() == 4);
    contentBytes += 4;

    // BgpUpdatePathAttributesLocalPref
    for (size_t i = 0; i < pathAttrs.getLocalPrefArraySize(); i++) {
        auto& attr = pathAttrs.getLocalPref(i);
        contentBytes += attr.getType().flags.estendedLengthBit ? 4 : 3;
        ASSERT(attr.getLength() == 4);
        contentBytes += 4;
    }

    // BgpUpdatePathAttributesAtomicAggregate
    for (size_t i = 0; i < pathAttrs.getAtomicAggregateArraySize(); i++) {
        auto& attr = pathAttrs.getAtomicAggregate(i);
        contentBytes += attr.getType().flags.estendedLengthBit ? 4 : 3;
        ASSERT(attr.getLength() == 0);
    }

    return contentBytes;
}

void BgpUpdateMessage::setPathAttributeList(const BgpUpdatePathAttributeList& pathAttrs)
{
    unsigned int old_bytes = getPathAttributeListArraySize() == 0 ? 0 : computePathAttributesBytes(getPathAttributeList(0));
    unsigned int new_bytes = computePathAttributesBytes(pathAttrs);

    setPathAttributeListArraySize(1);
    BgpUpdateMessage_Base::setPathAttributeList(0, pathAttrs);

    setChunkLength(getChunkLength() + B(new_bytes - old_bytes));
}

void BgpUpdateMessage::setNLRI(const BgpUpdateNlri& NLRI_var)
{
    //FIXME length = B(1) + B((length+7)/8)
    B oldLen = BgpUpdateMessage_Base::NLRI.length == 0 ? B(0) : (B(1) + B((BgpUpdateMessage_Base::NLRI.length + 7) / 8));
    B newLen = NLRI_var.length == 0 ? B(0) : (B(1) + B((NLRI_var.length + 7) / 8));
    setChunkLength(getChunkLength() - oldLen + newLen);
    BgpUpdateMessage_Base::NLRI = NLRI_var;
}

} // namespace bgp

} // namespace inet

