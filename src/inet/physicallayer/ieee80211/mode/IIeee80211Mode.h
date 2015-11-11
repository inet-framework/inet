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

#ifndef __INET_IIEEE80211MODE_H
#define __INET_IIEEE80211MODE_H

#include "inet/physicallayer/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

class INET_API IIeee80211PreambleMode : public IPrintableObject
{
  public:
    virtual const simtime_t getDuration() const = 0;
};

class INET_API IIeee80211HeaderMode : public IPrintableObject
{
  public:
    virtual bps getNetBitrate() const = 0;
    virtual bps getGrossBitrate() const = 0;
    virtual int getBitLength() const = 0;
    virtual const simtime_t getDuration() const = 0;
    virtual const IModulation *getModulation() const = 0;
};

class INET_API IIeee80211DataMode : public IPrintableObject
{
  public:
    virtual bps getNetBitrate() const = 0;
    virtual bps getGrossBitrate() const = 0;
    virtual int getBitLength(int dataBitLength) const = 0;
    virtual const simtime_t getDuration(int dataBitLength) const = 0;
    virtual const IModulation *getModulation() const = 0;
    virtual int getNumberOfSpatialStreams() const = 0;
};

class INET_API IIeee80211Mode : public IPrintableObject
{
  public:
    virtual int getLegacyCwMin() const = 0;
    virtual int getLegacyCwMax() const = 0;
    virtual const char *getName() const = 0;
    virtual const IIeee80211PreambleMode *getPreambleMode() const = 0;
    virtual const IIeee80211HeaderMode *getHeaderMode() const = 0;
    virtual const IIeee80211DataMode *getDataMode() const = 0;
    virtual const simtime_t getDuration(int dataBitLength) const = 0;
    virtual const simtime_t getSlotTime() const = 0;
    virtual const simtime_t getSifsTime() const = 0;
    virtual const simtime_t getRifsTime() const = 0;
    virtual const simtime_t getCcaTime() const = 0;
    virtual const simtime_t getPhyRxStartDelay() const = 0;
    virtual const simtime_t getRxTxTurnaroundTime() const = 0;
    virtual const simtime_t getPreambleLength() const = 0;
    virtual const simtime_t getPlcpHeaderLength() const = 0;
    virtual int getMpduMaxLength() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IIEEE80211MODE_H

