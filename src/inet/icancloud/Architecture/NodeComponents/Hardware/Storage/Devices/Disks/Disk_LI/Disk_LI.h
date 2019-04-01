#ifndef _DISK_LI_H_
#define _DISK_LI_H_


#include "inet/icancloud/Architecture/NodeComponents/Hardware/Storage/Devices/IStorageDevice.h"

/** Number of branch sizes (in number of blocks) */
#define NUM_BLOCK_SIZES 19

/** Number of jump sizes */
#define NUM_JUMP_SIZES 9

/** MB BlockSize Index */
#define MB_BLOCKSIZE_INDEX 11

/** 32KB Jump index */
#define KB32_JUMP_INDEX 4


//----------------------------------------- DISK STATES ----------------------------------------- //

#define     DISK_ON                     "disk_on"                       // Disk ON
#define     DISK_OFF                    "disk_off"                      // Disk OFF
#define     DISK_ACTIVE                 "disk_active"                       // Disk ACTIVE
#define     DISK_IDLE                   "disk_idle"                     // Disk IDLE

namespace inet {

namespace icancloud {



/**
 * @class Disk_LI Disk_LI.h "Disk_LI.h"
 *
 * Class that simulates disk of 400GB using Lineal Interpolation
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-11
 *
 * @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
 * @date 2012-23-11
 */
class Disk_LI: public IStorageDevice{

	protected:

		/** Last accessed block */
		off64_T lastBlock = 0;

		/** Gate ID. Input gate. */
		cGate* inGate = nullptr;

		/** Gate ID. Output gate. */
		cGate* outGate = nullptr;

		/** Block Sizes array (in bytes) */
		static int64 blockSizes [NUM_BLOCK_SIZES];

		/** Jump Sizes array (in bytes) */
		static int64 jumpsSizes [NUM_JUMP_SIZES];

		/** Read times to Lineal Interpolation */
		static const const_simtime_t readTimes [NUM_BLOCK_SIZES] [NUM_JUMP_SIZES];

		/** Write times to Lineal Interpolation */
		static const const_simtime_t writeTimes [NUM_BLOCK_SIZES] [NUM_JUMP_SIZES];

		/** Read bandwidth */
	  	unsigned int readBandwidth = 0;

	  	/** Write bandwidth */
	  	unsigned int writeBandwidth = 0;

		/** Request time. */
	  	simtime_t requestTime;

		/** Pending message */
	    Packet *pendingMessage = nullptr;

	    /** Node state */
	    string nodeState = MACHINE_STATE_OFF;

	  /**
	   * Destructor
	   */
	   ~Disk_LI();

	   

      /**
       *  Module initialization.
       */
       void initialize(int stage) override;


	  /**
	   * Module ending.
	   */
	   void finish() override;


	private:


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
 		* Method that calculates the spent time to process the current request.
 		* @param sm Message that contains the current I/O request.
 		* @return Spent time to process the current request.
 		*/
		simtime_t service (Packet *) override;

	   /**
		* Method that implements the Disk simulation equation.
		*
		* @param offsetBlock Current offset (in blocks).
		* @param brachSize Number of blocks involved on current operation.
		* @param operation Operation type.
		* @return Spent time to perform the corresponding I/O operation.
		*/
		simtime_t storageDeviceTime (off_blockList_t offsetBlock, size_blockList_t brachSize, char operation);

	    /*
		 * Caculates coordinates to calculate transfer time using Lineal Interpolation
		 * @param jump Distance between last accessed block and current.
		 * @param numBlocks number of blocks of current request.
		 * @param x1 X index of first coordinate.
		 * @param y1 Y index of first coordinate.
		 * @param x2 X index of second coordinate.
		 * @param y2 Y index of second coordinate.
		 *
		 */
		void calculateIndex (int64 jump, int64 numBytes, int *blockSizeIndex_1, int *blockSizeIndex_2, int *offsetIndex_1, int *offsetIndex_2);


		/**
		 * Function to calculate Lineal interpolation
		 *
		 */
		simtime_t linealInterpolation (int64 x, int64 x0, int64 x1, simtime_t y0, simtime_t y1);


