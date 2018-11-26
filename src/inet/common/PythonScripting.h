//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_PYTHONSCRIPTING_H_
#define __INET_PYTHONSCRIPTING_H_

#include "INETDefs.h"


#include <Python.h>

using namespace omnetpp;

namespace inet {

/**
 * TODO - Generated class
 */
class PythonScripting : public cSimpleModule
{
    cMessage *msg;
    PyObject *m, *d, *l;
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

} //namespace

#endif
