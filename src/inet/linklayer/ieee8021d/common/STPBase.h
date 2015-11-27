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
// Author: Zsolt Prontvai
//

#ifndef __INET_STPBASE_H
#define __INET_STPBASE_H

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ethernet/switch/IMACAddressTable.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"

namespace inet {

/**
 * Base class for STP and RSTP.
 */
class INET_API STPBase : public cSimpleModule, public ILifecycle, public cListener
{
  protected:
    bool visualize = false;    // if true it visualize the spanning tree
    bool isOperational = false;    // for lifecycle
    unsigned int numPorts = 0;    // number of ports

    unsigned int bridgePriority = 0;    // bridge's priority
    MACAddress bridgeAddress;    // bridge's MAC address

    simtime_t maxAge;
    simtime_t helloTime;
    simtime_t forwardDelay;

    cModule *switchModule = nullptr;
    IMACAddressTable *macTable = nullptr;
    IInterfaceTable *ifTable = nullptr;
    InterfaceEntry *ie = nullptr;

  public:
    STPBase();
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void start();
    virtual void stop();

    /**
     * @brief Adds effects to be represented by Tkenv. Colors the link black if forwarding parameter is true
     * and the port to which the link is connected to is also forwarding, otherwise colors the link gray.
     */
    virtual void colorLink(unsigned int i, bool forwarding);

    /**
     * @brief Adds effects to be represented by Tkenv. Inactive links are colored grey.
     * Shows port role, state. Marks root switch.
     */
    virtual void updateDisplay();

    /**
     * @brief Obtains the root gate index.
     * @return The root gate index or -1 if there is no root gate.
     */
    virtual int getRootIndex();

    /**
     * @brief Gets Ieee8021dInterfaceData for port number.
     * @return The port's Ieee8021dInterfaceData, or nullptr if it doesn't exist.
     */
    Ieee8021dInterfaceData *getPortInterfaceData(unsigned int portNum);

    /**
     * @brief Gets InterfaceEntry for port number.
     * @return The port's InterfaceEntry, or nullptr if it doesn't exist.
     */
    InterfaceEntry *getPortInterfaceEntry(unsigned int portNum);

    /*
     * Returns the first non-loopback interface.
     */
    virtual InterfaceEntry *chooseInterface();
};

} // namespace inet

#endif // ifndef __INET_STPBASE_H

