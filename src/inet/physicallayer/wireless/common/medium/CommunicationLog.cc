//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/medium/CommunicationLog.h"

#include "inet/physicallayer/wireless/common/radio/packetlevel/Radio.h"

namespace inet {
namespace physicallayer {

void CommunicationLog::open()
{
    output.open(getActiveSimulationOrEnvir()->getConfig()->substituteVariables("${resultdir}/${configname}-${runnumber}.tlog"));
}

void CommunicationLog::close()
{
    output.close();
}

void CommunicationLog::writeTransmission(const IRadio *transmitter, const IWirelessSignal *signal)
{
    const ITransmission *transmission = signal->getTransmission();
    const Radio *transmitterRadio = check_and_cast<const Radio *>(transmitter);
    output << "T " << transmitterRadio->getFullPath() << " " << transmitterRadio->getId() << " "
           << "M " << check_and_cast<const WirelessSignal *>(signal)->getName() << " " << transmission->getId() << " "
           << "S " << transmission->getStartTime() << " " << transmission->getStartPosition() << " -> "
           << "E " << transmission->getEndTime() << " " << transmission->getEndPosition() << endl;
}

void CommunicationLog::writeReception(const IRadio *receiver, const IWirelessSignal *signal)
{
    const ITransmission *transmission = signal->getTransmission();
    const IReception *reception = signal->getReception();
    const Radio *receiverRadio = check_and_cast<const Radio *>(receiver);
    output << "R " << receiverRadio->getFullPath() << " " << reception->getReceiver()->getId() << " "
           << "M " << check_and_cast<const WirelessSignal *>(signal)->getName() << " " << transmission->getId() << " "
           << "S " << reception->getStartTime() << " " << reception->getStartPosition() << " -> "
           << "E " << reception->getEndTime() << " " << reception->getEndPosition() << endl;
}

} // namespace physicallayer
} // namespace inet

