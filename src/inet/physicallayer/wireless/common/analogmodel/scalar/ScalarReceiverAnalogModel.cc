//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarReceiverAnalogModel.h"

namespace inet {
namespace physicallayer {

Define_Module(ScalarReceiverAnalogModel);

void ScalarReceiverAnalogModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));
        sensitivity = mW(math::dBmW2mW(par("sensitivity")));
    }
}

IListening *ScalarReceiverAnalogModel::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, centerFrequency, bandwidth);
}

bool ScalarReceiverAnalogModel::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const ScalarReceptionAnalogModel *analogModel = check_and_cast<const ScalarReceptionAnalogModel *>(reception->getAnalogModel());
    if (bandListening->getCenterFrequency() != analogModel->getCenterFrequency() || bandListening->getBandwidth() < analogModel->getBandwidth()) {
        EV_DEBUG << "Computing whether reception is possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else {
        W minReceptionPower = analogModel->getPower();
        ASSERT(W(0.0) <= minReceptionPower);
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing whether reception is possible" << EV_FIELD(minReceptionPower) << EV_FIELD(sensitivity) << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

} // namespace physicallayer
} // namespace inet

