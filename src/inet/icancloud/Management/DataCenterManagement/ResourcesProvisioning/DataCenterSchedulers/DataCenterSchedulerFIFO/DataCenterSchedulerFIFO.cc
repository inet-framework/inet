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

#include "DataCenterSchedulerFIFO.h"

namespace inet {

namespace icancloud {


Define_Module(DataCenterSchedulerFIFO);


//-------------------Scheduling metods.------------------------------------

void DataCenterSchedulerFIFO::setupScheduler(){

    // TODO
}

AbstractNode* DataCenterSchedulerFIFO::selectNode (AbstractRequest* req){

    // TODO
    return nullptr;
}

vector<AbstractNode*> DataCenterSchedulerFIFO::selectStorageNodes (AbstractRequest* st_req){

    // TODO
    std::vector<AbstractNode*> val;
    return val;
}

void DataCenterSchedulerFIFO::schedule (){

      //TODO
}

void DataCenterSchedulerFIFO::freeResources(int uId, int pId, AbstractNode* computingNode){
// TODO
}

void DataCenterSchedulerFIFO::finalizeManager(){

    // TODO
}

void DataCenterSchedulerFIFO::printEnergyValues(){

  // TODO
}


vector<AbstractNode*> DataCenterSchedulerFIFO::remoteShutdown (AbstractRequest* req){
//TODO
    std::vector<AbstractNode*> val;
    return val;
}

} // namespace icancloud
} // namespace inet
