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

#ifndef LORAPHY_LORATRANSMITTER_H_
#define LORAPHY_LORATRANSMITTER_H_

#include "inet/lora/loraphy/LoRaRadio.h"
#include "inet/lora/loraphy/LoRaModulation.h"
#include "inet/lora/loraphy/LoRaTransmission.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmitterBase.h"
#include "inet/lora/lorabase/LoRaMacFrame_m.h"

namespace inet {

namespace physicallayer {

class LoRaTransmitter : public FlatTransmitterBase {
    public:
        LoRaTransmitter();
        virtual void initialize(int stage) override;
        virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
        virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;

    private:

        bool iAmGateway;

        simsignal_t LoRaTransmissionCreated;

};

}

}

#endif /* LORAPHY_LORATRANSMITTER_H_ */
