// (C) 2012 OpenSim Ltd

// OLSR packet plan for a new implementation


#ifndef __INET_OLSRPACKETNEW_H
#define __INET_OLSRPACKETNEW_H

#include "INETDefs.h"
#include "OlsrPacketNew_m.h"

class INET_API OlsrPacket : public OlsrPacket_Base
{
  protected:
    typedef std::vector<OlsrMsg*> OlsrMsgArray;
    OlsrMsgArray olsrMsgArray;

  private:
    void copy(const OlsrPacket& other);

  public:
    OlsrPacket(const char *name=NULL, int kind=0) : OlsrPacket_Base(name,kind) {}
    OlsrPacket(const OlsrPacket& other) : OlsrPacket_Base(other) {copy(other);}
    ~OlsrPacket();
    OlsrPacket& operator=(const OlsrPacket& other) {if (this==&other) return *this; OlsrPacket_Base::operator=(other); copy(other); return *this;}
    virtual OlsrPacket *dup() const {return new OlsrPacket(*this);}

    /** Generated but unused method, should not be called. */
    virtual void setMsgArraySize(unsigned int size);

    virtual unsigned int getMsgArraySize() const;

    virtual OlsrMsg& getMsg(unsigned int k);

    /** Generated but unused method, should not be called. */
    virtual void setMsg(unsigned int k, const OlsrMsg& msg);

    OlsrMsg* addMsg(OlsrMessageType type);
};


#endif // __INET_OLSRPACKETNEW_H

