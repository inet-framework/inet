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

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/linklayer/ethernet/switch/IMacAddressTable.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet {

/**
 * Base class for Stp and Rstp.
 */
class INET_API StpBase : public OperationalBase, public cListener
{
public:
    enum BdpuType { BPDU_TYPE_CONFIG = 0x00,
        BPDU_TYPE_TCN = 0x80,
        BPDU_TYPE_RSTP = 0x02 };

    enum ProtocolVersion { PROTO_VERSION_STP = 0,
        PROTO_VERSION_RSTP = 2,
        PROTO_VERSION_MSTP = 3,
        PROTO_VERSION_SPB = 4 };

  protected:
    bool visualize = false;    // if true it visualize the spanning tree
    std::string colorLinkEnabled = "";
    std::string colorLinkDisabled = "";
    std::string colorRootBridge = "";

    unsigned int numPorts = 0;    // number of ports

    unsigned int bridgePriority = 0;    // bridge's priority
    MacAddress bridgeAddress;    // bridge's MAC address

    simtime_t maxAge;
    simtime_t helloTime;
    simtime_t forwardDelay;

    cModule *switchModule = nullptr;
    IMacAddressTable *macTable = nullptr;
    IInterfaceTable *ifTable = nullptr;

  public:
    StpBase();
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void start();
    virtual void stop();

    /**
     * @brief Adds effects to be represented by Tkenv. Colors the link black if forwarding parameter is true
     * and the port to which the link is connected to is also forwarding, otherwise colors the link gray.
     */
    virtual void colorLink(InterfaceEntry *ie, bool forwarding) const;

    /**
     * @brief Adds effects to be represented by Tkenv. Inactive links are colored grey.
     * Shows port role, state. Marks root switch.
     */
    virtual void refreshDisplay() const override;

    /**
     * @brief Obtains the root interface ID.
     * @return The root gate interface ID or -1 if there is no root interface.
     */
    virtual int getRootInterfaceId() const;

    /**
     * @brief Gets Ieee8021dInterfaceData for interface ID.
     * @return The port's Ieee8021dInterfaceData, or throws error if it doesn't exist.
     */
    Ieee8021dInterfaceData *getPortInterfaceData(unsigned int interfaceId);
    const Ieee8021dInterfaceData *getPortInterfaceData(unsigned int interfaceId) const { return const_cast<StpBase *>(this)->getPortInterfaceData(interfaceId); }

    /**
     * @brief Gets InterfaceEntry for interface ID.
     * @return The port's InterfaceEntry, throws error if it doesn't exist.
     */
    InterfaceEntry *getPortInterfaceEntry(unsigned int interfaceId);
};

} // namespace inet

#endif // ifndef __INET_STPBASE_H

