#ifndef __MVRPDU_H
#define __MVRPDU_H

#include <omnetpp.h>
#include "MVRPDU_m.h"

class cDisplayString;



class MVRPDU : public MVRPDU_Base
{
    public:
      MVRPDU();
      virtual ~MVRPDU(){}
      MVRPDU(const MVRPDU& other) : MVRPDU_Base() {operator=(other);}
      MVRPDU& operator=(const MVRPDU& other) {MVRPDU_Base::operator=(other); return *this;}
      // ADD CODE HERE to redefine and implement pure virtual functions from BPDU_Base
      virtual MVRPDU *dup() const {return new MVRPDU(*this);}
};

#endif