		/*
		 * Change the energy state of the storage given by node state
		 */
		void changeDeviceState (const string & state, unsigned componentIndex) override;
		void changeDeviceState (const string & state) override {changeDeviceState(state, 0);}
		/*
		 * Change the energy state of the disk
		 */
		void changeState (const string & energyState, unsigned componentIndex) override;
		void changeState (const string & energyState) override {changeState(energyState, 0);}
};


int64 Disk_LI::blockSizes [19] = {512, 1024 , 2048 , 4096 , 8192 , 16384 , 32768 , 65536 , 131072 , 262144 , 524288 , 1048576 , 2097152 , 4194304 , 8388608 , 16777216 , 33554432 , 67108864 , 134217728 };
int64 Disk_LI::jumpsSizes [9] = {0, 512 , 1024 , 4096 , 32768 , 131072 , 1048576 , 10485760 , 104857600 };


const const_simtime_t Disk_LI::readTimes [19] [9] = {
	{ 0.00000062, 0.00000063, 0.00000063, 0.00000063, 0.00000063, 0.00000063, 0.00000063, 0.00000063, 0.00000063}, // 512
	{ 0.00000075, 0.00000075, 0.00000075, 0.00000078, 0.00000073, 0.00000075, 0.00000074, 0.00000075, 0.00000078}, // 1 KB	
	{ 0.00000134, 0.00000149, 0.00000140, 0.00000134, 0.00000144, 0.00000134, 0.00000142, 0.00000134, 0.00000138}, // 2 KB	
	{ 0.00000189, 0.00000170, 0.00000189, 0.00000177, 0.00000182, 0.00000184, 0.00000189, 0.00000201, 0.00000180}, // 4 KB		
	{ 0.00000210, 0.00000222, 0.00000249, 0.00000222, 0.00000220, 0.00000231, 0.00000203, 0.00000210, 0.00000240}, // 8 KB
	{ 0.00000406, 0.00000410, 0.00000411, 0.00000408, 0.00000409, 0.00000412, 0.00000414, 0.00000413, 0.00000409}, // 16 KB		
	{ 0.00000735, 0.00000739, 0.00000741, 0.00000742, 0.00000735, 0.00000737, 0.00000739, 0.00000742, 0.00000740}, // 32 KB	
	{ 0.00001335, 0.00001338, 0.00001340, 0.00001341, 0.00001342, 0.00001343, 0.00001342, 0.00001342, 0.00001339}, // 64 KB		
	{ 0.00002750, 0.00002751, 0.00002753, 0.00002757, 0.00002754, 0.00002764, 0.00002769, 0.00002698, 0.00002780}, // 128 KB		
	{ 0.0005400, 0.0005413, 0.0005423, 0.0005414, 0.0005416, 0.0005436, 0.0005436, 0.0005485, 0.0005474}, // 256 KB	
	{ 0.0011213, 0.0011265, 0.0011256, 0.0011286, 0.0011301, 0.0011305, 0.0011315, 0.0011325, 0.0011336}, // 512 KB					
	{ 0.0021445, 0.0021451, 0.0021452, 0.0021449, 0.0021456, 0.0021501, 0.0021520, 0.0021545, 0.0021876}, // 1 MB		
	{ 0.0040345, 0.0040364, 0.0040375, 0.0040378, 0.0040379, 0.0040396, 0.0040385, 0.0040355, 0.0040399}, // 2 KB	
	{ 0.0076035, 0.0076085, 0.0076265, 0.0076300, 0.0076135, 0.0076235, 0.0076564, 0.0076647, 0.0076764},  // 4 MB	
	{ 0.0142343, 0.0141243, 0.0150123, 0.0148753, 0.0145563, 0.0156664, 0.0144445, 0.0143993, 0.0143335}, // 8 MB
	{ 0.0290983, 0.0288334, 0.0290998, 0.0290944, 0.0295553, 0.0295543, 0.0309923, 0.0304453, 0.0293334}, // 16 MB
	{ 0.0618734, 0.0614557, 0.0644456, 0.0614457, 0.0612246, 0.0619964, 0.0614468, 0.0634257, 0.0635568}, // 32 MB
	{ 0.1246785, 0.1254673, 0.1355468, 0.1376743, 0.1477376, 0.1345578, 0.1356647, 0.1265656, 0.1453676}, // 64 MB
	{ 0.2566467, 0.2578443, 0.2655788, 0.2863576, 0.2763657, 0.2877647, 0.2567875, 0.2753576, 0.2976468}  // 128 MB
	};	
	
	
const const_simtime_t Disk_LI::writeTimes [19] [9] = {
	{ 0.0000071, 0.0000073, 0.0000077, 0.0000071, 0.0000074, 0.0000069, 0.0000080, 0.0000078, 0.0000075}, // 512	 
	{ 0.0000078, 0.0000078, 0.0000077, 0.0000078, 0.0000078, 0.0000073, 0.0000076, 0.0000082, 0.0000075}, // 1 KB	
	{ 0.0000082, 0.0000083, 0.0000084, 0.0000084, 0.0000080, 0.0000082, 0.0000081, 0.0000086, 0.0000085}, // 2 KB		
	{ 0.0000071, 0.0000072, 0.0000079, 0.0000092, 0.0000085, 0.0000110, 0.0000095, 0.0000076, 0.0000103}, // 4 KB	
	{ 0.0000115, 0.0000110, 0.0000115, 0.0000112, 0.0000116, 0.0000119, 0.0000111, 0.0000119, 0.0000113}, // 8 KB
	{ 0.0000213, 0.0000219, 0.0000210, 0.0000218, 0.0000221, 0.0000222, 0.0000220, 0.0000219, 0.0000215}, // 16 KB		
	{ 0.0000351, 0.0000362, 0.0000355, 0.0000349, 0.0000360, 0.0000368, 0.0000355, 0.0000359, 0.0000353}, // 32 KB	
	{ 0.0000628, 0.0000632, 0.0000637, 0.0000630, 0.0000630, 0.0000625, 0.0000634, 0.0000631, 0.0000634}, // 64 KB	
	{ 0.0008520, 0.0008589, 0.0008499, 0.0008526, 0.0008623, 0.0008723, 0.0008601, 0.0008579, 0.0008543}, // 128 KB		
	{ 0.0017244, 0.0017342, 0.0017442, 0.0017343, 0.0017441, 0.0018003, 0.0018101, 0.0017983, 0.0017354}, // 256 KB		
	{ 0.0035841, 0.0035854, 0.0035898, 0.0035832, 0.0035901, 0.0035951, 0.0035922, 0.0035855, 0.0035840}, // 512 KB	
	{ 0.0064673, 0.0064701, 0.0064654, 0.0064600, 0.0065073, 0.0064604, 0.0064354, 0.0064301, 0.0064254}, // 1 MB	
	{ 0.0124346, 0.0124323, 0.0124606, 0.0124989, 0.0123998, 0.0124001, 0.0124767, 0.0124542, 0.0124533}, // 2 MB		
	{ 0.0231692, 0.0231674, 0.0231699, 0.0231703, 0.0231776, 0.0231798, 0.0231804, 0.0231745, 0.0231545}, // 4 MB	
	{ 0.0417728 , 0.0402334, 0.0423986, 0.0446549, 0.0433413, 0.0387546, 0.0417728, 0.0417728, 0.0417728}, // 8 MB	
	{ 0.0792544 , 0.0787637, 0.0776567, 0.0795239, 0.0812387, 0.0840973, 0.0792564, 0.0812135, 0.0808132}, // 16 MB
	{ 0.1584295 , 0.1501243, 0.1601232, 0.1521954, 0.1543785, 0.1543278, 0.1523432, 0.1598704, 0.1606581}, // 32 MB
	{ 0.3032347 , 0.3197864, 0.3546743, 0.3123877, 0.3223879, 0.3094356, 0.3137578, 0.3346646, 0.3235467}, // 64 MB
	{ 0.6176840 , 0.6343576, 0.6546745, 0.6345775, 0.6345656, 0.6097654, 0.6345671, 0.6346847, 0.6333567}  // 128 MB
	};

} // namespace icancloud
} // namespace inet

#endif
