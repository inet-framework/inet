//
// This class defines a Hypervisor
//
// This class links the four main manager from hypervisor with the node. IT is a cover with the main functionality of link
// and unlink virtual machines.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

#ifndef HYPERVISOR_H_
#define HYPERVISOR_H_

#include "inet/icancloud/Architecture/Node/NodeVL/NodeVL.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_CPUManager/H_CPUManager_Base.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_MemoryManager/H_MemoryManager_Base.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/Managers/H_NETManager_Base.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/H_StorageManager_Base.h"
#include <omnetpp.h>

namespace inet {

namespace icancloud {


class NodeVL;

class Hypervisor : public cSimpleModule{

protected:
    H_CPUManager_Base* cpuM;
    H_MemoryManager_Base* memM;
    H_NETManager_Base* netM;
    H_StorageManager_Base* storageM;;
public:

    virtual ~Hypervisor();

    virtual void initialize() override;
    

    void finish() override;

    void handleMessage(cMessage* msg) override;

    void setVM (cGate** iGateCPU, cGate** oGateCPU, cGate* iGateMemI,
                cGate* oGateMemI, cGate* iGateMemO, cGate* oGateMemO, cGate* iGateNet,
                cGate* oGateNet, cGate* iGateStorage, cGate* oGateStorage, int numCores, string virtualIP,
                int requestedMemoryKB, int requestedStorageKB, int uId, int pId);

    void freeResources (int uId, int pId);

    double getMemoryOverhead(){return memM->getMemoryOverhead();};
};
} // namespace icancloud
} // namespace inet

#endif /* HYPERVISOR_H_ */
