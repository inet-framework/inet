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

#include "H_CPU_SCHED_FIFO.h"

namespace inet {

namespace icancloud {


Define_Module(H_CPU_SCHED_FIFO);

H_CPU_SCHED_FIFO::~H_CPU_SCHED_FIFO() {
    coreQueue.clear();
}

void H_CPU_SCHED_FIFO::initialize(int stage) {
    H_CPUManager_Base::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {

        for (int i = 0; i < (int) numCPUs; i++) {
            vmIDs.push_back(-1);
        }

        coreQueue.clear();
    }
}

void H_CPU_SCHED_FIFO::finish(){
    coreQueue.clear();
    H_CPUManager_Base::finish();
}

void H_CPU_SCHED_FIFO::schedulingCPU(Packet *pkt){

    // Define ..
    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("Packet doesn't include icancloud_Message header");

    int core = -1;
    bool selected = false;
    // Begin. .
    for (int i = 0; (i < (int)numCPUs) && (!selected); i++){
        core = (*(vmIDs.begin() + i));
        if (core == -1){
            selected = true;
            core = i;
            (*(vmIDs.begin() + i)) = pkt->getArrivalGate()->getIndex();
        }
    }

    // If there is at least a free core
    if (selected)
        sendRequestToCore(pkt, toNodeCPU[core], core);
    // Enqueue the processor. The core is full
    else{
        coreQueue.push_back(pkt);
    }


////    ---------------------
//  pCPU_maxSpeed <- getCoreMaxSpeed();
//  numCores <- getNumCores();
//
//  if (newMessageArrival()){
//      vmID <- getVMID (message);
//      Dom  <- getDomain(vmID);
//
//      //Creates the domain and the vcpus
//     if (!existsDomain(Dom)){
//         numVCPUs <-getNumberCPUs(vmID);
//           vCPU_speed<- getVCPUSpeed(vmID);
//
//           // Get the pCPU where the domain is going to be executed
//           pCPU <- getPCPU_MinDomains();
//
//           //create vCPU structure
//           Dom <- createDomain(Domain_name);
//           for (i = 0; i < numVCPUs; i++){
//               vCPU.name = vmID;
//               vCPU.tSlice = 30ms;
//               vCPU.credit = (-(1<<30));
//               vCPU.weight <- 256;
//               vCPU.cap <- ((vCPUspeed *100) / pCPUspeed);
//               vCPU.lastUpdate <- -1;
//
//               // if the pCPU is migrated. The return will be found using the parameters
//               vCPU.Domain = Dom;
//               vCPU.pCPU = pCPU;
//           }
//
//           // link new Domain to concrete physical cpu
//           insert(pCPU, Dom);
//     }
//
//     vCPU <- getVCPU_IDLE(Dom);
//       if (existsVCPU(vCPU)){
//           vCPU.tSlice = 30ms;
//           vCPU.credit <- vCPU.weight;
//       } else {
//           vCPU<-getvCPU_minLoad(Dom);
//       }
//
//       insertOrderedRunQ(pCPU, vCPU); // insertInOrder
//       enqueue(message,vCPU);
//  }
//  else if(messageArrivalFromPCPU()){
//
//      vCPU <- getVCPU(message);
//      Dom  <- getDomain (vCPU);
//      pCPU <- identifyCore(message);
//      pCPU.running = false;   //To identify if it is neccesary to launch again a new message
//
//      if (finishCompute(message)){
//
//          returnToSource(message);
//
//          if (!hasPendingMessages(vCPU)){
//
//              // if it was a migrated vCPU from another core
//              if (vCPU.pCPU != pCPU){
//                  migrateVCPU(vCPU,vCPU.pCPU); //migrate vCPU to the original pCPU. The domain is into pCPU.
//              }
//
//              // reenqueue the vCPU
//              vCPU.Priority = IDLE;
//              insertToIdleVCPU(vCPU);
//          }
//
//      }
//
//      // the message is due to a finished slice or alarm
//      else{
//
//          burnCredit(vCPU,now);
//
//          // credit 0
//          if (vCPU.credit <= 0){
//              vCPU.credit <- (-1);
//              vCPU.Priority = OVER;
//
//              //insert at last and the head will be executed
//              if (existsMoreVCPU()){
//                  pushBackRunQ(pCPU, vCPU);
//              }
//          }
//
//          // slice of message
//          if (tSlice <= 0){
//              enqueue(message,vCPU);
//              setMessageToExecute(vCPU, getMessageHead(vCPU));
//          }
//      }
//  }
//
//    // Recalculate current executions.
//
//     DEFINE: IDLE_PCPUS = [];        // cpus that are idle or off.
//     DEFINE: vCPUs_OVERLOAD = [];    // cpus that have more that 1 vcpu allocated
//
//     for (i = 0; i < numCores; i++){
//
//        pCPU <- getPCPU (i);
//
//        // Get idle structures at pCPU and overloaded cores due to vCPUs
//        if ((getCoreState(i) == IDLE) || (getCoreState(i) == OFF)){
//            IDLE_PCPUS.insert (pCPU);
//        } else {
//            vCPU <- getExecutingVCPU(pCPU);
//            burnCredit(vCPU, now);
//            if (getNumberOfvCPUs > 2) {
//                vCPUs_OVERLOAD.insert(vCPU);
//            }
//        }
//     }
//
//     // migrate if there are pCPUs idle and vCPUs waiting for execute at overloaded cores
//     while ((IDLE_PCPUS.size() > 0) && (VCPUs_OVERLOAD > 0)){
//         pCPU <- IDLE_PCPUS(0);
//         vCPU <- VCPUs_OVERLOAD(0);
//         migrateVCPU(vCPU,pCPU);
//         setMessageToExecute(vCPU, getMessageHead(vCPU));
//         insertOrderedRunQ(pCPU, vCPU); // insertInOrder
//     }
//
//     // Execute the messages at cores
//     for (int i = 0; i < numCores; i++){
//         pCPU <- getPCPU(i);
//
//         if ((runQueueSize(pCPU) > 0) &&
//             (!isRunning (pCPU))
//            ){
//
//             // There is a message to execute.
//             changePerformanceState(vCPU.cap);
//
//             sendMessageToCore(getMessageToExecute(), pCPU);
//             setMessageAsExecuting(pCPU, vCPU, id <- getMessageID(message));
//
//             setVCPUAsExecuting(pCPU, vCPU);
//             pCPU.running = true;
//
//         }
//     }
//
//
//
//
//
//
//  }

}

void H_CPU_SCHED_FIFO::processHardwareResponse(Packet *pkt){

    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("Packet doesn't include icancloud_Message header");
    // Manage the core state
    int core =  pkt->getArrivalGate()->getIndex();
    (*(vmIDs.begin() + core)) = -1;

    // If there is more messages, schedule it to the empty core..
    if (coreQueue.size() != 0) {
        auto enqueuedMsg = coreQueue.front();
        coreQueue.pop_front();
        schedulingCPU(enqueuedMsg);
    }
}


} // namespace icancloud
} // namespace inet
