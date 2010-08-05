
#include "BGPUpdate.h"

Register_Class(BGPUpdate)

void BGPUpdate::setUnfeasibleRoutesLength(unsigned short unfeasibleRoutesLength_var)
{
	unsigned short delta_bytes = unfeasibleRoutesLength_var - getUnfeasibleRoutesLength();
	setBitLength(getLength() + delta_bytes);
}

void BGPUpdate::setWithdrawnRoutesArraySize(unsigned int size)
{
	unsigned short delta_size = size - getWithdrawnRoutesArraySize();
	unsigned short delta_bytes = delta_size * 5; // 5 = Withdrawn Route length
	setUnfeasibleRoutesLength(getUnfeasibleRoutesLength() + delta_bytes);
}

void BGPUpdate::setTotalPathAttributeLength(unsigned short totalPathAttributeLength_var)
{
	unsigned short delta_bytes = totalPathAttributeLength_var - getTotalPathAttributeLength();
	setBitLength(getLength() + delta_bytes);
	BGPUpdate_Base::totalPathAttributeLength_var = totalPathAttributeLength_var;
}

void BGPUpdate::setPathAttributesContent(const BGPUpdatePathAttributesContent& pathAttributesContent_var)
{
	setPathAttributesContentArraySize(1);
	
	unsigned short nb_path_attr = 2 + pathAttributesContent_var.getAsPathArraySize()
		+ pathAttributesContent_var.getLocalPrefArraySize()
		+ pathAttributesContent_var.getAtomicAggregateArraySize();

	// BGPUpdatePathAttributes (4)
	unsigned short contentBytes = nb_path_attr * 4;
	// BGPUpdatePathAttributesOrigin (1)
	contentBytes += 1; 
	// BGPUpdatePathAttributesASPath
	for(unsigned int i=0; i<pathAttributesContent_var.getAsPathArraySize(); i++)
		contentBytes += 2 + pathAttributesContent_var.getAsPath(i).getLength(); // type (1) + length (1) + value
	// BGPUpdatePathAttributesNextHop (4)
	contentBytes += 4; 
	// BGPUpdatePathAttributesLocalPref (4)
	contentBytes = 4 * pathAttributesContent_var.getLocalPrefArraySize(); 

	setTotalPathAttributeLength(contentBytes);

	BGPUpdate_Base::pathAttributesContent_var[0]=pathAttributesContent_var;
}

void BGPUpdate::setNLRI(const BGPUpdateNLRI& NLRI_var)
{
	setBitLength(getLength() + 5); //5 = NLRI (lenght (1) + IPAddress (4))
	BGPUpdate_Base::NLRI_var = NLRI_var;
}

