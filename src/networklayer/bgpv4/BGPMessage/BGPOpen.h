#pragma once

#include <BGPOpen_m.h>

class BGPOpenMessage : public BGPOpenMessage_Base
{
public:
	BGPOpenMessage(const char *name="BGPOpenMessage", int kind=0) : BGPOpenMessage_Base(name,kind)
	{
		setType(BGP_OPEN);
		setBitLength(BGP_HEADER_OCTETS + BGP_OPEN_OCTETS);
	}

	BGPOpenMessage(const BGPOpenMessage& other) : BGPOpenMessage_Base(other.getName()) {operator=(other);}
	BGPOpenMessage& operator=(const BGPOpenMessage& other) {BGPOpenMessage_Base::operator=(other); return *this;}
	virtual BGPOpenMessage_Base *dup() const {return new BGPOpenMessage(*this);}

	void setBitLength(unsigned short length_var)
	{
		BGPHeader::setLength(length_var);
		setByteLength(length_var);
	}

	static const int BGP_OPEN_OCTETS = 10;
};
