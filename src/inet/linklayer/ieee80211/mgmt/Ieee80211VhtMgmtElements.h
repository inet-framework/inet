// Copyright (C) 2026 INET Framework contributors
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_IEEE80211VHTMGMTELEMENTS_H
#define __INET_IEEE80211VHTMGMTELEMENTS_H
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211VhtCapabilities.h"

namespace inet {
namespace ieee80211 {

inline bool decodeVhtCapabilities(const Ptr<const Ieee80211MgmtFrame>& frame, Ieee80211VhtCapabilities& capabilities)
{
    capabilities = Ieee80211VhtCapabilities();
    if (!frame->getVhtCapabilitiesPresent() || frame->isIncorrect() || frame->isIncomplete())
        return false;
    const auto& element = frame->getVhtCapabilities();
    // 80+80 support is not modeled. Value 2 also supports contiguous 160 MHz.
    if (element.supportedChannelWidthSet < 0 || element.supportedChannelWidthSet > 2)
        return false;
    capabilities.supported160Mhz = element.supportedChannelWidthSet != 0;
    capabilities.shortGi80 = element.shortGi80;
    capabilities.shortGi160 = element.shortGi160;
    capabilities.rxHighestLongGiRateMbps = element.rxHighestLongGiRateMbps;
    capabilities.txHighestLongGiRateMbps = element.txHighestLongGiRateMbps;
    if (capabilities.rxHighestLongGiRateMbps < 0 || capabilities.rxHighestLongGiRateMbps > 8191 ||
            capabilities.txHighestLongGiRateMbps < 0 || capabilities.txHighestLongGiRateMbps > 8191)
        return false;
    for (int i = 0; i < 8; i++) {
        capabilities.rxMaxMcs[i] = element.rxMaxMcs[i];
        capabilities.txMaxMcs[i] = element.txMaxMcs[i];
    }
    // VHT SGI at 20/40 MHz is conveyed by HT Capabilities, not the VHT IE.
    if (frame->getHtCapabilitiesPresent()) {
        capabilities.shortGi20 = frame->getHtCapabilities().shortGi20;
        capabilities.shortGi40 = frame->getHtCapabilities().shortGi40;
    }
    return isValidVhtMcsMap(capabilities.rxMaxMcs) && isValidVhtMcsMap(capabilities.txMaxMcs);
}

inline bool decodeVhtOperation(const Ptr<const Ieee80211MgmtFrame>& frame, Ieee80211VhtOperation& operation)
{
    operation = Ieee80211VhtOperation();
    if (!frame->getVhtOperationPresent() || frame->isIncorrect() || frame->isIncomplete())
        return false;
    const auto& element = frame->getVhtOperation();
    if (element.centerFrequencySegment0 < 0 || element.centerFrequencySegment0 > 255 ||
            element.centerFrequencySegment1 < 0 || element.centerFrequencySegment1 > 255)
        return false;
    operation.centerFrequencySegment0 = element.centerFrequencySegment0;
    operation.centerFrequencySegment1 = element.centerFrequencySegment1;
    // IEEE Std 802.11-2024, 9.4.2.157: revised signaling uses width 1 for
    // contiguous 160 MHz with segment centers separated by eight channels.
    if (element.channelWidth == 0)
        operation.channelWidth = frame->getHtOperationPresent() && frame->getHtOperation().staChannelWidth40Mhz ? MHz(40) : MHz(20);
    else if (element.channelWidth == 1 && element.centerFrequencySegment0 != 0 && element.centerFrequencySegment1 == 0)
        operation.channelWidth = MHz(80);
    else if ((element.channelWidth == 2 && element.centerFrequencySegment0 != 0) ||
            (element.channelWidth == 1 && element.centerFrequencySegment0 != 0 && element.centerFrequencySegment1 != 0 && std::abs(element.centerFrequencySegment0 - element.centerFrequencySegment1) == 8))
        operation.channelWidth = MHz(160);
    else
        return false; // Unsupported 80+80 or invalid width/center encoding.
    for (int i = 0; i < 8; i++) {
        int value = element.basicMaxMcs[i];
        if (value != -1 && value != 7 && value != 8 && value != 9)
            return false;
        operation.basicMaxMcs[i] = value;
    }
    return true;
}

inline void setVhtCapabilities(const Ptr<Ieee80211MgmtFrame>& frame, const Ieee80211VhtCapabilities& capabilities)
{
    Ieee80211VhtCapabilitiesElement element;
    element.supportedChannelWidthSet = capabilities.supported160Mhz ? 1 : 0;
    element.shortGi80 = capabilities.shortGi80;
    element.shortGi160 = capabilities.shortGi160;
    element.rxHighestLongGiRateMbps = capabilities.rxHighestLongGiRateMbps;
    element.txHighestLongGiRateMbps = capabilities.txHighestLongGiRateMbps;
    for (int i = 0; i < 8; i++) {
        element.rxMaxMcs[i] = capabilities.rxMaxMcs[i];
        element.txMaxMcs[i] = capabilities.txMaxMcs[i];
    }
    frame->setVhtCapabilities(element);
    frame->setVhtCapabilitiesPresent(true);
}

inline void setVhtOperation(const Ptr<Ieee80211MgmtFrame>& frame, const Ieee80211VhtOperation& operation)
{
    Ieee80211VhtOperationElement element;
    element.channelWidth = operation.channelWidth <= MHz(40) ? 0 : 1;
    element.centerFrequencySegment0 = operation.centerFrequencySegment0;
    element.centerFrequencySegment1 = operation.centerFrequencySegment1;
    for (int i = 0; i < 8; i++)
        element.basicMaxMcs[i] = operation.basicMaxMcs[i];
    frame->setVhtOperation(element);
    frame->setVhtOperationPresent(true);
}

inline B getVhtMgmtElementsLength(const Ptr<const Ieee80211MgmtFrame>& frame)
{
    return B((frame->getVhtCapabilitiesPresent() ? 14 : 0) + (frame->getVhtOperationPresent() ? 7 : 0));
}

} // namespace ieee80211
} // namespace inet
#endif
