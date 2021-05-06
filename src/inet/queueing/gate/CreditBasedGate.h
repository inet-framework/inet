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

#ifndef __INET_CREDITBASEDGATE_H
#define __INET_CREDITBASEDGATE_H

#include "inet/queueing/base/PacketGateBase.h"

namespace inet {
namespace queueing {

class INET_API CreditBasedGate : public PacketGateBase, public cListener
{
  public:
    static simsignal_t currentCreditChangedSignal;

  protected:
    // parameters
    double idleCreditGainRate = NaN;
    double transmitCreditSpendRate = NaN;
    double transmitCreditLimit = NaN;
    double minCredit = NaN;
    double maxCredit = NaN;

    // state
    double currentCredit = NaN;
    double currentCreditGainRate = NaN;
    double lastCurrentCreditEmitted = NaN;
    simtime_t lastCurrentCreditEmittedTime;

    cMessage *changeTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void processPacket(Packet *packet) override;
    virtual void updateCurrentCredit();
    virtual void emitCurrentCredit();
    virtual void scheduleChangeTimer();
    virtual void processChangeTimer();

  public:
    virtual ~CreditBasedGate() { cancelAndDelete(changeTimer); }

    virtual const char *resolveDirective(char directive) const override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace queueing
} // namespace inet

#endif

