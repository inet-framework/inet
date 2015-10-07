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
#include "inet/linklayer/ieee80211/newmac/AccessCategory.h"  //TODO REMOVE -- do not reference AccessCategory from the physical layer!

namespace inet {

namespace physicallayer {

using namespace inet::ieee80211; // for the AccessCategory enum -- TODO remove!

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
  protected:
    virtual int getLegacyCwMin() const = 0;
    virtual int getLegacyCwMax() const = 0;

  public:
    virtual const IIeee80211PreambleMode *getPreambleMode() const = 0;
    virtual const IIeee80211HeaderMode *getHeaderMode() const = 0;
    virtual const IIeee80211DataMode *getDataMode() const = 0;
    virtual const simtime_t getDuration(int dataBitLength) const = 0;
    virtual int getAifsNumber(AccessCategory ac) const = 0;
    virtual const simtime_t getSlotTime() const = 0;
    virtual const simtime_t getSifsTime() const = 0;
    virtual const simtime_t getRifsTime() const = 0;
    virtual const simtime_t getEifsTime(const IIeee80211Mode *slowestMandatoryMode, AccessCategory ac, int ackLength) const = 0;
    virtual const simtime_t getDifsTime() const = 0;
    virtual const simtime_t getPifsTime() const = 0;
    virtual const simtime_t getAifsTime(AccessCategory ac) const = 0;
    virtual const simtime_t getCcaTime() const = 0;
    virtual const simtime_t getPhyRxStartDelay() const = 0;
    virtual const simtime_t getRxTxTurnaroundTime() const = 0;
    virtual const simtime_t getPreambleLength() const = 0;
    virtual const simtime_t getPlcpHeaderLength() const = 0;
    virtual const simtime_t getTxopLimit(AccessCategory ac) const = 0; // Table 8-105—Default EDCA Parameter Set element parameter values if dot11OCBActivated is false
    virtual int getCwMin(AccessCategory ac) const = 0;
    virtual int getCwMax(AccessCategory ac) const = 0;
    virtual int getMpduMaxLength() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IIEEE80211MODE_H

