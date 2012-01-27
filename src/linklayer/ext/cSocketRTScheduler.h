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

#ifndef __CSOCKETRTSCHEDULER_H__
#define __CSOCKETRTSCHEDULER_H__

#define WANT_WINSOCK2

#include <platdep/sockets.h>
#include <platdep/timeutil.h>
#include "INETDefs.h"

// prevent pcap.h to redefine int8_t,... types on Windows
#include "bsdint.h"
#define HAVE_U_INT8_T
#define HAVE_U_INT16_T
#define HAVE_U_INT32_T
#define HAVE_U_INT64_T
#ifdef HAVE_PCAP
#include <pcap.h>
#endif
#include "ExtFrame_m.h"

class cSocketRTScheduler : public cScheduler
{
    protected:
        int fd;

        virtual bool receiveWithTimeout();
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
#ifdef HAVE_PCAP
        static std::vector<cModule *> modules;
        static std::vector<pcap_t *> pds;
        static std::vector<int> datalinks;
        static std::vector<int> headerLengths;
#endif
        static timeval baseTime;

        /**
         * Called at the beginning of a simulation run.
         */
        virtual void startRun();

        /**
         * Called at the end of a simulation run.
         */
        virtual void endRun();

        /**
         * Recalculates "base time" from current wall clock time.
         */
        virtual void executionResumed();

        /**
         * To be called from the module which wishes to receive data from the
         * socket. The method must be called from the module's initialize()
         * function.
         */
        void setInterfaceModule(cModule *mod, const char *dev, const char *filter);

        /**
         * Scheduler function -- it comes from cScheduler interface.
         */
        virtual cMessage *getNextEvent();

        /**
         * Send on the currently open connection
         */
        void sendBytes(unsigned char *buf, size_t numBytes, struct sockaddr *from, socklen_t addrlen);
};

#endif

