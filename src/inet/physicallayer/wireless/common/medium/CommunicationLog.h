//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

