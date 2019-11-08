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
#include "inet/lora/loraphy/LoRaMedium.h"
#include "inet/lora/lorabase/LoRaMacFrame_m.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/physicallayer/common/packetlevel/Interference.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/contract/packetlevel/IErrorModel.h"


namespace inet {

namespace lora {

Define_Module(LoRaMedium);
LoRaMedium::LoRaMedium() : RadioMedium()
{
}

LoRaMedium::~LoRaMedium()
{
}

bool LoRaMedium::matchesMacAddressFilter(const IRadio *radio, const Packet *packet) const
{
    const auto &chunk = packet->peekAtFront<Chunk>();
    const auto & loraHeader = dynamicPtrCast<const lora::LoRaMacFrame>(chunk);
    if (loraHeader == nullptr)
        return false;
    MacAddress address = MacAddress(loraHeader->getReceiverAddress().getInt());
    if (address.isBroadcast() || address.isMulticast())
        return true;

    cModule *host = getContainingNode(check_and_cast<const cModule *>(radio));
    IInterfaceTable *interfaceTable = check_and_cast<IInterfaceTable *>(host->getSubmodule("interfaceTable"));
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        const InterfaceEntry *interface = interfaceTable->getInterface(i);
        if (interface && interface->getMacAddress() == address)
            return true;
    }
    return false;
}


const IReceptionResult *LoRaMedium::getReceptionResult(const IRadio *radio, const IListening *listening, const ITransmission *transmission) const
{
    cacheResultGetCount++;
    const IReceptionResult *result = communicationCache->getCachedReceptionResult(radio, transmission);
    if (result)
        cacheResultHitCount++;
    else {
        result = computeReceptionResult(radio, listening, transmission);

        auto pkt = const_cast<Packet *>(result->getPacket());
        if (!pkt->findTag<SnirInd>()) {
            const ISnir *snir = getSNIR(radio, transmission);
            auto snirInd = pkt->addTagIfAbsent<SnirInd>();
            snirInd->setMinimumSnir(snir->getMin());
            snirInd->setMaximumSnir(snir->getMax());
        }
        if (!pkt->findTag<ErrorRateInd>()) {
            auto errorModel = dynamic_cast<IErrorModel *>(getSubmodule("errorModel"));
            const ISnir *snir = getSNIR(radio, transmission);
            auto errorRateInd = pkt->addTagIfAbsent<ErrorRateInd>(); // TODO: should be done  setPacketErrorRate(packetModel->getPER());
            errorRateInd->setPacketErrorRate(errorModel ? errorModel->computePacketErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
            errorRateInd->setBitErrorRate(errorModel ? errorModel->computeBitErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
            errorRateInd->setSymbolErrorRate(errorModel ? errorModel->computeSymbolErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
        }

        communicationCache->setCachedReceptionResult(radio, transmission, result);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << result->getReception() << " and results in " << result << endl;
    }
    return result;
}

}
}
