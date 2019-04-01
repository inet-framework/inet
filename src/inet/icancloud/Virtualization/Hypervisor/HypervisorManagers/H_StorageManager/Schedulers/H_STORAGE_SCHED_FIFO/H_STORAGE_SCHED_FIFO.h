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

#ifndef HSTORAGESCHEDFIFO_H_
#define HSTORAGESCHEDFIFO_H_

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/H_StorageManager_Base.h"

namespace inet {

namespace icancloud {


class H_STORAGE_SCHED_FIFO : public H_StorageManager_Base{


public:

    virtual ~H_STORAGE_SCHED_FIFO();

    /*
     * Module initialization
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

    /*
     * Module finalization
     */
    void finish() override;

    /*
     * Scheduler fifo
     */
    void schedulingStorage(Packet *msg) override;

    int getNumberOfVMs (){return vmIDs.size();};

};

} // namespace icancloud
} // namespace inet

#endif /* HSTORAGESCHEDFIFO_H_ */
