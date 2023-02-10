//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154NARROWBANDSCALARRECEIVER_H
#define __INET_IEEE802154NARROWBANDSCALARRECEIVER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee802154NarrowbandScalarReceiver : public FlatReceiverBase
{
  protected:
    W minInterferencePower;

  public:
    Ieee802154NarrowbandScalarReceiver();

    void initialize(int stage) override;

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual W getMinInterferencePower() const override { return minInterferencePower; }
};

} // namespace physicallayer

} // namespace inet

#endif

