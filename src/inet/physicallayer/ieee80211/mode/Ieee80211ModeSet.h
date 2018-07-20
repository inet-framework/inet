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

#ifndef __INET_IEEE80211MODESET_H
#define __INET_IEEE80211MODESET_H

#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/common/DelayedInitializer.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211ModeSet : public IPrintableObject, public cObject
{
  protected:
    class INET_API Entry
    {
      public:
        bool isMandatory;
        const IIeee80211Mode *mode;
    };

    struct EntryNetBitrateComparator {
        bool operator() (const Entry& left, const Entry& right) { return left.mode->getDataMode()->getNetBitrate() < right.mode->getDataMode()->getNetBitrate(); }
    };

  protected:
    std::string name;
    const std::vector<Entry> entries;

  public:
    static const DelayedInitializer<std::vector<Ieee80211ModeSet>> modeSets;

  protected:
    int findModeIndex(const IIeee80211Mode *mode) const;
    int getModeIndex(const IIeee80211Mode *mode) const;

  public:
    Ieee80211ModeSet(const char *name, const std::vector<Entry> entries);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Ieee80211ModeSet, name = " << name; }

    const char *getName() const override { return name.c_str(); }

    bool containsMode(const IIeee80211Mode *mode) const { return findModeIndex(mode) != -1; }
    bool getIsMandatory(const IIeee80211Mode *mode) const;

    const IIeee80211Mode *findMode(bps bitrate, Hz bandwidth = Hz(NaN), int numSpatialStreams = -1) const;
    const IIeee80211Mode *findMode(bps minBitrate, bps maxBitrate, Hz bandwidth = Hz(NaN), int numSpatialStreams = -1) const;
    const IIeee80211Mode *getMode(bps bitrate, Hz bandwidth = Hz(NaN), int numSpatialStreams = -1) const;
    const IIeee80211Mode *getMode(bps minBitrate, bps maxBitrate, Hz bandwidth = Hz(NaN), int numSpatialStreams = -1) const;
    const IIeee80211Mode *getSlowestMode() const;
    const IIeee80211Mode *getFastestMode() const;
    const IIeee80211Mode *getSlowerMode(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *getFasterMode(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *getSlowestMandatoryMode() const;
    const IIeee80211Mode *getFastestMandatoryMode() const;
    const IIeee80211Mode *getSlowerMandatoryMode(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *getFasterMandatoryMode(const IIeee80211Mode *mode) const;

    static const Ieee80211ModeSet *findModeSet(const char *mode);
    static const Ieee80211ModeSet *getModeSet(const char *mode);

    simtime_t getSifsTime() const { return entries[0].mode->getSifsTime(); }
    simtime_t getSlotTime() const { return entries[0].mode->getSlotTime(); }
    simtime_t getPhyRxStartDelay() const { return entries[0].mode->getPhyRxStartDelay(); }
    int getCwMin() const { return entries[0].mode->getLegacyCwMin(); }
    int getCwMax() const { return entries[0].mode->getLegacyCwMax(); }

    IIeee80211Mode *_getSlowestMode() const { return const_cast<IIeee80211Mode*>(getSlowestMode()); }
    IIeee80211Mode *_getFastestMode() const { return const_cast<IIeee80211Mode*>(getFastestMode()); }
    IIeee80211Mode *_getSlowestMandatoryMode() const { return const_cast<IIeee80211Mode*>(getSlowestMandatoryMode()); }
    IIeee80211Mode *_getFastestMandatoryMode() const { return const_cast<IIeee80211Mode*>(getFastestMandatoryMode()); }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211MODESET_H

