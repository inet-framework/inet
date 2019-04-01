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
/**
 * @class SmartUser SmartUser.cc SmartUser.h
 *
 * This class is the behavior of a tenant that will launch parallel/Distributed applications.
 * He waits until all requested resources will be assigned, and after that event he configure the environment mpi, and then
 * perform the experiments.
 *
 * @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2013-06-08
 */


#ifndef SMART_USER_H_
#define SMART_USER_H_

#include "inet/icancloud/Users/Profiles/CloudUser/AbstractCloudUser.h"

namespace inet {

namespace icancloud {


class SmartUser : public AbstractCloudUser {

protected:

    // Struct for pending connections with storage nodes
        int pendingConnections;
        JobQueue* waiting_for_all_VMs;

        vector <UserJob*> pendingJobsVector;

        // MPI Values .-
            int numApplications;
            int port;
            int lastRank;

        vector < vector<int> > freePositionsAtVM;
        int pendingVms;

protected:

		virtual ~SmartUser();

	   /**
        * Module initialization.
        */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	     

       /**
        * Module ending.
        */
        void finish() override;


        // ------------------------------------------------------

        /*
         * The basic behavior, the first step to do is select the VMs to startUp and the
         * second is to request them to the cloud manager.
         */
        void userInitialization () override;

        /*
         * Things to do by tenant before leave the system
         */
        void userFinalization () override;


        //------------------------ Interact with VMs -------------------------

        /* This method request to the cloud manager the VMs to start them.
         *  @Param: The map of vms to calculate the request of vms.
         */
        AbstractRequest* selectResourcesJob (jobBase* job) override;

        /*
         * This method select where the app is going to execute.
         * This will select all the resources to deploy the mpi environment
         *
         */
        AbstractRequest* selectVMs_ToStartUp () override;
        /*
         * This method select where the app is going to execute.
         * To execute in a VM several apps.
         *      - add into the request VMs more than one time the same VM.
         *      - get the VMs in VM_STATE_FREE and VM_STATE_RUNNING
         */
        void requestAttended (AbstractRequest* req) override;

        /*
         * When the CloudManager attends a request and creates the VMs, it notifies this fact to
         * the user with request attended. The parameters are a vector with pointers created to the VMs modules and
         * the virtual machine type which they belong.
         */
        void requestErrorDeleted (AbstractRequest* req) override;

        // ------------------------- Interact with jobs --------------------------------

        /*
         * This method returns the first job of the waitingQueue structure of the user.
         * @param: the index is the position of the job in the vector jobList.
         */
        UserJob* selectJob () override;

        /*
         *  This method is useful to extract values from the job, or the timings of the executions.
         *  The method notify_UserJobHasFinished is responsible for deleting the job, move it from running queue to finish queue
         *  and call to schedule.
         */

        void jobHasFinished (jobBase* job) override;

        /*
         * This method define the behavior of the user. The order of the actions and the main decisions of the user.
         */
        void schedule() override;

private:

        // Variable to control the creation of as replicas as the user has defined at generateNumberOfAppReplicas method
        bool replicasCreated;

        /*
         * This method generates the mpi environment. It allocates the ip, port and rank of each process.
         */
		void generateMPIEnv(RequestVM *req);

		/*
		 * This method defines the number of mpi apps that will be generated.
		 * In this case, it will be generated as much replicas as the quantity of cores of a VM.
		 */
		void generateNumberOfAppReplicas ();

};

} // namespace icancloud
} // namespace inet

#endif /* BASICBEHAVIOR_H_ */
