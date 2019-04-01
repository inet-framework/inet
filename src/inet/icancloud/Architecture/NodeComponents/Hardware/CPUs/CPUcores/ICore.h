#ifndef _iCore_H_
#define _iCore_H_

#include "inet/icancloud/Architecture/NodeComponents/Hardware/HWEnergyInterface.h"

#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message.h"
#include "inet/icancloud/Base/include/Constants.h"
#include "inet/icancloud/Architecture/NodeComponents/Hardware/CPUs/CPUController/CPUController.h"

namespace inet {

namespace icancloud {


/**
 * @class iCore iCore.h "iCore.h"
 *   
 * Class that simulates a processor core
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-11
 *
 * @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
 * @date 2012-23-11
 *
 */

class CPUController;

class ICore : public HWEnergyInterface{


protected:

        /** Gate ID. Input gate. */
        cGate* inGate = nullptr;

        /** Gate ID. Output gate. */
        cGate* outGate = nullptr;

        /** Pending message */
        Packet *pendingMessage = nullptr;

        /** Speed of the CPU core */
        unsigned long int speed;

        /** Actual tick time (in seconds) */
        simtime_t tick_s;

        /** CPUController to indicate if the core is off and the other cores depends on it*/
        CPUController* cpuController;

        /** Instructions per tick */
        long int ipt = 0;

        /** The actual performance state of the cpu*/
        int actualPState = 0;

        /** The set of PStates */
        static const char * PStates [];

        /** Node state */
        string nodeState = MACHINE_STATE_OFF;

        /** Energy independent per core */
        bool independentCores;

       /**
        * Destructor.
        */
        virtual ~ICore();

       /**
        *  Module initialization.
        */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

        /**
        * Module ending.
        */
        virtual void finish() override = 0;


       /**
        * Get the outGate ID to the module that sent <b>msg</b>
        * @param msg Arrived message.
        * @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
        */
        virtual cGate* getOutGate (cMessage *msg) override = 0;

       /**
        * Process a self message.
        * @param msg Self message.
        */
        virtual void processSelfMessage (cMessage *msg) override = 0;

       /**
        * Process a request message.
        * @param sm Request message.
        */
        virtual void processRequestMessage (Packet *) override = 0;

       /**
        * Process a response message.
        * @param sm Request message.
        */
        virtual void processResponseMessage (Packet *) override = 0;

       /**
        * Calculates the amount of time to execute completely the current computing block.
        * @param reainingMIs Million instructions to be executed.
        * @return Time to execute reainingMIs instructions.
        */
        virtual simtime_t getTimeToExecuteCompletely (long int remainingMIs) = 0;

       /**
        * Calculates the amount of time to execute completely the current computing block.
        * @param reainingTime Amount of time for executing current CB.
        * @return Amount of CPU time to execute reainingTime.
        */
        virtual simtime_t getMaximumTimeToExecute (simtime_t reainingTime) = 0;

public:
        /*
         * Change the energy state of the memory given by node state
         */
        virtual void changeDeviceState (const string &nodeState,unsigned componentIndex) override= 0;

        /*
         * Change the energy state of the processor!
         */
        virtual void changeState (const string &energyState,unsigned componentIndex) override = 0;


        /*
         * Register the change of state into the HWInterface from cpu controller
         */
        void e_changeState (const string &state) override;

        /*
         * Get the info of the number of states from the cpu controller
         */
        int e_getStatesSize () override;
};

} // namespace icancloud
} // namespace inet

#endif
