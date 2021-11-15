//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_MACADDRESSTABLECONFIGURATOR_H
#define __INET_MACADDRESSTABLECONFIGURATOR_H

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

class INET_API MacAddressTableConfigurator : public NetworkConfiguratorBase, public cListener
{
  protected:
    std::map<int, cValueArray *> configurations;

  protected:
    virtual void initialize(int stage) override;

    /**
     * Computes the network configuration for all nodes in the network.
     * The result of the computation is only stored in the configurator.
     */
    virtual void computeConfiguration();
    virtual void extendConfiguration(Node *destinationNode, Interface *destinationInterface, MacAddress macAddress);

    virtual void computeMacAddressTables();
    virtual void configureMacAddressTables() const;

  public:
    virtual ~MacAddressTableConfigurator();
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

