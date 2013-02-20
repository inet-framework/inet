//
// Copyright (C) 2012 Opensim Ltd.
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

#ifndef __INET_MODULEPATHADDRESSPOLICY_H
#define __INET_MODULEPATHADDRESSPOLICY_H

#include "INETDefs.h"
#include "IAddressPolicy.h"
#include "ModulePathAddress.h"
#include "GenericNetworkProtocolControlInfo.h"

class INET_API ModulePathAddressPolicy : public IAddressPolicy
{
    public:
        static ModulePathAddressPolicy INSTANCE;

    public:
        ModulePathAddressPolicy() { }
        virtual ~ModulePathAddressPolicy() { }

        virtual int getMaxPrefixLength() const { return 0; }
        virtual Address getLinkLocalManetRoutersMulticastAddress() const { return ModulePathAddress(-109); } // TODO: constant
        virtual Address getLinkLocalRIPRoutersMulticastAddress() const { return ModulePathAddress(-9); } // TODO: constant
        virtual INetworkProtocolControlInfo * createNetworkProtocolControlInfo() const { return new GenericNetworkProtocolControlInfo(); }
};

#endif
