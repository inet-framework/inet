//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE802154UWBIRRECEIVER_H
#define __INET_IEEE802154UWBIRRECEIVER_H

#include <vector>

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.h"
#include "inet/physicallayer/wireless/ieee802154/bitlevel/Ieee802154UwbIrMode.h"

namespace inet {

namespace physicallayer {

// This class was created by porting some C++ code from the DeciderUWBIRED class in MiXiM.
class INET_API Ieee802154UwbIrReceiver : public ReceiverBase
{
  protected:
    const Ieee802154UwbIrMode& cfg = Ieee802154UwbIrMode::cfg_mandatory_16M;

  protected:
    virtual std::vector<bool> *decode(const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const;
    virtual std::pair<double, double> integrateWindow(simtime_t_cref pNow, simtime_t_cref burst, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
    virtual bool computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const override;
    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

