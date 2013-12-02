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

#ifndef __INET_NETWORKLAYERBASE_H_
#define __INET_NETWORKLAYERBASE_H_

#include "LayeredProtocolBase.h"
#include "NodeOperations.h"
#include "ProtocolMap.h"
#include "IInterfaceTable.h"

class INET_API NetworkProtocolBase : public LayeredProtocolBase
{
  protected:
    ProtocolMapping protocolMapping;
    IInterfaceTable* interfaceTable;

  protected:
    NetworkProtocolBase();

    virtual void initialize(int stage);

    virtual void handleUpperCommand(cMessage* message);

    virtual void sendUp(cMessage* message, int transportProtocol);

    virtual void sendDown(cMessage* message, int interfaceId = -1);

    virtual bool isUpperMessage(cMessage* message);

    virtual bool isLowerMessage(cMessage* message);
};

#endif
