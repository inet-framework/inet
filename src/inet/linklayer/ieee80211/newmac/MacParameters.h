//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

#ifndef __INET_MACTIMINGPARAMETERS_H
#define __INET_MACTIMINGPARAMETERS_H

#include "IMacParameters.h"

namespace inet {
namespace ieee80211 {

/**
 * The default implementation of IMacParameters
 */
class INET_API MacParameters : public IMacParameters
{
    private:
        MACAddress address;
        const IIeee80211Mode *basicFrameMode = nullptr;
        const IIeee80211Mode *defaultDataFrameMode = nullptr;
        int shortRetryLimit;
        int rtsThreshold;
        bool edca = false;
        simtime_t slotTime;
        simtime_t sifsTime;
        simtime_t aifsTime[AC_NUMCATEGORIES];
        simtime_t eifsTime[AC_NUMCATEGORIES];
        simtime_t pifsTime;
        simtime_t rifsTime;
        int cwMin[AC_NUMCATEGORIES];
        int cwMax[AC_NUMCATEGORIES];
        int cwMulticast[AC_NUMCATEGORIES];
        simtime_t txopLimit[AC_NUMCATEGORIES];

    private:
        void checkAC(AccessCategory ac) const {if (edca) ASSERT(ac>=0 && ac<4); else ASSERT(ac == AC_LEGACY);}

    public:
        MacParameters() {}
        virtual ~MacParameters() {}

        void setAddress(const MACAddress& address) {this->address = address;}
        void setBasicFrameMode(const IIeee80211Mode *basicFrameMode) {this->basicFrameMode = basicFrameMode;}
        void setDefaultDataFrameMode(const IIeee80211Mode *defaultDataFrameMode) { this->defaultDataFrameMode = defaultDataFrameMode; }
        void setRtsThreshold(int rtsThreshold) { this->rtsThreshold = rtsThreshold; }
        void setShortRetryLimit(int shortRetryLimit) { this->shortRetryLimit = shortRetryLimit; }
        void setEdcaEnabled(bool enabled) {edca = enabled;}
        void setSlotTime(simtime_t value) {slotTime = value;}
        void setSifsTime(simtime_t value) {sifsTime = value;}
        void setAifsTime(AccessCategory ac, simtime_t value) {checkAC(ac); aifsTime[ac] = value;}
        void setEifsTime(AccessCategory ac, simtime_t value) {checkAC(ac); eifsTime[ac] = value;} //TODO this is mode-dependent
        void setPifsTime(simtime_t value) {pifsTime = value;}
        void setRifsTime(simtime_t value) {rifsTime = value;}
        void setCwMin(AccessCategory ac, int value) {checkAC(ac); cwMin[ac] = value;}
        void setCwMax(AccessCategory ac, int value) {checkAC(ac); cwMax[ac] = value;}
        void setCwMulticast(AccessCategory ac, int value) {checkAC(ac); cwMulticast[ac] = value;}
        void setTxopLimit(AccessCategory ac, simtime_t value) {checkAC(ac); txopLimit[ac] = value;}

        //TODO throw error when accessing parameters not previously set
        virtual const MACAddress& getAddress() const override {return address;}
        virtual const IIeee80211Mode *getBasicFrameMode() const override {return basicFrameMode;}
        virtual const IIeee80211Mode *getDefaultDataFrameMode() const override {return defaultDataFrameMode;}
        virtual int getShortRetryLimit() const override {return shortRetryLimit;}
        virtual int getRtsThreshold() const override {return rtsThreshold;}
        virtual bool isEdcaEnabled() const override {return edca;}
        virtual simtime_t getSlotTime() const override {return slotTime;}
        virtual simtime_t getSifsTime() const override {return sifsTime;}
        virtual simtime_t getAifsTime(AccessCategory ac) const override {checkAC(ac); return aifsTime[ac];}
        virtual simtime_t getEifsTime(AccessCategory ac) const override {checkAC(ac); return eifsTime[ac];}
        virtual simtime_t getPifsTime() const override {return pifsTime;}
        virtual simtime_t getRifsTime() const override {return rifsTime;}
        virtual int getCwMin(AccessCategory ac) const override {checkAC(ac); return cwMin[ac];}
        virtual int getCwMax(AccessCategory ac) const override {checkAC(ac); return cwMax[ac];};
        virtual int getCwMulticast(AccessCategory ac) const override {checkAC(ac); return cwMulticast[ac];}
        virtual simtime_t getTxopLimit(AccessCategory ac) const override {checkAC(ac); return txopLimit[ac];}
};

} // namespace ieee80211
} // namespace inet

#endif

