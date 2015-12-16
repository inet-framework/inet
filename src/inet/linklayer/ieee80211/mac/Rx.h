//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#ifndef __INET_RX_H
#define __INET_RX_H

#include "IRx.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class IUpperMac;
class ITx;
class IContention;
class IStatistics;

/**
 * The default implementation of IRx.
 */
class INET_API Rx : public cSimpleModule, public IRx
{
    protected:
        IUpperMac *upperMac = nullptr;
        IContention **contention = nullptr; // nullptr-terminated pointer array
        IStatistics *statistics = nullptr;

        MACAddress address;
        cMessage *endNavTimer = nullptr;
        IRadio::ReceptionState receptionState = IRadio::RECEPTION_STATE_UNDEFINED;
        IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;
        IRadioSignal::SignalPart receivedPart = IRadioSignal::SIGNAL_PART_NONE;
        bool mediumFree = true;  // cached state

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void setOrExtendNav(simtime_t navInterval);
        virtual bool isFcsOk(Ieee80211Frame *frame) const;
        virtual void recomputeMediumFree();
        virtual void updateDisplayString();

    public:
        Rx();
        ~Rx();

        virtual void setAddress(const MACAddress& address) override { this->address = address; }
        virtual bool isReceptionInProgress() const override;
        virtual bool isMediumFree() const override { return mediumFree; }
        virtual void receptionStateChanged(IRadio::ReceptionState newReceptionState) override;
        virtual void transmissionStateChanged(IRadio::TransmissionState transmissionState) override;
        virtual void receivedSignalPartChanged(IRadioSignal::SignalPart part) override;
        virtual void lowerFrameReceived(Ieee80211Frame *frame) override;
        virtual void frameTransmitted(simtime_t durationField) override;
};

} // namespace ieee80211
} // namespace inet

#endif

