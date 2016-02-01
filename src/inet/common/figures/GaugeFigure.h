//
// Copyright (C) 2013 OpenSim Ltd
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

#ifndef __INET_GAUGEFIGURE_H
#define __INET_GAUGEFIGURE_H

#include "inet/common/INETDefs.h"

// for the moment commented out as omnet cannot instatiate it from a namespace
//namespace inet {

#if OMNETPP_VERSION >= 0x500

class INET_API GaugeFigure : public cOvalFigure, protected cListener, protected cISimulationLifecycleListener
{
    cPathFigure *needle;
    cTextFigure *valueFigure;
    cTextFigure *labelFigure;
    const char *signalName;
    const char *moduleName;
    const char *label;
    const char *colorStrip;
    double minValue = 0;
    double maxValue = 100;
    double tickSize = 10;
    double value = minValue;

  public:
    GaugeFigure(const char *name = nullptr);
    virtual ~GaugeFigure() {};

    virtual void parse(cProperty *property) override;
    virtual bool isAllowedPropertyKey(const char *key) const override;

    void addChildren();
    void addColorCurve(const cFigure::Color &curveColor, double startAngle, double endAngle);

    void setValue(double newValue);
    void setLabel(const char *newValue);
    virtual const char *getClassNameForRenderer() const;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long l DETAILS_ARG) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, unsigned long l DETAILS_ARG) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d DETAILS_ARG) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    virtual void lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details) override;
};

#else

// dummy figure for OMNeT++ 4.x
class INET_API GaugeFigure : public cGroupFigure {

};

#endif // omnetpp 5

// } // namespace inet

#endif // ifndef __INET_GAUGEFIGURE_H

