#include "inet/icancloud/Base/icancloud_MemoryBlock.h"

namespace inet {

namespace icancloud {



string icancloud_MemoryBlock::getFileName (){
	return fileName;
}
		
	   
void icancloud_MemoryBlock::setFileName (string newName){
	fileName = newName;
}		
	
	   
unsigned int icancloud_MemoryBlock::getOffset (){
	return offset;
}
		
  
void icancloud_MemoryBlock::setOffset (unsigned int newOffset){
	offset = newOffset;
}
		
	   
unsigned int icancloud_MemoryBlock::getBlockSize (){
	return blockSize;
}
		
	   
void icancloud_MemoryBlock::setBlockSize (unsigned int newBlockSize){
	blockSize = newBlockSize;
}
	
	   
bool icancloud_MemoryBlock::getIsPending (){
	return isPending;
}
		
	   
void icancloud_MemoryBlock::setIsPending (bool newIsPending){
	isPending = newIsPending;
}				
		
	   
string icancloud_MemoryBlock::memoryBlockToString (){
	
	std::ostringstream info;
	
		info << "FileName:" << fileName.c_str()
			 << " - Offset:" << offset
			 << " - Size:" << blockSize
			 << " - isPending:" << isPending;
			 
	return info.str();	
}



} // namespace icancloud
} // namespace inet
