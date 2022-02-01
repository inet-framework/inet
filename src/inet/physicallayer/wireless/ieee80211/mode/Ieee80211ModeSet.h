//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MODESET_H
#define __INET_IEEE80211MODESET_H

#include "inet/common/DelayedInitializer.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211ModeSet : public IPrintableObject, public cObject
{
  protected:
    class INET_API Entry {
      public:
        bool isMandatory;
        const IIeee80211Mode *mode;
    };

    struct EntryNetBitrateComparator {
        bool operator()(const Entry& left, const Entry& right) { return left.mode->getDataMode()->getNetBitrate() < right.mode->getDataMode()->getNetBitrate(); }
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

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Ieee80211ModeSet, name = " << name; }

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

    IIeee80211Mode *_getSlowestMode() const { return const_cast<IIeee80211Mode *>(getSlowestMode()); }
    IIeee80211Mode *_getFastestMode() const { return const_cast<IIeee80211Mode *>(getFastestMode()); }
    IIeee80211Mode *_getSlowestMandatoryMode() const { return const_cast<IIeee80211Mode *>(getSlowestMandatoryMode()); }
    IIeee80211Mode *_getFastestMandatoryMode() const { return const_cast<IIeee80211Mode *>(getFastestMandatoryMode()); }
};

} // namespace physicallayer
} // namespace inet

#endif

