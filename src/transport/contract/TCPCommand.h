//
//

#ifndef __INET_TCPCOMMAND_H
#define __INET_TCPCOMMAND_H_

#include <omnetpp.h>
#include "TCPCommand_m.h"


/**
 * Class generated from <tt>transport/contract/TCPCommand.msg</tt> by opp_msgc.
 * <pre>
 * class TCPDataInfo extends TCPCommand
 * {
 *     cPacketPtr dataObject;
 *     bool isBegin;	
 * }
 * </pre>
 */
 class TCPDataMsg : public TCPDataMsg_Base
{
  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const TCPDataMsg&);

  public:
    TCPDataMsg(const char *name=NULL);
    TCPDataMsg(const TCPDataMsg& other);
    virtual ~TCPDataMsg();
    TCPDataMsg& operator=(const TCPDataMsg& other);
    virtual TCPDataMsg *dup() const {return new TCPDataMsg(*this);}

    virtual cPacket* removeDataObject();
};

inline void doPacking(cCommBuffer *b, TCPDataMsg& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, TCPDataMsg& obj) {obj.parsimUnpack(b);}


#endif // __INET_TCPCOMMAND_H_
