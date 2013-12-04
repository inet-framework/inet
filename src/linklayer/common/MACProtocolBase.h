//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
//

#ifndef __INET_MACPROTOCOLBASE_H_
#define __INET_MACPROTOCOLBASE_H_

#include "LayeredProtocolBase.h"

class INET_API MACProtocolBase : public LayeredProtocolBase
{
  public:
    /** @brief Gate ids */
    //@{
    int upperLayerInGateId;
    int upperLayerOutGateId;
    int lowerLayerInGateId;
    int lowerLayerOutGateId;
    //@}

  protected:
    MACProtocolBase();

    virtual void initialize(int stage);

    virtual void sendUp(cMessage* message);

    virtual void sendDown(cMessage* message);

    virtual bool isUpperMessage(cMessage* message);

    virtual bool isLowerMessage(cMessage* message);
};

#endif
