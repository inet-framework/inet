//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_IRECOVERYPROCEDURE_H
#define __INET_IRECOVERYPROCEDURE_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecoveryProcedure
{
    public:
        static simsignal_t contentionWindowChangedSignal;
        static simsignal_t retryLimitReachedSignal;

    public:
        class ICwCalculator
        {
            public:
                virtual ~ICwCalculator() { }

                virtual void incrementCw() = 0;
                virtual void resetCw() = 0;
                virtual int getCw() = 0;
        };

    public:
        virtual ~IRecoveryProcedure() { }
};

} // namespace ieee80211
} // namespace inet

#endif // ifndef __INET_IRECOVERYPROCEDURE_H
