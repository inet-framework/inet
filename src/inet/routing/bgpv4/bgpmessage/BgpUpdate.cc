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

unsigned short computePathAttributeBytes(const BgpUpdatePathAttributes& pathAttr)
{
    unsigned short contentBytes = pathAttr.getExtendedLengthBit() ? 4 : 3;
    switch (pathAttr.getTypeCode()) {
        case BgpUpdateAttributeTypeCode::ORIGIN: {
            auto& attr = *check_and_cast<const BgpUpdatePathAttributesOrigin *>(&pathAttr);
            ASSERT(attr.getLength() == 1);
            contentBytes += 1;
            return contentBytes;
        }
        case BgpUpdateAttributeTypeCode::AS_PATH: {
            auto& attr = *check_and_cast<const BgpUpdatePathAttributesAsPath*>(&pathAttr);
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
            return contentBytes;
        }
        case BgpUpdateAttributeTypeCode::NEXT_HOP: {
            auto& attr = *check_and_cast<const BgpUpdatePathAttributesNextHop*>(&pathAttr);
            ASSERT(attr.getLength() == 4);
            contentBytes += 4;
            return contentBytes;
        }
        case BgpUpdateAttributeTypeCode::LOCAL_PREF: {
            auto& attr = *check_and_cast<const BgpUpdatePathAttributesLocalPref*>(&pathAttr);
            ASSERT(attr.getLength() == 4);
            contentBytes += 4;
            return contentBytes;
        }
        case BgpUpdateAttributeTypeCode::ATOMIC_AGGREGATE: {
            auto& attr = *check_and_cast<const BgpUpdatePathAttributesAtomicAggregate*>(&pathAttr);
            ASSERT(attr.getLength() == 0);
            return contentBytes;
        }
        default:
            throw cRuntimeError("Unknown BgpUpdateAttributeTypeCode: %d", (int)pathAttr.getTypeCode());
    }
}

} // namespace bgp
} // namespace inet

