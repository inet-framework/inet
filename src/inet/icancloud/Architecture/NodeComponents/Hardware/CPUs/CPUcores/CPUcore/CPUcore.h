#ifndef _CPU_CORE_H_
#define _CPU_CORE_H_

#include "inet/icancloud/Architecture/NodeComponents/Hardware/CPUs/CPUcores/ICore.h"

/**
 * @class CPUcore CPUcore.h "CPUcore.h"
 *   
 * CPU Core Module
 * 
 * @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
 * @date 2012-23-11
 */
//-------------------------------------- CPU C-STATES ------------------------------------ //

#define     OFF                         std::string("off")                           // The cpu is completely off.
#define     C0_OperatingState           std::string("c0_operating_state")            // The cpu is fully turned on (Here actuates the P-states)
#define     C1_Halt                     std::string("c1_halt")                       // Stops CPU main internal clocks via software; bus interface unit and APIC are kept running at full speed.
#define     C2_StopGrant                std::string("c2_stop_grant")                 // Stops CPU main internal clocks via hardware and reduces CPU voltage; bus interface unit and APIC are kept running at full speed.
#define     C3_Sleep                    std::string("c3_sleep")                      // Stops all CPU internal and external clocks
#define     C4_DeeperSleep              std::string("c4_deeper_sleep")               // Reduces CPU voltage
#define     C5_EnhancedDeeperSleep      std::string("c5_enhanced_deeper_sleep")      // Reduces CPU voltage even more and turns off the memory cache
//#define     C6_DeepPowerDown            std::string("c6_deep_power_down")            // Reduces the CPU internal voltage to any value, including 0 V

//-------------------------------------- CPU P-STATES ------------------------------------ //

// P-States are 12. As example, the processor is: Intel(R) Core(TM) i3 CPU  M 370  @ 2.40GHz
//cpufreq stats: 2.40 GHz 2.27 GHz 2.13 GHz 2.00 GHz 1.87 GHz 1.73 GHz 1.60 GHz 1.47 GHz 1.33 GHz 1.20 GHz 1.07 GHz 933 MHz

#define     C0_P0                       std::string("c0_p0")                         // 2.40 GHz. The maximum speed.
#define     C0_P1                       std::string("c0_p1")                         // 2.27 GHz
#define     C0_P2                       std::string("c0_p2")                         // 2.13 GHz
#define     C0_P3                       std::string("c0_p3")                         // 2.00 GHz
#define     C0_P4                       std::string("c0_p4")                         // 1.87 GHz
#define     C0_P5                       std::string("c0_p5")                         // 1.73 GHz
#define     C0_P6                       std::string("c0_p6")                         // 1.60 GHz
#define     C0_P7                       std::string("c0_p7")                         // 1.47 GHz
#define     C0_P8                       std::string("c0_p8")                         // 1.33 GHz
#define     C0_P9                       std::string("c0_p9")                         // 1.20 GHz
#define     C0_P10                      std::string("c0_p10")                        // 1.07 GHz
#define     C0_P11                      std::string("c0_p11")                        // 933 GHz

namespace inet {

namespace icancloud {


class CPUcore : public ICore{

	protected:		

    /** Currently tick time (in seconds) */
    double current_speed = 0;

    /** Time to finalize the processing of the message*/
    simtime_t currentTime;

    /** Flag to control when the message is new or is one that is been processed*/
    bool executingMessage = false;

        virtual ~CPUcore();

        

       /**
        *  Module initialization.
        */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	    
	   /**
	 	* Module ending.
	 	*/ 
        virtual void finish() override;
	    
	    
	private:
	
	   /**
		* Get the outGate ID to the module that sent <b>msg</b>
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
		* Calculates the amount of time to execute completely the current computing block.
		* @param reainingMIs Million instructions to be executed.
		* @return Time to execute reainingMIs instructions.
		*/
		simtime_t getTimeToExecuteCompletely (long int reainingMIs) override ;
		
	   /**
		* Calculates the amount of time to execute completely the current computing block.
		* @param reainingTime Amount of time for executing current CB.
		* @return Amount of CPU time to execute reainingTime.
		*/
		simtime_t getMaximumTimeToExecute (simtime_t reainingTime) override ;

		/*
		 * Calculates the number of instructions per second depending on the tick_s parameter
		 *
		 */
		void calculateIPT (double current_speed){ipt = (long int) (current_speed * tick_s.dbl());};


	public:
		/*
		 * Change the energy state of the memory given by node state
		 */
		void changeDeviceState (const string & nodeState, unsigned componentIndex = 0) override ;

		/*
		 * Change the energy state of the processor!
		 */
		void changeState (const string & energyState, unsigned componentIndex = 0) override ;
};

} // namespace icancloud
} // namespace inet

#endif
