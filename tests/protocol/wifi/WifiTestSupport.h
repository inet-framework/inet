//
// WiFi conformance suite -- shared test support.
//
// The single place the INET 802.11 PHY-mode + tag headers are #included. Provides
// C++ predicates that read the transmitted frame's Ieee80211ModeReq tag (an opaque
// `const IIeee80211Mode *` the PacketFilter string engine cannot follow) and cast it
// to the concrete per-generation mode class to assert PHY rate / MCS / bandwidth /
// spatial-stream count. Used from a test via:
//
//   .match([](const MatchContext& c){ return isVht(c.event, MHz(80), 1); })
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
#ifndef __INET_PROTOCOLTEST_WIFI_WIFITESTSUPPORT_H
#define __INET_PROTOCOLTEST_WIFI_WIFITESTSUPPORT_H

#include "ProtocolTest.h"

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211DsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HrDsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ErpOfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace protocoltest {

using namespace inet::physicallayer;
using namespace inet::units::values;

// The PHY transmission mode of a frame, read off its Ieee80211ModeReq tag (or nullptr).
inline const IIeee80211Mode *txMode(const PacketEvent& e)
{
    if (!e.packet)
        return nullptr;
    auto req = e.packet->findTag<Ieee80211ModeReq>();
    return req != nullptr ? req->getMode() : nullptr;
}

// --- per-PHY-family predicates (the dynamic_cast is what the string engine cannot do) ---

// Legacy DSSS (802.11-1997, 1/2 Mbps).
inline bool isDsss(const PacketEvent& e)
{
    return dynamic_cast<const Ieee80211DsssMode *>(txMode(e)) != nullptr;
}

// HR/DSSS-CCK (802.11b, 5.5/11 Mbps).
inline bool isHrDsss(const PacketEvent& e)
{
    return dynamic_cast<const Ieee80211HrDsssMode *>(txMode(e)) != nullptr;
}

// ERP-OFDM (802.11g). Checked before plain OFDM since it derives from it.
inline bool isErpOfdm(const PacketEvent& e)
{
    return dynamic_cast<const Ieee80211ErpOfdmMode *>(txMode(e)) != nullptr;
}

// Plain OFDM (802.11a) -- explicitly not the ERP-OFDM subclass.
inline bool isOfdm(const PacketEvent& e)
{
    auto m = txMode(e);
    return dynamic_cast<const Ieee80211OfdmMode *>(m) != nullptr
           && dynamic_cast<const Ieee80211ErpOfdmMode *>(m) == nullptr;
}

// HT (802.11n) with at least minSs spatial streams.
inline bool isHt(const PacketEvent& e, int minSs = 1)
{
    auto m = dynamic_cast<const Ieee80211HtMode *>(txMode(e));
    return m != nullptr && m->getDataMode()->getNumberOfSpatialStreams() >= minSs;
}

// VHT (802.11ac) at the given channel bandwidth with at least minSs spatial streams.
inline bool isVht(const PacketEvent& e, Hz bandwidth, int minSs = 1)
{
    auto m = dynamic_cast<const Ieee80211VhtMode *>(txMode(e));
    return m != nullptr
           && m->getDataMode()->getBandwidth() == bandwidth
           && m->getDataMode()->getNumberOfSpatialStreams() >= minSs;
}

// Net data bitrate of the frame's mode, in Mbps (-1 if no mode tag).
inline double txNetBitrateMbps(const PacketEvent& e)
{
    auto m = txMode(e);
    return m != nullptr ? m->getDataMode()->getNetBitrate().get() / 1e6 : -1.0;
}

} // namespace protocoltest
} // namespace inet

#endif
