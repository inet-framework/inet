//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_COMMUNICATIONLOG_H
#define __INET_COMMUNICATIONLOG_H

#include <fstream>

#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IWirelessSignal.h"

namespace inet {
namespace physicallayer {

class INET_API CommunicationLog
{
  protected:
    std::ofstream output;

  public:
    virtual void open();
    virtual void close();
    virtual void writeTransmission(const IRadio *transmitter, const IWirelessSignal *signal);
    virtual void writeReception(const IRadio *receiver, const IWirelessSignal *signal);
};

} // namespace physicallayer
} // namespace inet

#endif

