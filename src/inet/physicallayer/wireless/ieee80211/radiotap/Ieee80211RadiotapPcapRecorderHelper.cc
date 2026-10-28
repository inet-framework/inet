//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/radiotap/Ieee80211RadiotapPcapRecorderHelper.h"

#include <algorithm>
#include <cmath>

#include "inet/common/FcsInd_m.h"
#include "inet/common/INETMath.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/radiotap/Ieee80211RadiotapHeaderSerializer.h"

namespace inet {

namespace physicallayer {

Register_Class(Ieee80211RadiotapPcapRecorderHelper);

namespace {

bool isPositive(double value)
{
    return std::isfinite(value) && value > 0;
}

// Gives a power as the signed dBm octet of a Radiotap field.
int8_t getRadiotapDbm(double milliwatts)
{
    return (int8_t)std::clamp(std::round(math::mW2dBmW(milliwatts)), -128.0, 127.0);
}

// Gives the bandwidth code of the Radiotap VHT field, or -1 for another bandwidth.
int getRadiotapVhtBandwidth(Hz bandwidth)
{
    switch ((int)std::round(MHz(bandwidth).get())) {
        case 20: return 0;
        case 40: return 1;
        case 80: return 4;
        case 160: return 11;
        default: return -1;
    }
}

} // namespace

PcapLinkType Ieee80211RadiotapPcapRecorderHelper::protocolToLinkType(const Protocol *protocol) const
{
    return *protocol == Protocol::ieee80211Mac ? LINKTYPE_IEEE802_11_RADIOTAP : LINKTYPE_INVALID;
}

bool Ieee80211RadiotapPcapRecorderHelper::matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const
{
    // the Radiotap header exists only in the converted copy of the frame
    return false;
}

Packet *Ieee80211RadiotapPcapRecorderHelper::tryConvertToLinkType(const Packet *packet, b frontOffset, b backOffset, PcapLinkType pcapLinkType, const Protocol *protocol, Direction direction) const
{
    if (*protocol != Protocol::ieee80211Mac || pcapLinkType != LINKTYPE_IEEE802_11_RADIOTAP)
        return nullptr;
    auto newPacket = packet->dup();
    newPacket->eraseAtFront(frontOffset); // the PHY header of a sent frame
    newPacket->eraseAtBack(backOffset); // the PHY tail and padding of a sent frame
    newPacket->insertAtFront(createHeader(packet, direction));
    newPacket->addTagIfAbsent<PacketProtocolTag>()->set(&Protocol::ieee80211Radiotap);
    return newPacket;
}

Ptr<Ieee80211RadiotapHeader> Ieee80211RadiotapPcapRecorderHelper::createHeader(const Packet *packet, Direction direction) const
{
    auto header = makeShared<Ieee80211RadiotapHeader>();
    uint32_t present = 1 << RADIOTAP_FLAGS;
    // each 802.11 frame of INET ends with an FCS trailer, and its serializer needs fcsMode = "computed",
    // so each recorded frame has a correct FCS; only an imported frame can have no FCS
    auto fcsInd = packet->findTag<FcsInd>();
    uint8_t flags = fcsInd == nullptr || fcsInd->getHasFcs() ? RADIOTAP_FLAG_FCS_AT_END : 0;
    // the receiver of a frame with bit errors finds a wrong frame check sequence
    if (packet->hasBitError())
        flags |= RADIOTAP_FLAG_BAD_FCS;
    const IIeee80211Mode *mode = nullptr;
    const Ieee80211Channel *channel = nullptr;
    if (direction == DIRECTION_INBOUND) {
        if (auto modeInd = packet->findTag<Ieee80211ModeInd>())
            mode = modeInd->getMode();
        if (auto channelInd = packet->findTag<Ieee80211ChannelInd>())
            channel = channelInd->getChannel();
        if (auto signalTimeInd = packet->findTag<SignalTimeInd>()) {
            // INET has no TSF timer, so the simulation time of the first bit stands in for it
            present |= 1 << RADIOTAP_TSFT;
            header->setTsft(signalTimeInd->getStartTime().inUnit(SIMTIME_US));
        }
        auto signalPowerInd = packet->findTag<SignalPowerInd>();
        double signalPower = signalPowerInd != nullptr ? mW(signalPowerInd->getPower()).get() : NaN;
        if (isPositive(signalPower)) {
            present |= 1 << RADIOTAP_DBM_ANTSIGNAL;
            header->setAntennaSignal(getRadiotapDbm(signalPower));
            // the noise plus the interference at the minimum SNIR, which the error models use by default
            auto snirInd = packet->findTag<SnirInd>();
            if (snirInd != nullptr && isPositive(snirInd->getMinimumSnir())) {
                present |= 1 << RADIOTAP_DBM_ANTNOISE;
                header->setAntennaNoise(getRadiotapDbm(signalPower / snirInd->getMinimumSnir()));
            }
        }
    }
    else {
        if (auto modeReq = packet->findTag<Ieee80211ModeReq>())
            mode = modeReq->getMode();
        if (auto channelReq = packet->findTag<Ieee80211ChannelReq>())
            channel = channelReq->getChannel();
        auto signalPowerReq = packet->findTag<SignalPowerReq>();
        double transmitPower = signalPowerReq != nullptr ? mW(signalPowerReq->getPower()).get() : NaN;
        if (isPositive(transmitPower)) {
            present |= 1 << RADIOTAP_DBM_TX_POWER;
            header->setTxPower(getRadiotapDbm(transmitPower));
        }
    }
    auto modulationClass = mode != nullptr ? mode->getModulationClass() : IIeee80211Mode::ModulationClass::UNKNOWN;
    if (mode != nullptr) {
        if (mode->getLegacyPreambleType() == IIeee80211Mode::PreambleType::SHORT)
            flags |= RADIOTAP_FLAG_SHORT_PREAMBLE;
        if (modulationClass == IIeee80211Mode::ModulationClass::HT) {
            present |= 1 << RADIOTAP_MCS;
            // INET models neither STBC nor extension spatial streams, and it codes with BCC
            header->setMcsKnown(RADIOTAP_MCS_KNOWN_BANDWIDTH | RADIOTAP_MCS_KNOWN_INDEX | RADIOTAP_MCS_KNOWN_GUARD_INTERVAL |
                    RADIOTAP_MCS_KNOWN_FEC | RADIOTAP_MCS_KNOWN_STBC | RADIOTAP_MCS_KNOWN_EXTENSION_STREAMS);
            uint8_t mcsFlags = 0;
            if (mode->getDataMode()->getBandwidth() >= MHz(40))
                mcsFlags |= RADIOTAP_MCS_FLAG_BANDWIDTH_40;
            if (mode->isHtShortGuardInterval())
                mcsFlags |= RADIOTAP_MCS_FLAG_SHORT_GUARD_INTERVAL;
            header->setMcsFlags(mcsFlags);
            header->setMcsIndex(mode->getHtMcsIndex());
        }
        else if (modulationClass == IIeee80211Mode::ModulationClass::VHT) {
            present |= 1 << RADIOTAP_VHT;
            uint16_t vhtKnown = RADIOTAP_VHT_KNOWN_STBC | RADIOTAP_VHT_KNOWN_GUARD_INTERVAL;
            int bandwidth = getRadiotapVhtBandwidth(mode->getDataMode()->getBandwidth());
            if (bandwidth != -1) {
                vhtKnown |= RADIOTAP_VHT_KNOWN_BANDWIDTH;
                header->setVhtBandwidth(bandwidth);
            }
            header->setVhtKnown(vhtKnown);
            header->setVhtFlags(mode->isVhtShortGuardInterval() ? RADIOTAP_VHT_FLAG_SHORT_GUARD_INTERVAL : 0);
            header->setVhtMcsNss(0, (mode->getVhtMcsIndex() << 4) | mode->getDataMode()->getNumberOfSpatialStreams());
        }
        else {
            double rate = mode->getDataMode()->getNetBitrate().get() / 500000;
            if (rate >= 1 && rate <= 255 && rate == std::trunc(rate)) {
                present |= 1 << RADIOTAP_RATE;
                header->setRate(rate);
            }
        }
    }
    if (channel != nullptr) {
        double frequency = MHz(channel->getCenterFrequency()).get();
        present |= 1 << RADIOTAP_CHANNEL;
        header->setChannelFrequency(std::round(frequency));
        uint16_t channelFlags = frequency < 3000 ? RADIOTAP_CHANNEL_2GHZ : frequency < 6000 ? RADIOTAP_CHANNEL_5GHZ : 0;
        if (modulationClass == IIeee80211Mode::ModulationClass::DSSS_HRDSSS)
            channelFlags |= RADIOTAP_CHANNEL_CCK;
        else if (modulationClass != IIeee80211Mode::ModulationClass::UNKNOWN)
            channelFlags |= RADIOTAP_CHANNEL_OFDM;
        header->setChannelFlags(channelFlags);
    }
    header->setFlags(flags);
    header->setPresent(present);
    header->setChunkLength(computeIeee80211RadiotapHeaderLength(present));
    return header;
}

} // namespace physicallayer

} // namespace inet
