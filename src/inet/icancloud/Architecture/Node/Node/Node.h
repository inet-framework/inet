
// Module that implements a Node.
//
// This class models Computing node.
//
// @author Gabriel González Castañé
// @date 2014-12-12

#ifndef _NODE_H_
#define _NODE_H_

#include "inet/icancloud/Architecture/Node/AbstractNode.h"
#include "inet/icancloud/EnergySystem/EnergyMeter/EnergyMeterController/EnergyMeterController.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/icancloud/Management/MachinesStructure/ElementType.h"
#include "inet/icancloud/Management/DataCenterManagement/AbstractDCManager.h"

namespace inet {

namespace icancloud {


class Node : public AbstractNode{

protected:

		 bool energyMeter;                                  // true = energy meter active. false otherwise.
		 bool storageNode;                                  // true = the node is a storage node.

 		 EnergyMeterController* energyMeterPtr;             // Pointer to the energy meter

         virtual void initialize(int stage) override;
         virtual int numInitStages() const override { return NUM_INIT_STAGES; }
 	    
		 virtual void finish() override;

public:

		//------------------ To operate with the state of the node -------------
            virtual void turnOn () override;                                                     // Change the node state to on.
            virtual void turnOff () override;                                                    // Change the node state to off.

        //------------------ To operate check the energy of the node -------------

            double getCPUInstantConsumption(int partIndex = -1){return energyMeterPtr->cpuInstantConsumption(energyMeterPtr->getCurrentCPUState(),partIndex);};
            double getCPUEnergyConsumed(int partIndex = -1){return energyMeterPtr->getCPUEnergyConsumed(partIndex);};
            double getMemoryInstantConsumption(int partIndex = -1){return energyMeterPtr->getMemoryInstantConsumption(energyMeterPtr->getCurrentMemoryState(),partIndex);};
            double getMemoryEnergyConsumed(int partIndex = -1){return energyMeterPtr->getMemoryEnergyConsumed(partIndex);};
            double getNICInstantConsumption(int partIndex = -1){return energyMeterPtr->getNICInstantConsumption(energyMeterPtr->getCurrentNICState(),partIndex);};
            double getNICEnergyConsumed(int partIndex = -1){return energyMeterPtr->getNICEnergyConsumed(partIndex);};
            double getStorageInstantConsumption(int partIndex = -1){return energyMeterPtr->getStorageInstantConsumption(energyMeterPtr->getCurrentStorageState(),partIndex);};
            double getStorageEnergyConsumed(int partIndex = -1){return energyMeterPtr->getStorageEnergyConsumed(partIndex);};
            double getPSUConsumptionLoss(){return energyMeterPtr->getPSUConsumptionLoss();};
            double getPSUEnergyLoss(){return energyMeterPtr->getPSUEnergyLoss();};
            double getInstantConsumption(){return energyMeterPtr->getNodeInstantConsumption();};
            double getEnergyConsumed(){return energyMeterPtr->getNodeEnergyConsumed();};
            double getSubsystemsConsumption(){return energyMeterPtr->getNodeSubsystemsConsumption();};
            void resetEnergyHistory (){energyMeterPtr->resetNodeEnergy();};

         // ------------------------- Memorization ------------------------------
            // Get component names
            string getCPUName(){return energyMeterPtr->getCPUName();};
            string getMemoryName(){return energyMeterPtr->getMemoryName();};
            string getStorageName(){return energyMeterPtr->getStorageName();};
            string getNetworkName(){return energyMeterPtr->getNetworkName();};
            void loadMemo(MemoSupport* cpu,MemoSupport* memory,MemoSupport* storage,MemoSupport* network){energyMeterPtr->loadMemo(cpu,memory,storage,network);};

       virtual void setManager(icancloud_Base* manager) override;

       virtual void notifyManager(Packet *) override;

       virtual void initNode () override;

};

} // namespace icancloud
} // namespace inet

#endif /* _NODE_H_ */
