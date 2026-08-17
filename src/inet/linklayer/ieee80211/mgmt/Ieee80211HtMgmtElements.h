//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211HTMGMTELEMENTS_H
#define __INET_IEEE80211HTMGMTELEMENTS_H

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211HtCapabilities.h"

namespace inet {
namespace ieee80211 {

inline Ieee80211HtCapabilitiesElement makeHtCapabilitiesElement(const Ieee80211HtCapabilities& capabilities)
{
    Ieee80211HtCapabilitiesElement element;
    element.ldpc = capabilities.ldpc;
    element.supportedChannelWidth40Mhz = capabilities.supportedChannelWidths.count(MHz(40)) != 0;
    element.greenfield = capabilities.greenfield;
    element.shortGi20 = capabilities.shortGi20;
    element.shortGi40 = capabilities.shortGi40;
    element.maxAmpduLengthExponent = capabilities.maxAmpduLengthExponent;
    for (int i = 0; i < 77; i++)
        element.rxMcsSupported[i] = capabilities.rxMcsSupported[i];
    element.txMcsSetDefined = capabilities.txMcsSetDefined;
    element.txRxMcsSetNotEqual = capabilities.txRxMcsSetNotEqual;
    element.txMaxNss = capabilities.txMaxNss;
    element.txUnequalModulation = capabilities.txUnequalModulation;
    if (element.txMcsSetDefined && !element.txRxMcsSetNotEqual) {
        for (int nss = 0; nss < 4; nss++) {
            int rxMaximum = -1;
            for (int mcs = 0; mcs < 8; mcs++)
                if (capabilities.rxMcsSupported[nss * 8 + mcs])
                    rxMaximum = mcs;
            if (capabilities.txMcsNss.maxMcsPerNss[nss] != rxMaximum)
                throw cRuntimeError("Equal HT Tx/Rx MCS Set does not match the Rx MCS bitmap");
        }
    }
    return element;
}

inline Ieee80211HtCapabilities makeHtCapabilities(const Ieee80211HtCapabilitiesElement& element)
{
    if (element.maxAmpduLengthExponent < 0 || element.maxAmpduLengthExponent > 3)
        throw cRuntimeError("Invalid Maximum A-MPDU Length Exponent: %d", element.maxAmpduLengthExponent);
    Ieee80211HtCapabilities capabilities;
    capabilities.supportedChannelWidths.insert(MHz(20));
    if (element.supportedChannelWidth40Mhz)
        capabilities.supportedChannelWidths.insert(MHz(40));
    capabilities.ldpc = element.ldpc;
    capabilities.greenfield = element.greenfield;
    capabilities.shortGi20 = element.shortGi20;
    capabilities.shortGi40 = element.shortGi40;
    capabilities.maxAmpduLengthExponent = element.maxAmpduLengthExponent;
    capabilities.txMcsSetDefined = element.txMcsSetDefined;
    capabilities.txRxMcsSetNotEqual = element.txRxMcsSetNotEqual;
    capabilities.txMaxNss = element.txMaxNss;
    capabilities.txUnequalModulation = element.txUnequalModulation;
    for (int i = 0; i < 77; i++)
        capabilities.rxMcsSupported[i] = element.rxMcsSupported[i];
    for (int nss = 0; nss < 4; nss++) {
        int rxMaximum = -1;
        for (int mcs = 0; mcs < 8; mcs++)
            if (element.rxMcsSupported[nss * 8 + mcs])
                rxMaximum = mcs;
        capabilities.txMcsNss.maxMcsPerNss[nss] = element.txMcsSetDefined && !element.txRxMcsSetNotEqual ? rxMaximum : -1;
    }
    return capabilities;
}

inline Ieee80211HtOperationElement makeHtOperationElement(const Ieee80211HtOperation& operation)
{
    Ieee80211HtOperationElement element;
    element.primaryChannel = operation.primaryChannel;
    element.secondaryChannelOffset = operation.secondaryChannelOffset;
    element.staChannelWidth40Mhz = operation.operatingChannelWidth == MHz(40);
    element.protectionMode = static_cast<int>(operation.protectionMode);
    for (int i = 0; i < 77; i++)
        element.basicMcsSupported[i] = operation.basicMcsSupported[i];
    return element;
}

inline Ieee80211HtOperation makeHtOperation(const Ieee80211HtOperationElement& element)
{
    if (element.secondaryChannelOffset == 2 || element.secondaryChannelOffset < 0 || element.secondaryChannelOffset > 3)
        throw cRuntimeError("Invalid HT Secondary Channel Offset: %d", element.secondaryChannelOffset);
    if (element.protectionMode < 0 || element.protectionMode > 3)
        throw cRuntimeError("Invalid HT Protection field: %d", element.protectionMode);
    Ieee80211HtOperation operation;
    operation.primaryChannel = element.primaryChannel;
    operation.secondaryChannelOffset = element.secondaryChannelOffset;
    operation.operatingChannelWidth = element.staChannelWidth40Mhz ? MHz(40) : MHz(20);
    operation.protectionMode = static_cast<Ieee80211HtProtectionMode>(element.protectionMode);
    for (int i = 0; i < 77; i++)
        operation.basicMcsSupported[i] = element.basicMcsSupported[i];
    return operation;
}

inline B getHtMgmtElementsLength(const Ptr<const Ieee80211MgmtFrame>& frame)
{
    B length(0);
    if (frame->getHtCapabilitiesPresent())
        length += B(28);
    if (frame->getHtOperationPresent())
        length += B(24);
    return length;
}

inline void setHtCapabilities(const Ptr<Ieee80211MgmtFrame>& frame, const Ieee80211HtCapabilities& capabilities)
{
    frame->setHtCapabilitiesPresent(true);
    frame->setHtCapabilities(makeHtCapabilitiesElement(capabilities));
}

inline void setHtOperation(const Ptr<Ieee80211MgmtFrame>& frame, const Ieee80211HtOperation& operation)
{
    frame->setHtOperationPresent(true);
    frame->setHtOperation(makeHtOperationElement(operation));
}

} // namespace ieee80211
} // namespace inet

#endif
