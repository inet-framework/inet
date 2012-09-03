// (C) 2012 OpenSim Ltd

// OLSR packet plan for a new implementation


#include "OlsrPacketNew.h"


Register_Class(OlsrPacket);


OlsrPacket::~OlsrPacket()
{
    for (OlsrMsgArray::const_iterator i = olsrMsgArray.begin(); i != olsrMsgArray.end(); ++i)
        dropAndDelete(*i);
}

void OlsrPacket::copy(const OlsrPacket& other)
{
    for (OlsrMsgArray::const_iterator i = other.olsrMsgArray.begin(); i != other.olsrMsgArray.end(); ++i)
    {
        OlsrMsg *msg = (*i)->dup();
        take(msg);
        olsrMsgArray.push_back(msg);
    }
}

void OlsrPacket::setMsgArraySize(unsigned int size)
{
    throw cRuntimeError("setMsgArraySize() not supported, use addMsg()");
}

unsigned int OlsrPacket::getMsgArraySize() const { return olsrMsgArray.size(); }

OlsrMsg& OlsrPacket::getMsg(unsigned int k)
{
    ASSERT(k < olsrMsgArray.size());
    return *(olsrMsgArray[k]);
}

void OlsrPacket::setMsg(unsigned int k, const OlsrMsg& msg)
{
    throw cRuntimeError("setMsg() not supported, use addMsg()");
}

OlsrMsg* OlsrPacket::addMsg(OlsrMessageType type)
{
    OlsrMsg *msg = NULL;
    switch (type) {
        case OLSR_HELLO_MSG: msg = new OlsrHelloMsg(); break;
        case OLSR_TC_MSG: msg = new OlsrTcMsg(); break;
        case OLSR_MID_MSG: msg = new OlsrMidMsg(); break;
        case OLSR_HNA_MSG: msg = new OlsrHnaMsg(); break;
        default: throw cRuntimeError("Invalid OlsrMessageType value: %d", type); break;
    }
    take(msg);
    olsrMsgArray.push_back(msg);
    return msg;
}

