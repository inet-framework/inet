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

#ifndef __INET_APSKPHYFRAMESERIALIZER_H
#define __INET_APSKPHYFRAMESERIALIZER_H

#include "inet/common/BitVector.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKPhyFrame_m.h"

namespace inet {

namespace physicallayer {

#define APSK_PHY_FRAME_HEADER_BYTE_LENGTH    6

class INET_API APSKPhyFrameSerializer
{
  public:
    APSKPhyFrameSerializer();

    virtual BitVector *serialize(const APSKPhyFrame *phyFrame) const;
    virtual APSKPhyFrame *deserialize(const BitVector *bits) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_APSKPHYFRAMESERIALIZER_H

