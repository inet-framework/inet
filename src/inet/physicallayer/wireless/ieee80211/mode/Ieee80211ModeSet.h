//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MODESET_H
#define __INET_IEEE80211MODESET_H

#include <array>
#include <set>

#include "inet/common/DelayedInitializer.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211ModeSet : public IPrintableObject, public cObject
{
  public:
    enum class PhyType {
        OFDM,
        HR_DSSS,
        ERP,
        HT,
        VHT,
    };

  protected:
    class INET_API Entry {
      public:
        bool isMandatory;
        const IIeee80211Mode *mode;
        bool isLegacyOperational = false;
    };

    struct EntryNetBitrateComparator {
        bool operator()(const Entry& left, const Entry& right) { return left.mode->getDataMode()->getNetBitrate() < right.mode->getDataMode()->getNetBitrate(); }
    };

  protected:
    std::string name;
    const std::vector<Entry> entries;
    const PhyType phyType;
    // PHY timing and contention parameters remain anchored to the explicitly
    // configured reference mode, even though entries are sorted by bitrate for lookup.
    const IIeee80211Mode *referenceMode;
    std::vector<const IIeee80211Mode *> legacyOperationalModes;
    std::array<bool, 77> htMcsSupported = {};
    std::array<bool, 77> htMcsMandatory = {};
    std::set<Hz> htSupportedChannelWidths;
    std::set<Hz> htShortGuardIntervalChannelWidths;
    bool htOperationSupported = false;

  public:
    static const DelayedInitializer<std::vector<Ieee80211ModeSet>> modeSets;

  protected:
    int findModeIndex(const IIeee80211Mode *mode) const;
    int getModeIndex(const IIeee80211Mode *mode) const;

  public:
    Ieee80211ModeSet(const char *name, const std::vector<Entry> entries, const IIeee80211Mode *referenceMode,
            PhyType phyType, bool htOperationSupported = false);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Ieee80211ModeSet, name = " << name; }

    const char *getName() const override { return name.c_str(); }

    int getNumModes() const { return entries.size(); }
    const IIeee80211Mode *getMode(int index) const { return entries[index].mode; }
    bool isMandatory(int index) const { return entries[index].isMandatory; }
    bool isHtOperationSupported() const { return htOperationSupported; }
    Hz getMaximumChannelWidth() const;
    int getMaximumNumberOfSpatialStreams() const;
    // The management policy advertises all explicitly eligible representable
    // legacy modes in deterministic mandatory-first order. Overflow is split
    // into the Extended Supported Rates element by management.
    const std::vector<const IIeee80211Mode *>& getLegacyOperationalModes() const { return legacyOperationalModes; }
    const IIeee80211Mode *getFastestLegacyOperationalMode() const;
    const std::array<bool, 77>& getHtMcsSupported() const { return htMcsSupported; }
    const std::array<bool, 77>& getHtMcsMandatory() const { return htMcsMandatory; }
    const std::set<Hz>& getHtSupportedChannelWidths() const { return htSupportedChannelWidths; }
    const std::set<Hz>& getHtShortGuardIntervalChannelWidths() const { return htShortGuardIntervalChannelWidths; }
    bool isHtShortGuardIntervalSupported(Hz bandwidth) const { return htShortGuardIntervalChannelWidths.count(bandwidth) != 0; }

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

    // PHY timing and contention policy remain anchored to the explicitly
    // configured reference mode, independent of bitrate sorting.
    PhyType getPhyType() const { return phyType; }
    const IIeee80211Mode *getReferenceMode() const { return referenceMode; }
    simtime_t getSifsTime() const { return referenceMode->getSifsTime(); }
    simtime_t getSlotTime() const { return referenceMode->getSlotTime(); }
    simtime_t getPhyRxStartDelay() const { return referenceMode->getPhyRxStartDelay(); }
    int getCwMin() const { return referenceMode->getLegacyCwMin(); }
    int getCwMax() const { return referenceMode->getLegacyCwMax(); }

    IIeee80211Mode *_getSlowestMode() const { return const_cast<IIeee80211Mode *>(getSlowestMode()); }
    IIeee80211Mode *_getFastestMode() const { return const_cast<IIeee80211Mode *>(getFastestMode()); }
    IIeee80211Mode *_getSlowestMandatoryMode() const { return const_cast<IIeee80211Mode *>(getSlowestMandatoryMode()); }
    IIeee80211Mode *_getFastestMandatoryMode() const { return const_cast<IIeee80211Mode *>(getFastestMandatoryMode()); }
};

} // namespace physicallayer
} // namespace inet

#endif
