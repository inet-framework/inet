//
// This class defines a Virtual Machine Image.
//
// A virtual machine image is defined to iCanCloud simulator as a type of machine without physical resources.
// The physical resources are managed by hypervisor and the virtual machine is linked to the hypervisor in order
// to perform those tasks.
// The main parameters that define a virtual machine are:
//      - identification
//      - numCores
//      - memorySize_MB
//      - storageSize_GB
//
// This module is used to storage the parameters to be defined at omnetpp.ini defined by cloud provider to be rented by tenants
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//


#ifndef VmImage_H_
#define VmImage_H_

#include <omnetpp.h>

namespace inet {

namespace icancloud {


using namespace omnetpp;

class VmImage: public cSimpleModule{
protected:
    virtual ~VmImage();
    virtual void initialize() override;
    
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override {};
};

} // namespace icancloud
} // namespace inet

#endif /* FSUSERDEFINITION_H_ */
