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

#ifndef __INET_QOSCLASSIFIER_H
#define __INET_QOSCLASSIFIER_H

#include "inet/common/INETDefs.h"
#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

/**
 * This module classifies and assigns User Priority to packets.
 */
class INET_API QosClassifier : public cSimpleModule, public IProtocolRegistrationListener
{
  protected:
    int defaultUp;
    std::map<int, int> ipProtocolUpMap;
    std::map<int, int> udpPortUpMap;
    std::map<int, int> tcpPortUpMap;

    virtual int parseUserPriority(const char *text);
    virtual void parseUserPriorityMap(const char *text, std::map<int, int>& upMap);

    virtual int getUserPriority(cMessage *msg);

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

  public:
    QosClassifier() {}

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif

