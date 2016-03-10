//
// Copyright (C) 2005 Andras Varga,
//                    Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_CSOCKETRTSCHEDULER_H
#define __INET_CSOCKETRTSCHEDULER_H

#define WANT_WINSOCK2

#include <platdep/sockets.h>
#include "inet/common/INETDefs.h"

#if OMNETPP_VERSION < 0x500
#include <platdep/timeutil.h>
#else // OMNETPP_VERSION < 0x500
#include <omnetpp/platdep/timeutil.h>
#endif // OMNETPP_VERSION < 0x500

// prevent pcap.h to redefine int8_t,... types on Windows
#include "inet/common/serializer/headers/bsdint.h"
#define HAVE_U_INT8_T
#define HAVE_U_INT16_T
#define HAVE_U_INT32_T
#define HAVE_U_INT64_T

#include <pcap.h>
#include "inet/linklayer/ext/ExtFrame_m.h"

namespace inet {

class INET_API cSocketRTScheduler : public cScheduler
{
  protected:
    int fd;

    virtual bool receiveWithTimeout(long usec);
    virtual int receiveUntil(const timeval& targetTime);

  public:
    /**
     * Constructor.
     */
    cSocketRTScheduler();

    /**
     * Destructor.
     */
    virtual ~cSocketRTScheduler();
    static std::vector<cModule *> modules;
    static std::vector<pcap_t *> pds;
    static std::vector<int> datalinks;
    static std::vector<int> headerLengths;
    static timeval baseTime;

    /**
     * Called at the beginning of a simulation run.
     */
    virtual void startRun() override;

    /**
     * Called at the end of a simulation run.
     */
    virtual void endRun() override;

    /**
     * Recalculates "base time" from current wall clock time.
     */
    virtual void executionResumed() override;

    /**
     * To be called from the module which wishes to receive data from the
     * socket. The method must be called from the module's initialize()
     * function.
     */
    void setInterfaceModule(cModule *mod, const char *dev, const char *filter);

#if OMNETPP_VERSION >= 0x0500
    /**
     * Returns the first event in the Future Event Set.
     */
    virtual cEvent *guessNextEvent() override;

    /**
     * Scheduler function -- it comes from the cScheduler interface.
     */
    virtual cEvent *takeNextEvent() override;

    /**
     * Scheduler function -- it comes from the cScheduler interface.
     */
    virtual void putBackEvent(cEvent *event) override;
#else // if OMNETPP_VERSION >= 0x0500
      /**
       * Scheduler function -- it comes from cScheduler interface.
       */
    virtual cMessage *getNextEvent() override;
#endif // if OMNETPP_VERSION >= 0x0500

    /**
     * Send on the currently open connection
     */
    void sendBytes(unsigned char *buf, size_t numBytes, struct sockaddr *from, socklen_t addrlen);
};

} // namespace inet

#endif // ifndef __INET_CSOCKETRTSCHEDULER_H

