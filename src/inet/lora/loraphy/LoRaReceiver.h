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

#ifndef LORAPHY_LORARECEIVER_H_
#define LORAPHY_LORARECEIVER_H_

#include "inet/lora/loraapp/SimpleLoRaApp.h"
#include "inet/lora/lorabase/LoRaGWMac.h"
#include "inet/lora/lorabase/LoRaMac.h"
#include "inet/lora/loraphy/LoRaRadio.h"
#include "inet/lora/loraphy/LoRaBandListening.h"
#include "inet/lora/loraphy/LoRaModulation.h"
#include "inet/lora/loraphy/LoRaReception.h"
#include "inet/lora/loraphy/LoRaTransmission.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/common/packetlevel/ReceptionResult.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"
#include "inet/physicallayer/common/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarSnir.h"
#include "LoRaRadioControlInfo_m.h"
#include "inet/physicallayer/base/packetlevel/FlatReceiverBase.h"

//based on Ieee802154UWBIRReceiver

namespace inet {

namespace lora {

using namespace physicallayer;

class INET_API LoRaReceiver : public FlatReceiverBase

{
private:
    W LoRaTP;
    Hz LoRaCF;
    int LoRaSF;
    Hz LoRaBW;
    double LoRaCR;

    double snirThreshold;

    bool iAmGateway;
    bool alohaChannelModel;

    W energyDetection;
    simsignal_t LoRaReceptionCollision;

    //statistics
    long numCollisions;
    long rcvBelowSensitivity;

public:
  LoRaReceiver();

  void initialize(int stage) override;
  void finish() override;
  virtual W getMinInterferencePower() const override { return W(NaN); }
  virtual W getMinReceptionPower() const override { return W(NaN); }

  virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;

  virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
  virtual bool computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const override;

  virtual Packet * computeReceivedPacket(const ISnir *snir, bool isReceptionSuccessful) const override;

  virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
  virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;

  //virtual ReceptionIndication *createReceptionIndication() const;

  //virtual const LoRaReceptionIndication *computeReceptionIndication(const ISnir *snir) const override;

  virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;

  virtual double getSNIRThreshold() const override { return snirThreshold; }

  virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const override;

  virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;

  W getSensitivity(const LoRaReception *loRaReception) const;

  bool isPacketCollided(const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const;

  virtual void setLoRaTP(W newTP) { LoRaTP = newTP; };
  virtual void setLoRaCF(Hz newCF) { LoRaCF = newCF; };
  virtual void setLoRaSF(int newSF) { LoRaSF = newSF; };
  virtual void setLoRaBW(Hz newBW) { LoRaBW = newBW; };
  virtual void setLoRaCR(double newCR) { LoRaCR = newCR; };

};

}

}

#endif /* LORAPHY_LORARECEIVER_H_ */
