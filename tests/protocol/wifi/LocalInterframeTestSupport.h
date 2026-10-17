// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_LOCALINTERFRAMETESTSUPPORT_H
#define __INET_LOCALINTERFRAMETESTSUPPORT_H

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitterAnalogModel.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.h"

namespace inet::ieee80211 {

// The test waveform changes physical and analog duration together.
class LocalInterframeTransmitterBase : public physicallayer::Ieee80211Transmitter {
  protected:
    virtual bool omitsSignalExtension(const Packet *packet) const = 0;
  public:
    const physicallayer::ITransmission *createTransmission(const physicallayer::IRadio *radio,
            const Packet *packet, simtime_t start) const override {
        auto selected = computeTransmissionMode(packet);
        if (!omitsSignalExtension(packet) || selected->getHtMcsIndex() < 0 || selected->getSignalExtensionTime() == SIMTIME_ZERO)
            return Ieee80211Transmitter::createTransmission(radio, packet, start);
        auto phy = physicallayer::Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        auto duration = selected->getDuration(B(phy->getLengthField())) - selected->getSignalExtensionTime();
        auto preamble = selected->getPreambleMode()->getDuration();
        auto header = selected->getHeaderMode()->getDuration();
        auto data = duration - preamble - header;
        auto mobility = radio->getAntenna()->getMobility();
        auto position = mobility->getCurrentPosition();
        auto orientation = mobility->getCurrentAngularPosition();
        auto analog = getAnalogModel()->createAnalogModel(preamble, header, data, centerFrequency,
                selected->getDataMode()->getBandwidth(), computeTransmissionPower(packet));
        return new physicallayer::Ieee80211Transmission(radio, packet, start, start + duration, preamble, header, data,
                position, position, orientation, orientation, nullptr, nullptr, nullptr, nullptr,
                analog, selected, computeTransmissionChannel(packet));
    }
};

}
#endif
