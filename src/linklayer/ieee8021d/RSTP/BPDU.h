 /**
******************************************************
* @file BPDU.h
* @brief RSTP BPDU modification
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __BPDU_H
#define __BPDU_H

#include <omnetpp.h>
#include "BPDU_m.h"

class cDisplayString;



class BPDUieee8021D : public BPDUieee8021D_Base
{
    public:
      BPDUieee8021D(); //: BPDU_Base() {}
      virtual ~BPDUieee8021D() {}
      BPDUieee8021D(const BPDUieee8021D& other) : BPDUieee8021D_Base() {operator=(other);}
      BPDUieee8021D& operator=(const BPDUieee8021D& other) {BPDUieee8021D_Base::operator=(other); return *this;}
      // ADD CODE HERE to redefine and implement pure virtual functions from BPDU_Base
      virtual BPDUieee8021D *dup() const {return new BPDUieee8021D(*this);}
};

#endif
