//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/common/packetlevel/CommunicationLog.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"

namespace inet {
namespace physicallayer {

void CommunicationLog::open()
{
    output.open(getEnvir()->getConfig()->substituteVariables("${resultdir}/${configname}-${runnumber}.tlog"));
}

void CommunicationLog::close()
{
    output.close();
}

void CommunicationLog::writeTransmission(const IRadio *transmitter, const ISignal *signal)
{
    const ITransmission *transmission = signal->getTransmission();
    const Radio *transmitterRadio = check_and_cast<const Radio *>(transmitter);
    output << "T " << transmitterRadio->getFullPath() << " " << transmitterRadio->getId() << " "
           << "M " << check_and_cast<const Signal *>(signal)->getName() << " " << transmission->getId() << " "
           << "S " << transmission->getStartTime() << " " << transmission->getStartPosition() << " -> "
           << "E " << transmission->getEndTime() << " " << transmission->getEndPosition() << endl;
}

void CommunicationLog::writeReception(const IRadio *receiver, const ISignal *signal)
{
    const ITransmission *transmission = signal->getTransmission();
    const IReception *reception = signal->getReception();
    const Radio *receiverRadio = check_and_cast<const Radio *>(receiver);
    output << "R " << receiverRadio->getFullPath() << " " << reception->getReceiver()->getId() << " "
           << "M " << check_and_cast<const Signal *>(signal)->getName() << " " << transmission->getId() << " "
           << "S " << reception->getStartTime() << " " << reception->getStartPosition() << " -> "
           << "E " << reception->getEndTime() << " " << reception->getEndPosition() << endl;
}

} // namespace physicallayer
} // namespace inet

