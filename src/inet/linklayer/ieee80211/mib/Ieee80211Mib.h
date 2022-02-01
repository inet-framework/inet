//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MIB_H
#define __INET_IEEE80211MIB_H

#include "inet/linklayer/common/MacAddress.h"

namespace inet {

namespace ieee80211 {

class INET_API Ieee80211Mib : public cSimpleModule
{
  public:
    enum Mode {
        INFRASTRUCTURE,
        INDEPENDENT,
        MESH
    };

    enum BssStationType {
        ACCESS_POINT,
        STATION
    };

    enum BssMemberStatus {
        NOT_AUTHENTICATED,
        AUTHENTICATED,
        ASSOCIATED
    };

    class INET_API BssData {
      public:
        std::string ssid;
        MacAddress bssid;
    };

    class INET_API BssStationData {
      public:
        BssStationType stationType = static_cast<BssStationType>(-1);
        bool isAssociated = false;
    };

    class INET_API BssAccessPointData {
      public:
        std::map<MacAddress, BssMemberStatus> stations;
    };

  public:
    MacAddress address;
    Mode mode = static_cast<Mode>(-1);
    bool qos = false;

    BssData bssData;
    BssStationData bssStationData;
    BssAccessPointData bssAccessPointData;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
};

} // namespace ieee80211

} // namespace inet

#endif

