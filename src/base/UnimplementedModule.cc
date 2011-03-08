//
// This program is property of its copyright holder. All rights reserved.
// 

#include "UnimplementedModule.h"

Define_Module(UnimplementedModule);

void UnimplementedModule::initialize()
{
    // TODO - Generated method body
}

void UnimplementedModule::handleMessage(cMessage *msg)
{
    throw cRuntimeError("Unimplemented module");
}
