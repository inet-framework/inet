#ifndef __BASIC_FILE_SYSTEM_H_
#define __BASIC_FILE_SYSTEM_H_

#include <omnetpp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::vector;

#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Base/Messages/icancloud_File.h"
#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message.h"
#include "inet/icancloud/Base/Messages/icancloud_App_IO_Message.h"

#include "inet/icancloud/Base/Parser/cfgPreloadFS.h"

namespace inet {

namespace icancloud {


//
// @class Node_FileSystem Node_FileSystem.h "Node_FileSystem.h"
//
// This module translates an I/O file request expressed as (fileName, offset, requestSize) to a list of disk blocks.
// It is able to distinguish between different tenant accounts at file system.
// It's based on Basic_FileSystem
//
// @author Gabriel González Castañé
// @date 2013-10-6

class Node_FileSystem: public icancloud_Base{

public:
    /** Struct for defining the set of files into the file systems*/
    struct UserFileList{
        int userID;                                 /** User id*/
        vector <icancloud_File> fileList;           /** File list*/
    };
    typedef struct UserFileList userFileList;

    protected:

        /** PreLoad File System Files? */
        bool preLoadFiles;

        /** Gate ID. Input gate from VM. */
        cGate* fromVMGate = nullptr;

        /** Gate ID. Input gate from IOR. */
        cGate* fromIORGate = nullptr;

        /** Gate ID. Output gate to VM. */
        cGate* toVMGate = nullptr;

        /** Gate ID. Output gate to IOR. */
        cGate* toIORGate = nullptr;

        /** Pending messages */
        cMessage* latencyMessage = nullptr;

        struct pendingMsg{
            cMessage* msg;
            long int msgID;
        };

        vector<pendingMsg*> pendingMessages;

        /** Set of users with its file lists */
        vector <userFileList*> usersFileList;

        /** Open Time */
        static const const_simtime_t OPEN_TIME;

        /** Close Time */
        static const const_simtime_t CLOSE_TIME;

        /** Create Time */
        static const const_simtime_t CREATE_TIME;


       /**
        * Destructor.
        */
        ~Node_FileSystem();

       /**
        * Module initialization.
        */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        

       /**
        * Module ending.
        */
        virtual void finish() override;


    private:

        vector<int> msg_delete_user_id;

       /**
        * Get the outGate to the module that sent <b>msg</b>
        * @param msg Arrived message.
        * @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
        */
        cGate* getOutGate (cMessage *msg) override;

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
        * Process a response message.
        * @param sm Request message.
        */
        void processResponseMessage (Packet *) override;

       /**
        * Calculates the involved blocks (list of branches) of an I/O request and sends the request to Block Server.
        *
        * @param sm StandardMessage that contains the I/O request operation.
        */
        void processIORequest (Packet *);

    public:

       /**
        * PreLoad files in File System.
        * @return icancloud_OK if preLoad files is successful of icancloud_ERROR in another case.
        */
        int load_UserPreloadFiles (string preload_data, int userID);


    private:

       /**
        * Inserts a new file inside the File System's table.
        *
        * @param fileSize File's size.
        * @param fileName File's name.
        * @return icancloud_OK if the file was been inserted correctly, or icancloud_ERROR if something was wrong.
        */
        int insertNewFile (unsigned int fileSize, string fileName, userFileList* user);

        /*
         * Search a user in the file system
         */
        bool searchUser (int userName);

       /**
        * Search a file in the file system.
        *
        * @param fileName File name.
        * @return Iterator to the corresponding entry of <b>fileName</b>
        */
        vector <icancloud_File>::iterator searchFile (string fileName, userFileList* user);

        /**
        * Removes a user files from the File System's table.
        *
        * @param userName User's name.
        */
        void deleteUser (Packet *);

        /*
         * This method check if the message sent to the disk is due as result of a User deletion.
         */
        bool checkResponseMsg(Packet *);

       /**
        * Removes a file from the File System's table.
        *
        * @param fileName File's name.
        */
        void deleteFile (string fileName, userFileList* user);

       /**
        * Translate an user I/O request into a branch list.
        * If somethings goes wrong (file not exist, request out of file size,... ) the returned
        * string will be empty.
        *
        * @param sm_bl Block list Message.
        */
        void translateIORequests (Packet *, userFileList* user);


       /**
        * Parse all files information to string. Debug purpose only!
        * @return All files stored on FS in string format.
        */
        string FSFilesToString ();

        void pushPendingMessage(cMessage *, cMessage* latencyMsg);

        cMessage* extractPendingMessage(cMessage* msg);
};

} // namespace icancloud
} // namespace inet

#endif
