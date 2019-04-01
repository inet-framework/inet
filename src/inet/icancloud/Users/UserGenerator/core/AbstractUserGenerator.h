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

#ifndef ABSTRACTUSERGENERATOR_H_
#define ABSTRACTUSERGENERATOR_H_

#include "inet/icancloud/Base/include/icancloud_types.h"
#include "inet/icancloud/Users/Profiles/CloudUser/AbstractCloudUser.h"
#include "inet/icancloud/Applications/Base/UserJob.h"

#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Management/DataCenterManagement/Base/UserManagement.h"

class UserJob;
class UserManagement;

#define DEBUG false 

namespace inet {

namespace icancloud {


class AbstractUserGenerator : virtual public icancloud_Base{

protected:

	// Main user parameters
		string name;                                           /** id name of the users*/
		int users_created_counter;                             /** count to build the user name*/
		string behavior;                                       /** kind of user that define his behavior*/

        string distributionName;                               /** distribution which defines the user appearance at the cloud system */
        vector<double> distributionParams;

        // vm selected by user
        struct vmSelection{
            string vmtype;                          //Type VM (TypeVMID)
            int quantity;                           //Quantity
        };

        // Structure of vms of a user
            vector <vmSelection*> vmSelect;         // VMMAP
//
        // vm selected by user
        struct jobSelection{
            string appName;                         // Application Name
            UserJob* job;                          // Job definition
            int replicas;                           // Quantity
        };

        // Structures of jobs of user
            vector <jobSelection*> userJobSet;

	// To create the remote servers
		string remoteFileSystemType;

    // Pointer to cloud manager
		UserManagement *userManagementPtr;

    // Log name
        string logName;

        bool printResults;

        /** Output File directory */
        static const string OUTPUT_DIRECTORY;



    AbstractUserGenerator();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

   /**
	* Module ending.
	*/
	virtual void finish() override;

   /**
	* Process a request message.
	* @param sm Request message.
	*/
	virtual void processRequestMessage (Packet *) override {};

   /**
	* Process a response message.
	* @param sm Request message.
	*/
	virtual void processResponseMessage (Packet *) override {};

   /**
    * Get the out Gate to the module that sent <b>msg</b>.
    * @param msg Arrived message.
    * @return Gate (out) to module that sent <b>msg</b> or nullptr if gate not found.
    */
    cGate* getOutGate (cMessage *msg) override;

  /**
   * Process a self message.
   * @param msg Self message.
   */
   virtual void processSelfMessage (cMessage *msg) override {};

   /*
    * If allowToExecute = true means that the execution will be performed until the last user finish
    * allowToExecute = false means that the execution has finished the time and it is forced to finish
    */
    void finalizeUserGenerator(bool allowToExecute);

	void createUser ();

	double selectDistribution();

    virtual void userCreateGroups(int intervals, int nusers) = 0;

private:
    UserJob* cloneJob (UserJob* app, cModule* userMod, string appName);

public:
    virtual ~AbstractUserGenerator();
};

} // namespace icancloud
} // namespace inet

#endif /* USERGENERATOR_H_ */


