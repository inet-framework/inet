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

#ifndef IEEE80211MACRX_H_
#define IEEE80211MACRX_H_

#include "IIeee80211MacRx.h"
#include "Ieee80211MacPlugin.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class IIeee80211MacTx;
class IIeee80211UpperMac;

class Ieee80211MacRx : public cSimpleModule, public IIeee80211MacRx
{
    protected:
        cMessage *endNavTimer = nullptr;
        IRadio::ReceptionState receptionState = IRadio::RECEPTION_STATE_UNDEFINED;
        IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;
        MACAddress address;
        IIeee80211MacTx *tx = nullptr;
        IIeee80211UpperMac *upperMac = nullptr;

    protected:
        void initialize();
        void handleMessage(cMessage *msg);
        void setNav(simtime_t navInterval);
        bool isFcsOk(Ieee80211Frame *frame) const;

    public:
        Ieee80211MacRx();
        ~Ieee80211MacRx();

        virtual void setAddress(const MACAddress& address) override { this->address = address; }
        virtual void receptionStateChanged(IRadio::ReceptionState newReceptionState) override;
        virtual void transmissionStateChanged(IRadio::TransmissionState transmissionState) override;
        virtual bool isMediumFree() const override;
        virtual void lowerFrameReceived(Ieee80211Frame *frame) override;

};

}

} /* namespace inet */

#endif /* IEEE80211MACRECEPTION_H_ */
