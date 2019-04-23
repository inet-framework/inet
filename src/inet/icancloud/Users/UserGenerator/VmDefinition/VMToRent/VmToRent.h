//
// Module that implements a virtual machine definition that tenants can rent.
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

#ifndef VMTORENT_H_
#define VMTORENT_H_

#include <omnetpp.h>

namespace inet {

namespace icancloud {

using namespace omnetpp;
class VmToRent: public cSimpleModule{
protected:
    virtual ~VmToRent();
    virtual void initialize() override;
    
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override {};
};

} // namespace icancloud
} // namespace inet

#endif /* VMTORENT_H_ */
