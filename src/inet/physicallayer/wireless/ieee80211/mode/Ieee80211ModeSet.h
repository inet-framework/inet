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
    bool htGreenfieldSupported = false;
    bool htOperationSupported = false;
    // Entries are selectable modes; supportedEntries also contains immutable PHY capabilities needed for mandatory control responses.
    const std::vector<Entry> supportedEntries;
    const std::map<const IIeee80211Mode *, const IIeee80211Mode *> controlResponseModes;
    const std::map<const IIeee80211Mode *, const IIeee80211Mode *> htMixedControlResponseModes;
    const std::vector<Entry> nonHtControlResponseEntries;

  public:
    static OPP_THREAD_LOCAL const DelayedInitializer<std::vector<Ieee80211ModeSet>> modeSets;

  protected:
    static std::vector<Entry> completeHtGuardIntervalVariants(const char *name, const std::vector<Entry>& entries);
    int findModeIndex(const IIeee80211Mode *mode) const;
    int getModeIndex(const IIeee80211Mode *mode) const;
    static std::map<const IIeee80211Mode *, const IIeee80211Mode *> createControlResponseModes(const std::vector<Entry>& supportedEntries);
    static std::map<const IIeee80211Mode *, const IIeee80211Mode *> createHtMixedControlResponseModes(const std::vector<Entry>& supportedEntries);
    static std::vector<Entry> createNonHtControlResponseEntries(const std::vector<Entry>& supportedEntries);

  public:
    Ieee80211ModeSet(const char *name, const std::vector<Entry> entries, const IIeee80211Mode *referenceMode,
            PhyType phyType, bool htOperationSupported = false, const std::vector<Entry> supportedEntries = {});

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
    bool isHtGreenfieldSupported() const { return htGreenfieldSupported; }

    // containsMode() covers entries selectable as the persistent operating mode
    // (for example through Ieee80211Transmitter::setMode()). supportsMode()
    // additionally covers immutable PHY capabilities that may be selected per
    // packet through Ieee80211ModeReq, such as legacy control responses in a
    // 2.4 GHz HT profile.
    bool containsMode(const IIeee80211Mode *mode) const { return findModeIndex(mode) != -1; }
    bool supportsMode(const IIeee80211Mode *mode) const;
    bool getIsMandatory(const IIeee80211Mode *mode) const;
    // Finds a mode with the same PHY tuple as mode. Unlike findMode(), this
    // treats an absent guard interval (negative value) as an exact value.
    const IIeee80211Mode *findCompatibleMode(const IIeee80211Mode *mode) const;

    // Pointer lookup is intentionally strict. Use getControlResponseMode() for an explicitly requested response that needs HT-mixed translation.
    const IIeee80211Mode *findMode(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *getMode(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *findMode(bps bitrate, Hz bandwidth = Hz(NaN), int numSpatialStreams = -1, simtime_t guardInterval = -1) const;
    const IIeee80211Mode *findMode(bps minBitrate, bps maxBitrate, Hz bandwidth = Hz(NaN), int numSpatialStreams = -1, simtime_t guardInterval = -1) const;
    const IIeee80211Mode *getMode(bps bitrate, Hz bandwidth = Hz(NaN), int numSpatialStreams = -1, simtime_t guardInterval = -1) const;
    const IIeee80211Mode *getMode(bps minBitrate, bps maxBitrate, Hz bandwidth = Hz(NaN), int numSpatialStreams = -1, simtime_t guardInterval = -1) const;
    const IIeee80211Mode *getSlowestMode() const;
    const IIeee80211Mode *getFastestMode() const;
    const IIeee80211Mode *getSlowerMode(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *getFasterMode(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *getSlowestMandatoryMode() const;
    const IIeee80211Mode *getFastestMandatoryMode() const;
    const IIeee80211Mode *getMandatoryModeAtOrBelow(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *getSlowerMandatoryMode(const IIeee80211Mode *mode) const;
    const IIeee80211Mode *getFasterMandatoryMode(const IIeee80211Mode *mode) const;

    // Automatic responses follow IEEE 802.11-2024 10.6.6.5.3/10.6.6.5.7.
    // A configured CTS mode must be selectable and, for an HT RTS, must be HT;
    // it is translated only to the corresponding HT-mixed response. An explicitly
    // configured HT mode is a deliberate override and may bypass those response
    // constraints. A legacy configured CTS rate is rejected by rate-selection
    // initialization, while this API keeps the direct HT/legacy combination fatal.
    const IIeee80211Mode *getControlResponseMode(const IIeee80211Mode *mode, const IIeee80211Mode *configuredMode = nullptr) const;
    const IIeee80211Mode *getMandatoryControlResponseMode(const IIeee80211Mode *mode) const;
    // 2.4 GHz HT modes are converted to non-HT responses. With mandatory=false,
    // the input bitrate is used as a ceiling for selecting that non-HT mode.
    const IIeee80211Mode *getNonHtControlResponseMode(const IIeee80211Mode *mode, bool mandatory = true) const;
    // Returns the HT-mixed equivalent mode for an HT mode, or nullptr if not an HT mode or not mapped.
    const IIeee80211Mode *findHtMixedMode(const IIeee80211Mode *mode) const;

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
