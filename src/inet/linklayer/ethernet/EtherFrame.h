#ifndef __INET_ETHERFRAME_H
#define __INET_ETHERFRAME_H

#include "inet/linklayer/ethernet/EtherFrame_m.h"

namespace inet {

/**
 * Represents an Ethernet PHY frame. See EtherFrame.msg for details.
 */
class EtherPhyFrame : public EtherPhyFrame_Base
{
  private:
    void copy(const EtherPhyFrame& other) {}

  public:
    EtherPhyFrame(const char *name=nullptr, int kind=0) : EtherPhyFrame_Base(name,kind) {}
    EtherPhyFrame(const EtherPhyFrame& other) : EtherPhyFrame_Base(other) {copy(other);}
    EtherPhyFrame& operator=(const EtherPhyFrame& other) {if (this==&other) return *this; EtherPhyFrame_Base::operator=(other); copy(other); return *this;}
    virtual EtherPhyFrame *dup() const override {return new EtherPhyFrame(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from EtherPhyFrame_Base
    virtual const char *getDisplayString() const override { return hasEncapsulatedPacket() ? const_cast<EtherPhyFrame *>(this)->getEncapsulatedPacket()->getDisplayString() : EtherPhyFrame_Base::getDisplayString(); }
};

} // namespace inet

#endif // ifndef __INET_ETHERFRAME_H
