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

#ifndef ANSA_NETWORKLAYER_CLNS_CLNSCONTROLINFO_H_
#define ANSA_NETWORKLAYER_CLNS_CLNSCONTROLINFO_H_


#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/contract/clns/CLNSControInfo_m.h"

namespace inet {

class INET_API CLNSControlInfo : public CLNSControlInfo_Base, public INetworkProtocolControlInfo
  {
    private:
      void copy(const CLNSControlInfo& other) {  }
      void clean();

    public:
      CLNSControlInfo() : CLNSControlInfo_Base() {}
      virtual ~CLNSControlInfo();
      CLNSControlInfo(const CLNSControlInfo& other) : CLNSControlInfo_Base(other) {copy(other);}
      CLNSControlInfo& operator=(const CLNSControlInfo& other) {if (this==&other) return *this; CLNSControlInfo_Base::operator=(other); copy(other); return *this;}
      virtual CLNSControlInfo *dup() const override { return new CLNSControlInfo(*this);}
//      virtual IPv4ControlInfo *dup() const override { return new IPv4ControlInfo(*this); }
      // ADD CODE HERE to redefine and implement pure virtual functions from CLNSControlInfo_Base


      virtual short getTransportProtocol() const override { return 0; }
      virtual void setTransportProtocol(short protocol) override {  }
      virtual L3Address getSourceAddress() const override { return L3Address(srcAddr); }
      virtual void setSourceAddress(const L3Address& address) override { srcAddr = address.toCLNS(); }
      virtual L3Address getDestinationAddress() const override { return L3Address(destAddr); }
      virtual void setDestinationAddress(const L3Address& address) override { destAddr = address.toCLNS(); }
      virtual int getInterfaceId() const override { return CLNSControlInfo_Base::getInterfaceId(); }
      virtual void setInterfaceId(int interfaceId) override { CLNSControlInfo_Base::setInterfaceId(interfaceId); }
      virtual short getHopLimit() const override { return getTimeToLive(); }
      virtual void setHopLimit(short hopLimit) override { setTimeToLive(hopLimit); }
  };

} /* namespace inet */

#endif /* ANSA_NETWORKLAYER_CLNS_CLNSCONTROLINFO_H_ */
