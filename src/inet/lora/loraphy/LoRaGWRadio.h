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

#ifndef LORA_LORAGWRADIO_H_
#define LORA_LORAGWRADIO_H_

#include "inet/lora/loraphy/LoRaMedium.h"
#include "inet/lora/loraphy/LoRaReceiver.h"
#include "inet/lora/loraphy/LoRaReception.h"
#include "inet/lora/loraphy/LoRaTransmission.h"
#include "inet/lora/loraphy/LoRaTransmitter.h"
#include "inet/physicallayer/base/packetlevel/FlatRadioBase.h"
#include "inet/lora/lorabase/LoRaMacFrame_m.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/common/LayeredProtocolBase.h"

namespace inet {

namespace lora {
using namespace physicallayer;
class LoRaGWRadio : public FlatRadioBase {
private:
    void completeRadioModeSwitch(RadioMode newRadioMode);
protected:
    void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleSelfMessage(cMessage *message) override;
    virtual void handleUpperPacket(Packet *packet) override;
    void handleSignal(Signal *radioFrame) override;

    bool iAmTransmiting;
    virtual bool isTransmissionTimer(const cMessage *message) const;
    virtual void handleTransmissionTimer(cMessage *message) override;
    virtual void startTransmission(Packet *macFrame, IRadioSignal::SignalPart part) override;
    virtual void continueTransmission(cMessage *timer);
    virtual void endTransmission(cMessage *timer);

    virtual bool isReceptionTimer(const cMessage *message) const override;
    virtual void startReception(cMessage *timer, IRadioSignal::SignalPart part) override;
    virtual void continueReception(cMessage *timer) override;
    virtual void endReception(cMessage *timer) override;
    virtual void abortReception(cMessage *timer) override;


public:
    bool iAmGateway;

    std::list<cMessage *>concurrentReceptions;
    std::list<cMessage *>concurrentTransmissions;

    long LoRaGWRadioReceptionStarted_counter;
    long LoRaGWRadioReceptionFinishedCorrect_counter;
    simsignal_t LoRaGWRadioReceptionStarted;
    simsignal_t LoRaGWRadioReceptionFinishedCorrect;
};

}

}

#endif /* LORA_LORAGWRADIO_H_ */
