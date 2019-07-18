//
// Copyright (C) Alfonso Ariza 2011 Universidad de Malaga
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

#ifndef AddressModule_H_
#define AddressModule_H_
#include <vector>
#include <map>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

class AddressModule : public cNoncopyableOwnedObject,cListener
{
    protected:

        bool isInitialized;
        bool emitSignal;
        std::vector<L3Address> destAddresses;
        std::vector<int> destModuleId;

        L3Address chosedAddresses;
        int index;
        static simsignal_t changeAddressSignalInit;
        static simsignal_t changeAddressSignalDelete;
        L3Address myAddress;
        virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details);
    public:
        virtual void initModule(bool mode); // mode = true, use signals, useIpV6 = true use Ipv6 address
        virtual L3Address getAddress(int val = -1);
        bool isEmpty() {return destAddresses.empty();}
        unsigned int getNumAddress() const {return destAddresses.size();}
        virtual L3Address choseNewAddress() {return choseNewAddress(index);}
        virtual L3Address choseNewAddress(int &index);
        virtual int choseNewModule();
        AddressModule();
        virtual ~AddressModule();
        virtual bool isInit() const {return isInitialized;}
        virtual void rebuildAddressList();
        virtual int getModule(int val = -1);
};

}

#endif /* AddressModule_H_ */
