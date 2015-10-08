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

#ifndef __INET_IEEE80211MACRX_H
#define __INET_IEEE80211MACRX_H

#include "IRx.h"
#include "MacPlugin.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class ITx;
class IUpperMac;

class Rx : public cSimpleModule, public IRx
{
    protected:
        cMessage *endNavTimer = nullptr;
        IRadio::ReceptionState receptionState = IRadio::RECEPTION_STATE_UNDEFINED;
        IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;
        bool mediumFree;  // cached state
        MACAddress address;
        ITx *tx = nullptr;
        IUpperMac *upperMac = nullptr;

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual void setNav(simtime_t navInterval);
        virtual bool isFcsOk(Ieee80211Frame *frame) const;
        virtual void recomputeMediumFree();

    public:
        Rx();
        ~Rx();

        virtual void setAddress(const MACAddress& address) override { this->address = address; }
        virtual void receptionStateChanged(IRadio::ReceptionState newReceptionState) override;
        virtual void transmissionStateChanged(IRadio::TransmissionState transmissionState) override;
        virtual bool isMediumFree() const override { return mediumFree; }
        virtual void lowerFrameReceived(Ieee80211Frame *frame) override;

};

} // namespace ieee80211
} // namespace inet

#endif

