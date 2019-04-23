#ifndef _icancloud_MEMORY_BLOCK_H_
#define _icancloud_MEMORY_BLOCK_H_

//#include "stdio.h"
//#include "stdlib.h"
#include "inet/icancloud/Base/include/icancloud_types.h"
#include <omnetpp.h>
#include <string>
#include <vector>

namespace inet {

namespace icancloud {

using std::string;
using std::vector;

/**
 * @class icancloud_MemoryBlock icancloud_MemoryBlock.h "Base/icancloud_MemoryBlock.h"
 *
 * Base class that contains a cache block.
 *  
 */
class icancloud_MemoryBlock{
	
	private:	
		   	
		/** File Name */
		string fileName;
				
		/** File Size (in bytes) */
		unsigned int offset;		
		
		/** BlockSize */
		unsigned int blockSize;	
		
		/** is Pending? */
		bool isPending;
	
	public:
	
	
	   /**
		* Gets the file name
		* @return File name
		*/
		string getFileName ();
		
	   /**
		* Sets the file name
		* @param newName File name
		*/
		void setFileName (string newName);		
			
	
	   /**
		* Gets the offset
		* @return offset (in bytes)
		*/
		unsigned int getOffset ();
		
	   /**
		* Sets the offset
		* @param newOffset (in bytes)
		*/
		void setOffset (unsigned int newOffset);
		
		
	   /**
		* Gets the blockSize
		* @return blockSize (in bytes)
		*/
		unsigned int getBlockSize ();
		
	   /**
		* Sets the blockSize
		* @param newBlockSize (in bytes)
		*/
		void setBlockSize (unsigned int newBlockSize);
		
		
	   /**
		* Is a pending block?
		* @return true if current block is pending or falsein another case.
		*/
		bool getIsPending ();
		
		
	   /**
		* Sets the block state
		* @param newIsPending Block state
		*/
		void setIsPending (bool newIsPending);				
		
		
	   /**
		* Parses the cache block info to string. Debug purpose only!
		* 
		*/
		string memoryBlockToString ();  			 			
};

} // namespace icancloud
} // namespace inet

#endif /*_icancloud_MemoryBlock_H_*/
