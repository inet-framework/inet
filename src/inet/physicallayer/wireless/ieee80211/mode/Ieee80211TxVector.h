// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211TXVECTOR_H
#define INET_IEEE80211TXVECTOR_H

#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet::physicallayer {

// Mode is the per-copy mode for NON_HT_DUPLICATE (21.3.10.12).
struct INET_API Ieee80211TxVector
{
    enum Format : int { NON_HT = 0, NON_HT_DUPLICATE = 1, HT_MIXED = 2, VHT = 3 };
    const IIeee80211Mode *mode = nullptr;
    Format format = Format::NON_HT;
    Hz bandwidth = Hz(NaN);
    bool singleMpdu = false;

    Ieee80211TxVector() = default;
    explicit Ieee80211TxVector(const IIeee80211Mode *mode) : mode(mode) {
        if (mode) {
            bandwidth = mode->getDataMode()->getBandwidth();
            format = mode->getVhtMcsIndex() >= 0 ? Format::VHT : mode->getHtMcsIndex() >= 0 ? Format::HT_MIXED : Format::NON_HT;
        }
    }
    const IIeee80211Mode *getMode() const { return mode; }
    int getFormat() const { return static_cast<int>(format); }
    Hz getBandwidth() const { return bandwidth; }
    bool getSingleMpdu() const { return singleMpdu; }
    bool operator==(const Ieee80211TxVector& other) const {
        return mode == other.mode && format == other.format && bandwidth == other.bandwidth && singleMpdu == other.singleMpdu;
    }
    bool operator!=(const Ieee80211TxVector& other) const { return !(*this == other); }
};

} // namespace inet::physicallayer
#endif
