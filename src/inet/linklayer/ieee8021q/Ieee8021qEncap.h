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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IEEE8021QENCAP_H
#define __INET_IEEE8021QENCAP_H

#include "inet/common/INETDefs.h"

namespace inet {

// TODO: explain how it should work when the packet API is extended with chunk insertion into the middle of a packet
class INET_API Ieee8021qEncap : public cSimpleModule
{
  protected:
    const char *vlanTagType = nullptr;
    std::vector<int> inboundVlanIdFilter;
    std::map<int, int> inboundVlanIdMap;
    std::vector<int> outboundVlanIdFilter;
    std::map<int, int> outboundVlanIdMap;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual Ieee8021qHeader *findVlanTag(const Ptr<EthernetMacHeader>& ethernetMacHeader);
    virtual Ieee8021qHeader *addVlanTag(const Ptr<EthernetMacHeader>& ethernetMacHeader);
    virtual Ieee8021qHeader *removeVlanTag(const Ptr<EthernetMacHeader>& ethernetMacHeader);
    virtual void parseParameters(const char *filterParameterName, const char *mapParameterName, std::vector<int>& vlanIdFilter, std::map<int, int>& vlanIdMap);
    virtual void processPacket(Packet *packet, std::vector<int>& vlanIdFilter, std::map<int, int>& vlanIdMap, cGate *gate);
};

} // namespace inet

#endif // ifndef __INET_IEEE8021QENCAP_H

