#pragma once

#include <BGPUpdate_m.h>

class BGPUpdate : public BGPUpdate_Base
{
public:
	BGPUpdate(const char *name="BGPUpdate", int kind=0) : BGPUpdate_Base(name,kind)
	{
		setType(BGP_UPDATE);
		setBitLength(BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS);
	}

	BGPUpdate(const BGPUpdate& other) : BGPUpdate_Base(other.getName()) {operator=(other);}
	BGPUpdate& operator=(const BGPUpdate& other) {BGPUpdate_Base::operator=(other); return *this;}
	virtual BGPUpdate_Base *dup() const {return new BGPUpdate(*this);}
	
	void setBitLength(unsigned short length_var)
	{
		BGPHeader::setLength(length_var);
		setByteLength(length_var);
	}

	void setUnfeasibleRoutesLength(unsigned short unfeasibleRoutesLength_var);
	void setWithdrawnRoutesArraySize(unsigned int size);
	
	void setTotalPathAttributeLength(unsigned short totalPathAttributeLength_var);
	void setPathAttributesContent(const BGPUpdatePathAttributesContent& pathAttributesContent_var);

	void setNLRI(const BGPUpdateNLRI& NLRI_var);

	static const int BGP_EMPTY_UPDATE_OCTETS = 4; // UnfeasibleRoutesLength (2) + TotalPathAttributeLength (2)

};


