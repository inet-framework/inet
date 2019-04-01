
#include "inet/icancloud/Base/Messages/icancloud_File.h"

namespace inet {

namespace icancloud {


using namespace omnetpp;


icancloud_File::~icancloud_File(){	
	branchList.clear();	
}


unsigned int icancloud_File::getFileSize () const {
	return fileSize;
}


void icancloud_File::setFileSize (unsigned int newSize){
	fileSize = newSize;
}


void icancloud_File::updateFileSize (){
	setFileSize (getTotalSectors()*BYTES_PER_SECTOR);
}


string icancloud_File::getFileName () const  {
	return fileName;
}


void icancloud_File::setFileName (string newName){
	fileName = newName;
}


unsigned int icancloud_File::getNumberOfBranches () const {
	return (branchList.size());
}


size_blockList_t icancloud_File::getBranchSize (unsigned int branchNumber) const {

	if (branchNumber < getNumberOfBranches())
		return (branchList[branchNumber].numBlocks);
	else
		throw cRuntimeError("[icancloud_File.getBranchSize] Branch out of bounds!");
}


off_blockList_t icancloud_File::getBranchOffset (unsigned int branchNumber) const {

	if (branchNumber < getNumberOfBranches())
		return (branchList[branchNumber].offset);
	else
		throw cRuntimeError("[icancloud_File.getBranchOffset] Branch out of bounds! Existing:%d Requested:%d",
								getNumberOfBranches(), branchNumber);
}


void icancloud_File::addBranch (off_blockList_t offset, size_blockList_t size){

	fileBranch newBranch;

		newBranch.offset = offset;
		newBranch.numBlocks = size;
		branchList.push_back (newBranch);
		
		// Update file size
		updateFileSize();
}


void icancloud_File::addBlock (off_blockList_t sector, unsigned int blockSize){
	
	size_blockList_t blockNumber;
	size_blockList_t sectorsPerBlock;
	
		sectorsPerBlock = blockSize/BYTES_PER_SECTOR;
		blockNumber = sector/sectorsPerBlock; 
	
	addBranch (blockNumber*sectorsPerBlock, sectorsPerBlock);
	
	// Update file size
	updateFileSize();
}


size_blockList_t icancloud_File::getTotalSectors () const {

	//vector <fileBranch>::iterator file_it;
	size_blockList_t numBlocks;

		// Init
		numBlocks = 0;

		for (auto file_it=branchList.begin(); file_it!=branchList.end(); ++file_it){
			numBlocks+=(*file_it).numBlocks;
		}

	return numBlocks;
}


bool icancloud_File::containThisSector (size_blockList_t sector) const {
	
	int i;
	bool found;
	
		// init
		found = false;
		i = 0;
	
		while ((i< ((int)branchList.size())) && (!found)){
			
			if ((sector >= branchList[i].offset) &&
			   (sector < branchList[i].offset + branchList[i].numBlocks))
			   	found = true;
			else
				i++;			
		}
		
	return found;
}


void icancloud_File::clearBranchList(){
	branchList.clear();
	updateFileSize();
}


string icancloud_File::contentsToString (bool withBranches, bool printContents) const {

	std::ostringstream osStream;
	unsigned int i;
		
		if (printContents){
		
			osStream 	<< " FileName:" << getFileName()
						<< " Size:" << getFileSize()
						<< " Sectors:" << getTotalSectors()					
						<< " NumBranches:" << getNumberOfBranches()
						<< endl;
	
			// Detailes info?
			if (withBranches){
				for (i=0; i<getNumberOfBranches(); i++)
					osStream  << "   Branch[" << i << "]  offset:" << branchList[i].offset << "   branchSize:" << branchList[i].numBlocks << endl;
			}
		}

	return osStream.str();
}



} // namespace icancloud
} // namespace inet
