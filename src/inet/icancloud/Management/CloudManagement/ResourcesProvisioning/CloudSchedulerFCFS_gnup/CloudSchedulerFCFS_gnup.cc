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

#include "CloudSchedulerFCFS_gnup.h"

namespace inet {

namespace icancloud {


Define_Module(CloudSchedulerFCFS_gnup);

ICCLog csdccont3dAg;
ICCLog csdccont3D;
ICCLog csdcCont;
ICCLog csdcTot;
const string OUT_DIR="results";

void CloudSchedulerFCFS_gnup::initialize(int stage) {

    AbstractCloudManager::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        struct tm * timeinfo;
        minimum_percent_storageNode_free = 0.0;
        printNodePowerConsumption = false;
        printNodeEnergyConsumed = false;
        printComponentsPowerConsumption = false;
        printComponentsEnergyConsumed = false;
        printDataCenterPowerConsumption = false;
        printDataCenterEnergyConsumed = false;

        maximum_number_of_processes_per_node = par("numberOfVMperNode");

        dc_EnergyConsumed = 0.0;

        // define the files for get the results

        time_t rawtime;

        // Continue with the file creation.
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        sprintf(log_name1,
                "%s/dataCenter_total_%04d-%02d-%02d_%02d:%02d:%02d.dat",
                OUT_DIR.c_str(), timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
                timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
                timeinfo->tm_sec);
        csdcTot.Open(log_name1, par("outputCompression").boolValue());

        sprintf(log_name2,
                "%s/dataCenter_cont3DAggregated_%04d-%02d-%02d_%02d:%02d:%02d.dat",
                OUT_DIR.c_str(), timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
                timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
                timeinfo->tm_sec);
        csdccont3dAg.Open(log_name2, par("outputCompression").boolValue());

        sprintf(log_name3,
                "%s/dataCenter_cont3D_%04d-%02d-%02d_%02d:%02d:%02d.txt",
                OUT_DIR.c_str(), timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
                timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
                timeinfo->tm_sec);
        csdccont3D.Open(log_name3, par("outputCompression").boolValue());

        sprintf(log_name4,
                "%s/dataCenter_cont_%04d-%02d-%02d_%02d:%02d:%02d.txt",
                OUT_DIR.c_str(), timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
                timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
                timeinfo->tm_sec);
        csdcCont.Open(log_name4, par("outputCompression").boolValue());
    }

}

//-------------------Scheduling metods.------------------------------------

void CloudSchedulerFCFS_gnup::setupScheduler(){

    minimum_percent_storageNode_free = par("minimum_percent_storageNode_free").doubleValue();
    printNodePowerConsumption = par("printNodePowerConsumed").boolValue();
    printNodeEnergyConsumed = par("printNodeEnergyConsumed").boolValue();
    printComponentsPowerConsumption = par("printComponentsPowerConsumed").boolValue();
    printComponentsEnergyConsumed = par("printComponentsEnergyConsumed").boolValue();
    printDataCenterPowerConsumption = par("printDataCenterPowerConsumed").boolValue();
    printDataCenterEnergyConsumed = par("printDataCenterEnergyConsumed").boolValue();

    dc_EnergyConsumed = 0.0;

    AllocationManagement::setupStorageNodes();



}

void CloudSchedulerFCFS_gnup::schedule (){

    if (DEBUG_CLOUD_SCHED) printf("\n Method[CLOUD_SCHEDULER]: -------> schedule\n");

       //Define ...
           vector<AbstractNode*> selectedNodes;
           vector<VM*> attendedRequest_vms;
           NodeVL* node;
           Machine* machine;
           AbstractRequest* req;
           StorageRequest* req_st;
           RequestVM* req_vm;
           string uid;
           string nodeState;

           bool notEnoughResources;
           bool requestErased;
           int j;

       // Init ..

           notEnoughResources = false;
           requestErased = false;

           req = nullptr;
           selectedNodes.clear();
           attendedRequest_vms.clear();
           j = 0;

       // Begin ..

           // Schedule..
           if (schedulerBlock()){

               req =  getRequestByIndex(j);

               // Start with the vm allocation

               while ((j < (numPendingRequests())) && (req != nullptr)){

                   req_st = dynamic_cast<StorageRequest*>(req);
                   req_vm = dynamic_cast<RequestVM*>(req);

                   if (req_st != nullptr){

                       machine = getNodeByIndex(req_st->getNodeSetId(),req_st->getNodeId(), false);

                       node = check_and_cast<NodeVL*>(machine);

                       // Analyzes and create the connections vector in req_st depending on the selected fs
                       // This method will invoke selectStorageNodes
                       //TODO: Return the st_req and analyzes the error at sched
                       AbstractDCManager::userStorageRequest (req_st, node);
                       eraseRequest(req_st);
                   }
                   else if (req_vm != nullptr){

                       if (req->getOperation() == REQUEST_START_VM) {
                           notEnoughResources = request_start_vm (req_vm);
                           if (!notEnoughResources){
                               eraseRequest(req);
                               requestErased = true;
                           }
                       }

                       else if (req->getOperation() == REQUEST_FREE_RESOURCES){
                           request_shutdown_vm(req_vm);
                           eraseRequest(req);
                           requestErased = true;
                       }

                   }
                   else if(req->getOperation() == REQUEST_ABANDON_SYSTEM){
                       AbstractUser* user;
                       AbstractCloudUser* cl_user;
                       user = getUserById(req->getUid());
                       cl_user = check_and_cast<AbstractCloudUser*>(user);
                       cl_user->deleteAllVMs();
                       user->callFinish();
                       deleteUser(req->getUid());
                       eraseRequest(req);
                       requestErased = true;
                   }
                   else {
                       throw cRuntimeError("Error: Operation unknown for CloudScheduler\n");
                   }

                   if (!requestErased){
                       j++;

                   }

                   requestErased = false;
                   req = getRequestByIndex(j);

               }

               schedulerUnblock();
           }

}

AbstractNode* CloudSchedulerFCFS_gnup::selectNode (AbstractRequest* req){

    if (DEBUG_CLOUD_SCHED) printf("\n Method[SCHEDULER_FIFO]: -------> select_nodes\n");

	// Define ..
        vector <int> set;
        AbstractNode* node;
        int vmCPU;
        int vmMemory;
        int i;
        elementType* el;
        int numProcesses;
        RequestVM* reqVm;

        // Cast
        reqVm = dynamic_cast<RequestVM*>(req);
        if (reqVm == nullptr) throw cRuntimeError("AbstractCloudManager::selectNode->Error. Casting the request\n");

    // Init ..
        set.clear();
        node = nullptr;
        el = reqVm->getSingleRequestType();
        vmCPU = el->getNumCores();
        vmMemory = el->getMemorySize();
        numProcesses = 0;

    // Begin ..

        // Define ..
        //    vector<HeterogeneousSet*>::iterator setIt;

        // Begin ..

            // Push in the set the different heterogeneous node types where it is possible allocate the vm
                for (i = 0; i < getMapSize(); i++){
                    if ( (getSetMemorySize(i, false) >= vmMemory) && (getSetNumCores(i,false) >= vmCPU) ){
                        set.push_back(i);
                        break;
                    }
                }

            // select the first set
                for (i = 0; i < (int)set.size(); i++){
                    for (int j = 0; j < getSetSize((*(set.begin() + i))); j++){
                        NodeVL* node_vl = check_and_cast<NodeVL*>(getNodeByIndex((*(set.begin())),j));
                        numProcesses = node_vl->getNumOfLinkedVMs();
                        if (numProcesses < maximum_number_of_processes_per_node){
                            node = check_and_cast<Node*>(node_vl);
                            break;
                        }
                    }
                }

		return node;

}

vector<AbstractNode*> CloudSchedulerFCFS_gnup::selectStorageNodes (AbstractRequest* st_req){

    if (DEBUG_CLOUD_SCHED) printf("\n Method[SCHEDULER_FIFO]: -------> selectStorageNode\n");

    // Define ..
        int numNodesFs = 1000000;
        int i,j;
        vector<int> selected_nodes;
        vector<AbstractNode*> nodes;
        AbstractNode* node = nullptr;
        int diskCapacity;
        int diskFree;
        StorageRequest* streq = nullptr;

    // Initialize ..
        selected_nodes.clear();
        nodes.clear();
        streq = dynamic_cast<StorageRequest*>(st_req);
        if (streq == nullptr) throw cRuntimeError("CloudSchedulerFSFC_gnup::selectStorageNodes ->can not cast to storage request\n");

    // Select the number of nodes depending of the type of filesystem
        if (streq->getFsType() == FS_NFS){
            numNodesFs = 1;
        }
        else if (streq->getFsType() == FS_PFS){
            numNodesFs = numberOfPFSRemoteServers;
        }

        if (numNodesFs > getStorageNodesSize())
            throw cRuntimeError("[CloudSchedulerFSFC_gnup::selectStorageNodes] -> Trying to instantiate a file system that requires more disks than nodes.. (actually 1 disk device per node) ..\n");

        // Select the nodes

        i = 0;

        for (j = 0; (j < AllocationManagement::getStorageNodesSize()) && (i < numNodesFs); j++){

            node = AllocationManagement::getStorageNode(j);
            diskFree = node->getFreeStorage();
            diskCapacity = node->getStorageCapacity();

            if (diskFree >= (diskCapacity*minimum_percent_storageNode_free)){
                i++;
                selected_nodes.push_back(j);
                nodes.push_back(node);
            }

        }

        if (i != numNodesFs){
            nodes.clear();
        }


    // It is needed to update the structures ..
    if (selected_nodes.size() != 0)
        updateVmManagementStructures(selected_nodes, st_req->getPid(), st_req->getUid());

    return nodes;

}

vector<AbstractNode*> CloudSchedulerFCFS_gnup::remoteShutdown (AbstractRequest* req){

    // Define ..
        vector<AbstractNode*> nodes;
        RequestVM* reqVm;

    // Init ..
        nodes.clear();

     // Cast
        reqVm = dynamic_cast<RequestVM*>(req);
        if (reqVm == nullptr) throw cRuntimeError("AbstractCloudManager::remoteShutdown->Error. Casting the request\n");


    // Search the user entry if exists
        int position;
        position = searchUserVMAllocation(reqVm->getUid());


        // If it exists an entry, it means that the user has remote storage
        if (position != -1)
            nodes = deleteVMfromStorageNodes(position, req->getPid());

        else
            nodes.clear();


        return nodes;

}

void CloudSchedulerFCFS_gnup::freeResources (int uId, int pId, AbstractNode* computingNode){

}

void CloudSchedulerFCFS_gnup::printEnergyValues(){

    // Define ..
    int i, j;
    AbstractNode* nodeA;
    Node* node;
    ostringstream data;
    ofstream file;
    int computeNodeMapSize;
    int computeNodeSetSize;
    int storageNodeMapSize;
    int storageNodeSetSize;

    double nodeEnergyConsumed = 0.0;
    double nodeInstantConsumption = 0.0;
    double cpuEnergyConsumed = 0.0;
    double cpuInstantConsumption = 0.0;
    double memoryEnergyConsumed = 0.0;
    double memoryInstantConsumption = 0.0;
    double nicEnergyConsumed = 0.0;
    double nicInstantConsumption = 0.0;
    double storageEnergyConsumed = 0.0;
    double storageInstantConsumption = 0.0;
    double psuEnergyConsumed = 0.0;
    double psuInstantConsumption = 0.0;
    double dataCenterInstantConsumption = 0.0;
    int totalNodes = 0;
    // Init..
        computeNodeMapSize = getMapSize();
        storageNodeMapSize = getStorageMapSize();

    if ((printNodePowerConsumption) || (printNodeEnergyConsumed) ||
        (printComponentsEnergyConsumed) || (printComponentsPowerConsumption) ||
        (printDataCenterPowerConsumption)  || (printDataCenterEnergyConsumed)
        )
//        data << simTime();

    // Compute nodes
    for (i = 0; i < computeNodeMapSize; i++){

        computeNodeSetSize = getSetSize(i, false);

       for (j = 0; j < computeNodeSetSize; j++){

           nodeA = getNodeByIndex(i,j, false);
           node = dynamic_cast<Node*>(nodeA);

           // Get all the data to variables

           if (printComponentsPowerConsumption){
               cpuInstantConsumption = node->getCPUInstantConsumption();
               memoryInstantConsumption = node->getMemoryInstantConsumption();
               storageInstantConsumption = node->getStorageInstantConsumption();
               nicInstantConsumption = node->getNICInstantConsumption();
               psuInstantConsumption = node->getPSUConsumptionLoss();
           }

           if (printComponentsEnergyConsumed){
               cpuEnergyConsumed = node->getCPUEnergyConsumed();
               memoryEnergyConsumed = node->getMemoryEnergyConsumed();
               storageEnergyConsumed = node->getStorageEnergyConsumed();
               nicEnergyConsumed = node->getNICEnergyConsumed();
               psuEnergyConsumed = node->getPSUConsumptionLoss();
           }

           if (printNodePowerConsumption){
               if (!printComponentsPowerConsumption)
                   nodeInstantConsumption = node->getInstantConsumption();
               else
                   nodeInstantConsumption =  cpuInstantConsumption + memoryInstantConsumption + storageInstantConsumption+ nicInstantConsumption + psuInstantConsumption;
           }

           if (printNodeEnergyConsumed){
               if (!printComponentsEnergyConsumed)
                   nodeEnergyConsumed = node->getEnergyConsumed();
               else
                   nodeEnergyConsumed =  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
           }

           if(printDataCenterPowerConsumption){
               if (printNodePowerConsumption){
                   dataCenterInstantConsumption += nodeInstantConsumption;
               }
               else{
                   if (printComponentsPowerConsumption){
                       dataCenterInstantConsumption +=  cpuInstantConsumption + memoryInstantConsumption + storageInstantConsumption+ nicInstantConsumption + psuInstantConsumption;
                   } else{
                       dataCenterInstantConsumption += node->getInstantConsumption();
                   }
               }
           }

           if(printDataCenterEnergyConsumed){
               if (printNodeEnergyConsumed){
                   dc_EnergyConsumed += nodeEnergyConsumed;
               }
               else{
                   if (printComponentsEnergyConsumed){
                       dc_EnergyConsumed +=  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
                   } else{
                       dc_EnergyConsumed += node->getEnergyConsumed();
                   }
               }
           }

           if (printEnergyTrace) {
               data << simTime() << " " <<  totalNodes << " " << nodeInstantConsumption << " " << node->getFullName() << "\n";
               csdccont3D.Append(data.str().c_str()) ;

               data.str("");
               data.clear();

               data << simTime() << " " <<  totalNodes << " " << nodeEnergyConsumed << " " << node->getFullName() << "\n";
               csdccont3dAg.Append(data.str().c_str()) ;

               data.str("");
               data.clear();

               totalNodes++;

//               if ((printNodePowerConsumption) || (printNodeEnergyConsumed) || (printComponentsPowerConsumption) || (printComponentsPowerConsumption))
//                   data  << endl << node->getFullName() << ";" << node->getState();
//               if (printNodePowerConsumption)
//                   data << ";(nW)" << nodeInstantConsumption;
//               if (printNodeEnergyConsumed)
//                   data << ";(nJ)" << nodeEnergyConsumed;
//               if (printComponentsPowerConsumption)
//                   data << ";(cW)" << cpuInstantConsumption << ";" << memoryInstantConsumption << ";" << nicInstantConsumption << ";" <<  storageInstantConsumption << ";" << psuInstantConsumption;
//               if (printComponentsPowerConsumption)
//                   data << ";(cJ)" << cpuEnergyConsumed << ";" << memoryEnergyConsumed << ";" << nicEnergyConsumed << ";" <<  storageEnergyConsumed << ";" << psuEnergyConsumed;
           }
       }
    }

    // Storage nodes
    for (i = 0; i < storageNodeMapSize; i++){

       storageNodeSetSize = getSetSize(i, true);

       for (j = 0; j < storageNodeSetSize; j++){

           nodeA = getNodeByIndex(i,j, true);
           node = dynamic_cast<Node*>(nodeA);

           // Get all the data to variables

             if (printComponentsPowerConsumption){
                 cpuInstantConsumption = node->getCPUInstantConsumption();
                 memoryInstantConsumption = node->getMemoryInstantConsumption();
                 storageInstantConsumption = node->getStorageInstantConsumption();
                 nicInstantConsumption = node->getNICInstantConsumption();
                 psuInstantConsumption = node->getPSUConsumptionLoss();

             }

             if (printComponentsEnergyConsumed){
                 cpuEnergyConsumed = node->getCPUEnergyConsumed();
                 memoryEnergyConsumed = node->getMemoryEnergyConsumed();
                 storageEnergyConsumed = node->getStorageEnergyConsumed();
                 nicEnergyConsumed = node->getNICEnergyConsumed();
                 psuEnergyConsumed = node->getPSUConsumptionLoss();
             }

             if (printNodePowerConsumption){
                 if (!printComponentsPowerConsumption)
                     nodeInstantConsumption = node->getInstantConsumption();
                 else
                     nodeInstantConsumption =  cpuInstantConsumption + memoryInstantConsumption + storageInstantConsumption+ nicInstantConsumption + psuInstantConsumption;
             }

             if (printNodeEnergyConsumed){
                 if (!printComponentsEnergyConsumed)
                     nodeEnergyConsumed = node->getEnergyConsumed();
                 else
                     nodeEnergyConsumed =  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
             }

             if(printDataCenterPowerConsumption){
                 if (printNodePowerConsumption){
                     dataCenterInstantConsumption += nodeInstantConsumption;
                 }
                 else{
                     if (printComponentsPowerConsumption){
                         dataCenterInstantConsumption +=  cpuInstantConsumption + memoryInstantConsumption + storageInstantConsumption+ nicInstantConsumption + psuInstantConsumption;
                     } else{
                         dataCenterInstantConsumption += node->getInstantConsumption();
                     }
                 }
             }

             if(printDataCenterEnergyConsumed){
                 if (printNodeEnergyConsumed){
                     dc_EnergyConsumed += nodeEnergyConsumed;
                 }
                 else{
                     if (printComponentsEnergyConsumed){
                         dc_EnergyConsumed +=  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
                     } else{
                         dc_EnergyConsumed += node->getEnergyConsumed();
                     }
                 }
             }

             if (printEnergyTrace) {
                 data << simTime() << " " <<  totalNodes << " " << nodeInstantConsumption << " " << node->getFullName() << "\n";
                 csdccont3D.Append(data.str().c_str()) ;

                 data.str("");
                 data.clear();

                 data << simTime() << " " <<  totalNodes << " " << nodeEnergyConsumed << " " << node->getFullName() << "\n";
                 csdccont3dAg.Append(data.str().c_str()) ;

                 data.str("");
                 data.clear();

                 totalNodes++;
//                if ((printNodePowerConsumption) || (printNodeEnergyConsumed) || (printComponentsPowerConsumption) || (printComponentsPowerConsumption))
//                    data  << endl << node->getFullName() << ";" << node->getState();
//                if (printNodePowerConsumption)
//                    data << ";(nW)" << nodeInstantConsumption;
//                if (printNodeEnergyConsumed)
//                    data << ";(nJ)" << nodeEnergyConsumed;
//                if (printComponentsPowerConsumption)
//                    data << ";(cW)" << cpuInstantConsumption << ";" << memoryInstantConsumption << ";" << nicInstantConsumption << ";" <<  storageInstantConsumption << ";" << psuInstantConsumption;
//                if (printComponentsPowerConsumption)
//                    data << ";(cJ)" << cpuEnergyConsumed << ";" << memoryEnergyConsumed << ";" << nicEnergyConsumed << ";" <<  storageEnergyConsumed << ";" << psuEnergyConsumed;
             }
       }
   }

    data << simTime() << " " <<  dataCenterInstantConsumption << " " << dc_EnergyConsumed << "\n";
    csdcCont.Append(data.str().c_str());
//    if ((printDataCenterPowerConsumption)  || (printDataCenterEnergyConsumed)){
//        data << endl << "#";
//        if (printDataCenterPowerConsumption){
//            data << dataCenterInstantConsumption;
//        }
//
//        if (printDataCenterEnergyConsumed){
//            if (printDataCenterPowerConsumption) data << ";";
//            data << dc_EnergyConsumed << endl;
//        }
//    }
//
//    // print data to the file
//        csfifo_f.Append(data.str().c_str()) ;

}

void CloudSchedulerFCFS_gnup::finalizeScheduler(){
    // Define ..
      AbstractNode* nodeA;
      Node* node;
      ostringstream data;
      ostringstream file;
      vector<string> nodeNames;
      vector<double> energyValues;
      nodeNames.clear();
      energyValues.clear();
//      ofstream file;

      int i,j;
      int computeNodeMapSize;
      int storageNodeMapSize;
      int storageNodeSetSize;

      int computeNodeSetSize = 0;
      int storageSetSize = 0;

      double nodeEnergyConsumed = 0.0;
      double cpuEnergyConsumed = 0.0;
      double memoryEnergyConsumed = 0.0;
      double nicEnergyConsumed = 0.0;
      double storageEnergyConsumed = 0.0;
      double psuEnergyConsumed = 0.0;
      double dataCenterEnergyConsumed = 0.0;
      //vector<HeterogeneousSet*>::iterator setIt;

    if (printEnergyToFile){

        // Print the totals
            if (printEnergyTrace){

              // Compute nodes
                  for (i = 0; i < getMapSize(); i++)
                      computeNodeSetSize += getSetSize(i, false);

              // Storage nodes
                  for (i = 0; i < getStorageMapSize(); i++)
                      storageSetSize += getSetSize(i, true);


              // print the mode and the number of nodes
//              file.open(logName.c_str(), ios::app);
//              file << "@Total-mode;" << totalNumberNodes << endl;
//              file << simTime() << endl;

              // Init..
                  computeNodeMapSize = getMapSize();
                  storageNodeMapSize = getStorageMapSize();

              // Compute nodes
              for (i = 0; i < computeNodeMapSize; i++){

                  computeNodeSetSize = getSetSize(i, false);

                 for (j = 0; j < computeNodeSetSize; j++){

                     nodeA = getNodeByIndex(i,j, false);
                     node = dynamic_cast<Node*>(nodeA);
                 // Get all the data to variables
                     cpuEnergyConsumed = node->getCPUEnergyConsumed();
                     memoryEnergyConsumed = node->getMemoryEnergyConsumed();
                     storageEnergyConsumed = node->getStorageEnergyConsumed();
                     nicEnergyConsumed = node->getNICEnergyConsumed();
                     psuEnergyConsumed = node->getPSUConsumptionLoss();

                     nodeEnergyConsumed =  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;

                     nodeNames.push_back(node->getFullName());
                     energyValues.push_back(nodeEnergyConsumed);
//                     data  << node->getFullName() << " " << nodeEnergyConsumed << endl;
                     dataCenterEnergyConsumed += nodeEnergyConsumed;
                  }
             }

              // Storage nodes
              for (i = 0; i < storageNodeMapSize; i++){

                 storageNodeSetSize = getSetSize(i, true);

                 for (j = 0; j < storageNodeSetSize; j++){
                     nodeA = getNodeByIndex(i,j, true);
                     node = dynamic_cast<Node*>(nodeA);

                 // Get all the data to variables
                     cpuEnergyConsumed = node->getCPUEnergyConsumed();
                     memoryEnergyConsumed = node->getMemoryEnergyConsumed();
                     storageEnergyConsumed = node->getStorageEnergyConsumed();
                     nicEnergyConsumed = node->getNICEnergyConsumed();
                     psuEnergyConsumed = node->getPSUConsumptionLoss();

                     nodeEnergyConsumed =  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
//                     data << node->getFullName() << ";" << nodeEnergyConsumed << endl;
                     nodeNames.push_back(node->getFullName());
                     energyValues.push_back(nodeEnergyConsumed);

                     dataCenterEnergyConsumed += nodeEnergyConsumed;
                 }
              }

//              data << "#" << dataCenterEnergyConsumed << endl;

//              data << simTime() << " " <<  totalNodes << " " << nodeEnergyConsumed << " " << node->getFullName << "\n";
              for (i = 0; i < (int)nodeNames.size(); i++){
                  data << (*(nodeNames.begin() + i)) << " " << (*(energyValues.begin() + i)) << " " << dataCenterEnergyConsumed << "\n";
              }

              csdcTot.Append(data.str().c_str()) ;
              data.str("");
              data.clear();

              csdccont3D.Close();
              csdccont3dAg.Close();
              csdcCont.Close();
              csdcTot.Close();
              // print data to the file
//                csfifo_f.Append(file.str().c_str()) ;
//                csfifo_f.Close();
            }
    }

}

int CloudSchedulerFCFS_gnup::selectNodeSet (const string &setName, int vmcpu, int vmmemory){

	if (DEBUG_CLOUD_SCHED) printf("\n Method[SCHEDULER_FIFO]: -------> selectNodeSet\n");

	int bestFit;
	int acumCPU;
//	int acumVM;
	int i;
	int numTypeSize;
	int numCPUs;

	// Init ..
	bestFit = -1;
	acumCPU = 1024;
	numCPUs = -1;
	numTypeSize = getMapSize();
	//acumVM = set.begin()->getTotalMemory();

	for (i = 0; i < numTypeSize; i++){
	    numCPUs = getSetNumCores(i, false);
		if (((numCPUs - vmcpu) < acumCPU )){ //&& ((set.begin()->getTotalMemory - acumVM )  < vmmemory)){

			bestFit  = i;
			acumCPU  = numCPUs - vmcpu;
			//vmmemory = bestFit -> getTotalMemory - acumVM;
		}
	}

	return bestFit;

}


} // namespace icancloud
} // namespace inet
