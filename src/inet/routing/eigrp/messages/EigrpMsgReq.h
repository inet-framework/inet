//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */
#ifndef EIGRPMSGREQ_H_
#define EIGRPMSGREQ_H_

#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
namespace inet {
class EigrpMsgReq : public EigrpMsgReq_Base
{
  public:
    EigrpMsgReq(const char *name=NULL) : EigrpMsgReq_Base(name) { }
    EigrpMsgReq(const EigrpMsgReq& other) : EigrpMsgReq_Base(other) {}
    EigrpMsgReq& operator=(const EigrpMsgReq& other)
        {EigrpMsgReq_Base::operator=(other); return *this;}
    virtual EigrpMsgReq *dup() const {return new EigrpMsgReq(*this);}
    bool isMsgReliable() { return getOpcode() != EIGRP_HELLO_MSG; }
    int findMsgRoute(int routeId) const;
};

//Register_Class(EigrpMsgReq);
}
#endif /* EIGRPMSGREQ_H_ */
