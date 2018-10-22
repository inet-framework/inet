//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_EXTINTERFACE_H
#define __INET_EXTINTERFACE_H

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

class INET_API ExtInterface : public InterfaceEntry
{
  protected:
    virtual void configureInterface();
    virtual void copyNetworkInterfaceConfigurationFromExt();
    virtual void copyNetworkInterfaceConfigurationToExt();
    virtual void copyNetworkAddressFromExt();
    virtual void copyNetworkAddressToExt();

  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif // ifndef __INET_EXTINTERFACE_H

