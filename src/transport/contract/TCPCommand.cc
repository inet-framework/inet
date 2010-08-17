//
//

#include "TCPCommand.h"
/*
* class TCPDataMsg : public TCPDataMsg_Base
 * {
 *   public:
 *     TCPDataMsg& operator=(const TCPDataMsg& other) {TCPDataMsg_Base::operator=(other); return *this;}
 *     virtual TCPDataMsg *dup() const {return new TCPDataMsg(*this);}
 *     // ADD CODE HERE to redefine and implement pure virtual functions from TCPDataMsg_Base
 * };
 */

TCPDataMsg::TCPDataMsg(const char *name)
    : TCPDataMsg_Base(name)
{
    dataObject_var = NULL;
    isBegin_var = false;
}

TCPDataMsg::TCPDataMsg(const TCPDataMsg& other) : TCPDataMsg_Base(other.getName())
{
    operator=(other);
}

TCPDataMsg::~TCPDataMsg()
{
    delete dataObject_var;
}

TCPDataMsg& TCPDataMsg::operator=(const TCPDataMsg& other)
{
    if (this==&other)
        return *this;
    TCPDataMsg_Base::operator=(other);
    this->dataObject_var = other.dataObject_var->dup();
    return *this;
}

cPacket* TCPDataMsg::removeDataObject()
{
    cPacket* ret = dataObject_var;
    dataObject_var = NULL;
    return ret;
}
