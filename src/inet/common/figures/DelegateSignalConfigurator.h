//
// Copyright (C) 2016 Opensim Ltd.
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

#ifndef __INET_DELEGATESIGNALCONFIGURATOR_H
#define __INET_DELEGATESIGNALCONFIGURATOR_H

#include "IIndicatorFigure.h"
#include "inet/common/INETDefs.h"

namespace inet {
/**
 * This is a 3-in-1 class:
 *    1. implements the @delegatesignal properties
 *    2. implements the @displaysignal properties (not used)
 *    3. invokes refreshDisplay() methods on IMeterFigures
 *
 * Probably none of the above will be needed in OMNeT++ 5.1, so this class can go away then.
 */
class INET_API DelegateSignalConfigurator : public cSimpleModule
{
  protected:
    class INET_API DelegatingListener : public cIListener
    {
      protected:
        cComponent *component;
        simsignal_t signal;

      public:
        DelegatingListener(cComponent *component, simsignal_t signal) : component(component), signal(signal) {}
        virtual void receiveSignal(cComponent *, simsignal_t, bool b, cObject *details) { component->emit(signal, b, details); }
        virtual void receiveSignal(cComponent *, simsignal_t, long l, cObject *details) { component->emit(signal, l, details); }
        virtual void receiveSignal(cComponent *, simsignal_t, unsigned long l, cObject *details) { component->emit(signal, l, details); }
        virtual void receiveSignal(cComponent *, simsignal_t, double d, cObject *details) { component->emit(signal, d, details); }
        virtual void receiveSignal(cComponent *, simsignal_t, const SimTime& t, cObject *details) { component->emit(signal, t, details); }
        virtual void receiveSignal(cComponent *, simsignal_t, const char *s, cObject *details) { component->emit(signal, s, details); }
        virtual void receiveSignal(cComponent *, simsignal_t, cObject *obj, cObject *details) { component->emit(signal, obj, details); }
    };

    class INET_API FigureRecorder : public cNumericResultRecorder
    {
      protected:
        IIndicatorFigure *indicatorFigure = nullptr;
        int series = -1;

      protected:
        virtual void collect(simtime_t_cref t, double value, cObject *details) override { indicatorFigure->setValue(series, t, value); }

      public:
        FigureRecorder(IIndicatorFigure *figure, int series) : indicatorFigure(figure), series(series) {}
    };
    std::vector<IIndicatorFigure *> indicatorFigures;

  protected:
    virtual void initialize() override;
    virtual void configure(cModule *module);
    virtual void configureDisplaySignal(cModule *module, cProperty *property);
    virtual void configureDelegateSignal(cModule *module, cProperty *property);
    virtual void parseSignalPath(const char *signalPath, cModule *context, cModule *& module, simsignal_t& signal);
    virtual void refreshDisplay() const override;
};
}    // namespace inet

#endif // ifndef __INET_DELEGATESIGNALCONFIGURATOR_H

