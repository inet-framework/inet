//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */
#ifndef __INET_EIGRPMSGREQ_H
#define __INET_EIGRPMSGREQ_H

#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
namespace inet {
class INET_API EigrpMsgReq : public EigrpMsgReq_Base
{
  public:
    EigrpMsgReq(const char *name = nullptr) : EigrpMsgReq_Base(name) {}
    EigrpMsgReq(const EigrpMsgReq& other) : EigrpMsgReq_Base(other) {}
    EigrpMsgReq& operator=(const EigrpMsgReq& other)
    { EigrpMsgReq_Base::operator=(other); return *this; }
    virtual EigrpMsgReq *dup() const { return new EigrpMsgReq(*this); }
    bool isMsgReliable() { return getOpcode() != EIGRP_HELLO_MSG; }
    int findMsgRoute(int routeId) const;
};

//Register_Class(EigrpMsgReq);
} // namespace inet
#endif

