#pragma once

#include <BGPASPathSegment_m.h>

class BGPASPathSegment : public BGPASPathSegment_Base
{
  public:
    BGPASPathSegment() : BGPASPathSegment_Base() {}
    BGPASPathSegment(const BGPASPathSegment& other) : BGPASPathSegment_Base() {operator=(other);}
    BGPASPathSegment& operator=(const BGPASPathSegment& other) {BGPASPathSegment_Base::operator=(other); return *this;}
	virtual BGPASPathSegment_Base *dup() const {return new BGPASPathSegment(*this);}
    
};
