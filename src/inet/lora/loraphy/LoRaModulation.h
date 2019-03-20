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

#ifndef LORAPHY_LORAMODULATION_H_
#define LORAPHY_LORAMODULATION_H_

#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API LoRaModulation : public ApskModulationBase
{
protected:
    static const std::vector<ApskSymbol> constellation;
    int spreadFactor;
    Hz bandwith;
    bps bitRate;
    int notHeader = 1;
    double codeRate;
  public:
    LoRaModulation(int spreadFactor,Hz bandwith,bps bitRate, int notHeader, double codeRate);
    virtual Hz getBandwith() const {return  bandwith;}
    virtual bps getbitRate() const {return  bitRate;}
    virtual int getSpreadFactor() const {return  spreadFactor;}
    virtual simtime_t getSymbolTyme();
    virtual void setHeaderEnable(const bool &p){notHeader = 1; if (p) notHeader = 0;}
    virtual bool getHeaderEnable() {return notHeader == 0;}

    double calculateBER(double snir, Hz bandwidth, bps bitrate) const;
    double calculateSER(double snir, Hz bandwidth, bps bitrate) const;
};

} // namespace physicallayer

} // namespace inet

#endif /* LORAPHY_LORAMODULATION_H_ */
