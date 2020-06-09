//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_EXTAPP_H
#define __INET_EXTAPP_H

#include "inet/common/INETDefs.h"
#include "inet/common/LinuxUtils.h"
#include "inet/common/NetworkNamespaceContext.h"

#include <signal.h>

namespace inet {

class INET_API ExtApp : public cSimpleModule
{
    int pid = -1;
  public:

    int numInitStages() const override { return NUM_INIT_STAGES; }

    void initialize(int stage) override;

    void finish() override {
        if (pid >= 0) {
            kill(pid, SIGTERM); // SIGKILL?
            pid = -1;
        }
    }

    void handleMessage(cMessage *msg) override {
        ASSERT(false);
    }

    virtual ~ExtApp() {
        if (pid >= 0) {
            kill(pid, SIGTERM); // SIGKILL?
            pid = -1;
        }
    }

};

} // namespace inet

#endif // ifndef __INET_EXTAPP_H

