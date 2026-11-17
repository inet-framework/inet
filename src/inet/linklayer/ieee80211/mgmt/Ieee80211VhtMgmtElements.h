// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211VHTMGMTELEMENTS_H
#define INET_IEEE80211VHTMGMTELEMENTS_H
#include "inet/linklayer/ieee80211/mib/Ieee80211VhtCapabilities.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"

namespace inet::ieee80211 {

inline Ieee80211VhtCapabilitiesElement makeVhtCapabilitiesElement(const Ieee80211VhtCapabilities& c)
{
    Ieee80211VhtCapabilitiesElement e;
    int maximum = c.maxMpduLength == 3895 ? 0 : c.maxMpduLength == 7991 ? 1 : c.maxMpduLength == 11454 ? 2 : -1;
    if (maximum < 0 || c.maxAmpduLengthExponent < 0 || c.maxAmpduLengthExponent > 7)
        throw cRuntimeError("Invalid VHT receive size limit");
    e.capabilitiesInfo = maximum | (c.width160 ? 1 << 2 : 0) | (c.shortGi80 ? 1 << 5 : 0)
        | (c.shortGi160 ? 1 << 6 : 0) | (c.maxAmpduLengthExponent << 23);
    e.rxMcsMap = encodeVhtMcsMap(c.rxMcs); e.txMcsMap = encodeVhtMcsMap(c.txMcs);
    e.rxHighestSupportedDataRate = c.rxHighestRate; e.txHighestSupportedDataRate = c.txHighestRate;
    return e;
}

inline Ieee80211VhtCapabilities makeVhtCapabilities(const Ieee80211VhtCapabilitiesElement& e)
{
    int lengthCode = e.capabilitiesInfo & 3;
    int widthCode = (e.capabilitiesInfo >> 2) & 3;
    // Extended NSS and80+80 operation are outside the selected contiguous uniform-NSS profile.
    if (lengthCode == 3 || widthCode > 1 || (e.capabilitiesInfo >> 30)
            || (e.rxHighestSupportedDataRate & 0xE000) || (e.txHighestSupportedDataRate & 0xE000))
        throw cRuntimeError("Unsupported VHT capability/extended-NSS encoding");
    Ieee80211VhtCapabilities c;
    c.maxMpduLength = lengthCode == 0 ? 3895 : lengthCode == 1 ? 7991 : 11454;
    c.width160 = widthCode == 1;
    c.shortGi80 = e.capabilitiesInfo & (1 << 5); c.shortGi160 = e.capabilitiesInfo & (1 << 6);
    c.maxAmpduLengthExponent = (e.capabilitiesInfo >> 23) & 7;
    c.rxMcs = decodeVhtMcsMap(e.rxMcsMap); c.txMcs = decodeVhtMcsMap(e.txMcsMap);
    c.rxHighestRate = e.rxHighestSupportedDataRate; c.txHighestRate = e.txHighestSupportedDataRate;
    return c;
}

inline Ieee80211VhtOperationElement makeVhtOperationElement(const Ieee80211VhtOperation& o)
{
    Ieee80211VhtOperationElement e;
    e.channelWidth = o.width >= MHz(80) ? 1 : 0;
    e.centerFrequencySegment0 = o.center0; e.centerFrequencySegment1 = o.center1; e.basicMcsMap = o.basicMcsMap;
    return e;
}

} // namespace inet::ieee80211
#endif
