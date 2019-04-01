#ifndef __REMOTE_STORAGE_APP_H_
#define __REMOTE_STORAGE_APP_H_
//
#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {


/**
 * @class RemoteStorage RemoteStorage.cc "RemoteStorage.h"
 *
 * This application is an application system managed by syscall manager in order to create connections
 * between compute nodes and remote storage servers. In addition it is also in charge of load tenant
 * files from preloadfiles (files that is going to be loaded before an user application execution) and the
 * file system structure.
 *
 * @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute;
 * @author @date 2013-03-04
 */

class RemoteStorageApp : virtual public icancloud_Base {

	protected:			

	/** Input gate from OS. */
	cGate* fromOSGate;

	/** Output gate to OS. */
	cGate* toOSGate;

	cMessage* selfProcessingMsg;

	struct preloadCreation{
		int uId;
		int pId;
		int spId;
		int numberOfIor;
		vector<Packet *> sm_files;
	};

	vector <preloadCreation*> sm_files_creation;
	vector <preloadCreation*> sm_ior_creation;

    int localStoragePort;

    string storageConnectionType;

    struct listen_repetition_t{
        int uId;
        int pId;
    };

    typedef listen_repetition_t listen_repetition;

    vector<listen_repetition*> lrep;


	   /**
		* Destructor
		*/
		~RemoteStorageApp();

	   /**
 	 	*  Module initialization.
  		*/
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        

	   /**
		* Module ending.
		*/
		void finish() override;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		void processSelfMessage (cMessage *msg) override;;

	   /**
		* Process a request message.
		* @param sm Request message.
		*/
		void processRequestMessage (Packet *) override;

	   /**
		* Process a response message from a module in the local node. Empty method.
		* All response messages are computed by TcpSocket::CallbackInterface
		* @param sm Request message.
		*/
		void processResponseMessage (Packet *) override;
		
		/**
		* Get the out Gate to the module that sent <b>msg</b>.
		* @param msg Arrived message.
		* @return Gate (out) to module that sent <b>msg</b> or nullptr if gate not found.
		*/
		cGate* getOutGate (cMessage *msg) override;


		/**
		 * This method set the established connection.
		 *
		 * This method must be invoked when the response of a SM_CREATE_CONNECTION operation arrives.
		 * The response message will contain the corresponding connection descriptor stored in parameter communicationId.
		 *
		 * @param sm Response message.
		 */
		void setEstablishedConnection (Packet  *);

		/**
		 * Process a NET call response
		 */
		void processNetCallResponse (Packet *);


		/**
		 * Process an I/O call response
		 */
		void processIOCallResponse (Packet *pkt);

	// ----------------- --------------------------- ----------------- //
	// ----------------- Operations with connections ----------------- //
	// ----------------- --------------------------- ----------------- //


	public:

		void closeConnection (int uId, int pId);

		void initialize_storage_data(int localPort);

		int getStoragePort(){return localStoragePort;};

		void create_listen (int uId, int pId);


	// ----------------- -------------- ----------------- //
	// ----------------- Remote Storage ----------------- //
	// ----------------- -------------- ----------------- //

		void connect_to_storage_node(vector<string> destinationIPs, string fsType, int destPort, int uid, int pid, string vIP, int jobId);

		void createFilesToPreload(int uid, int pid, int spId, string vmIP, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths, bool remoteHost);

		void deleteUserFSFiles(int uid, int pid);

	protected:

		void setDataStorage_EstablishedConnection (Packet *sm_net);

};

} // namespace icancloud
} // namespace inet

#endif
