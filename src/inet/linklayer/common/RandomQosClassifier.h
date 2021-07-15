//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_RANDOMQOSCLASSIFIER_H
#define __INET_RANDOMQOSCLASSIFIER_H

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

/**
 * A QoS classifier that assigns a random User Priority. This is useful
 * for testing purposes.
 */
class INET_API RandomQosClassifier : public cSimpleModule, public TransparentProtocolRegistrationListener
{
  protected:
    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;

    virtual std::vector<cGate *> getRegistrationForwardingGates(cGate *gate) override;

  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif

