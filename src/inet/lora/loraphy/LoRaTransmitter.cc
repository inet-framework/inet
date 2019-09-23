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

#include "inet/lora/loraphy/LoRaTransmitter.h"

#include "inet/lora/loraphy/LoRaModulation.h"
#include "inet/lora/loraphy/LoraMode.h"
//#include "inet/lora/lorabase/LoRaMacFrame_m.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarTransmission.h"
#include "inet/lora/loraphy/LoRaPhyPreamble_m.h"
#include "inet/lora/lorabase/LoRaTagInfo_m.h"


namespace inet {

namespace lora {

Define_Module(LoRaTransmitter);

LoRaTransmitter::LoRaTransmitter() :
    FlatTransmitterBase()
{
}

void LoRaTransmitter::initialize(int stage)
{
    TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        preambleDuration = 0.001; //par("preambleDuration");

        bitrate = bps(par("bitrate"));
        power = W(par("power"));
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));
        LoRaTransmissionCreated = registerSignal("LoRaTransmissionCreated");
        std::string dataRate = par("dataRate").stdstringValue();

        if(strcmp(getParentModule()->getClassName(), "inet::lora::LoRaGWRadio") == 0)
        {
            iAmGateway = true;
        }
        else iAmGateway = false;
    }
}

std::ostream& LoRaTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "LoRaTransmitter";
    return FlatTransmitterBase::printToStream(stream, level);
}




const ITransmission *LoRaTransmitter::createTransmission(const IRadio *transmitter, const Packet *macFrame, const simtime_t startTime) const
{
    const LoraMode *mode = nullptr;

    const auto &frame = macFrame->peekAtFront<LoRaPhyPreamble>();

    if ((frame->getCenterFrequency() > MHz(400) && frame->getCenterFrequency() < MHz(500)) || (frame->getCenterFrequency() > MHz(800) && frame->getCenterFrequency() < MHz(900))) {
        for (int i = 0; i < LoraCompliantModes::LoraModeEuTotal; i++) {
            if (LoraCompliantModes::LoraModeEu[i]->getSpreadingFator() == frame->getSpreadFactor() && frame->getBandwidth() == LoraCompliantModes::LoraModeEu[i]->getBandwidth()) {
                mode = (LoraCompliantModes::LoraModeEu[i]);
                break;
            }
        }
    }
    else if (frame->getCenterFrequency() > MHz(800) && frame->getCenterFrequency() < MHz(900)) {
        for (int i = 0; i < LoraCompliantModes::LoraModeUsaTotal; i++) {
            if (LoraCompliantModes::LoraModeEu[i]->getSpreadingFator() == frame->getSpreadFactor() && frame->getBandwidth() == LoraCompliantModes::LoraModeEu[i]->getBandwidth()) {
                mode = (LoraCompliantModes::LoraModeUsa[i]);
                break;
            }
        }
    }

    if (mode == nullptr)
        throw cRuntimeError("Invalid lora mode CF %f, BW %f SF %f ", frame->getCenterFrequency(), frame->getBandwidth(), frame->getSpreadFactor());

    if (macFrame->getBitLength() > mode->getMpduMaxLength())
        throw cRuntimeError("to big"); // should discard or reduce the packet?

    LoRaModulation *modulation = dynamic_cast<LoRaModulation *>(const_cast<IModulation *>(mode->getModulation()));
    if (modulation == nullptr)
        throw cRuntimeError("error modulation type");

    const simtime_t duration = mode->getDuration(b(macFrame->getBitLength()));
    const simtime_t Tpayload = mode->getPayloadLength(b(macFrame->getBitLength()));
    const simtime_t Theader = mode->getHeaderLength(b(macFrame->getBitLength()));
    const simtime_t Tpreamble = mode->getPreambleLength();

    const simtime_t endTime = startTime + duration;
    //bps transmissionBitrate = modulation->getbitRate();
    //Hz transmissionBandwidth = modulation->getBandwith();
    W transmissionPower = computeTransmissionPower(macFrame);

    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const Quaternion startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion endOrientation = mobility->getCurrentAngularPosition();

    //auto *controlInfo = dynamic_cast<TransmissionRequest *>(macFrame->getControlInfo());
    //W transmissionPower = controlInfo && !std::isnan(controlInfo->getPower().get()) ? controlInfo->getPower() : power;
    //bps transmissionBitrate = controlInfo && !std::isnan(controlInfo->getBitrate().get()) ? controlInfo->getBitrate() : bitrate;
    const_cast<LoRaTransmitter* >(this)->emit(LoRaTransmissionCreated, true);

    /*
    int nPreamble = 8;
    simtime_t Tsym = (pow(2, frame->getLoRaSF()))/(frame->getLoRaBW().get()/1000);
    simtime_t Tpreamble = (nPreamble + 4.25) * Tsym / 1000;

    //preambleDuration = Tpreamble;
    int payloadBytes = 0;
    if(iAmGateway) payloadBytes = 15;
    else payloadBytes = 20;
    int payloadSymbNb = 8 + math::max(ceil((8*payloadBytes - 4*frame->getLoRaSF() + 28 + 16 - 20*0)/(4*(frame->getLoRaSF()-2*0)))*(frame->getLoRaCR() + 4), 0);

    simtime_t Theader = 0.5 * (8+payloadSymbNb) * Tsym / 1000;
    simtime_t Tpayload = 0.5 * (8+payloadSymbNb) * Tsym / 1000;


    const simtime_t duration = Tpreamble + Theader + Tpayload;
    */
    // check size
    if (macFrame->getBitLength() > mode->getMpduMaxLength())
        throw cRuntimeError("to big"); // should discard or reduce the packet?

    if(!iAmGateway) {
        LoRaRadio *radio = check_and_cast<LoRaRadio *>(getParentModule());
        radio->setCurrentTxPower(transmissionPower.get());
    }
    return new LoRaTransmission(transmitter,
            macFrame,
            startTime,
            endTime,
            Tpreamble,
            Theader,
            Tpayload,
            startPosition,
            endPosition,
            startOrientation,
            endOrientation,
            transmissionPower,
            frame->getCenterFrequency(),
            frame->getSpreadFactor(),
            frame->getBandwidth(),
            frame->getCodeRendundance());
}

}

}
