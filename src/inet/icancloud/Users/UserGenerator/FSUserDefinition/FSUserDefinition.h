//
// Module that implements a file system configuration given by a tenant for executing jobs
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

#ifndef FSUSERDEFINITION_H_
#define FSUSERDEFINITION_H_

#include <omnetpp.h>

namespace inet {

namespace icancloud {

using namespace omnetpp;
class FSUserDefinition: public cSimpleModule{
protected:
    virtual ~FSUserDefinition();
    virtual void initialize() override;
    
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override {};
};

} // namespace icancloud
} // namespace inet

#endif /* FSUSERDEFINITION_H_ */
