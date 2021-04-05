//
// Copyright (C) 2020 OpenSim Ltd.
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

#ifndef __INET_CARRIERBASEDLIFETIMER_H
#define __INET_CARRIERBASEDLIFETIMER_H

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {

using namespace inet::queueing;

class INET_API CarrierBasedLifeTimer : public cSimpleModule, cListener
{
  protected:
    NetworkInterface *networkInterface = nullptr;
    IPacketCollection *packetCollection = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void clearCollection();

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

