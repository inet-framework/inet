#pragma once

#include <BGPKeepAlive_m.h>
 
 class BGPKeepAliveMessage : public BGPKeepAliveMessage_Base
 {
public:
    BGPKeepAliveMessage(const char *name="BGPKeepAlive", int kind=0) : BGPKeepAliveMessage_Base(name,kind) 
    {
        setType(BGP_KEEPALIVE);
		setBitLength(BGP_HEADER_OCTETS);
    }
    
    BGPKeepAliveMessage(const BGPKeepAliveMessage& other) : BGPKeepAliveMessage_Base(other.getName()) {operator=(other);}
    BGPKeepAliveMessage& operator=(const BGPKeepAliveMessage& other) {BGPKeepAliveMessage_Base::operator=(other); return *this;}
    virtual BGPKeepAliveMessage_Base *dup() const {return new BGPKeepAliveMessage(*this);}
  
	void setBitLength(unsigned short length_var)
	{
		BGPHeader::setLength(length_var);
		setByteLength(length_var);
	}
 };