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

#include "inet/common/figures/DelegateSignalConfigurator.h"
#include "inet/common/INETUtils.h"

namespace inet {
Define_Module(DelegateSignalConfigurator);

void DelegateSignalConfigurator::initialize()
{
    configure(getSystemModule());
}

void DelegateSignalConfigurator::configure(cModule *module)
{
    cProperties *properties = module->getProperties();
    for (int i = 0; i < properties->getNumProperties(); i++)
        if (!strcmp(properties->get(i)->getName(), "displaysignal"))
            configureDisplaySignal(module, properties->get(i));
        else if (!strcmp(properties->get(i)->getName(), "delegatesignal"))
            configureDelegateSignal(module, properties->get(i));


    for (cModule::SubmoduleIterator it(module); !it.end(); ++it)
        configure(*it);
}

void DelegateSignalConfigurator::configureDisplaySignal(cModule *module, cProperty *property)
{
    try {
        EV_DETAIL << "Processing @" << property->getFullName() << " on " << module->getFullPath() << endl;

        // find source signal
        cModule *sourceModule;
        simsignal_t signal;
        parseSignalPath(property->getValue("source"), module, sourceModule, signal);

        // instantiate filter
        cResultFilter *filter = nullptr;
        if (const char *filterName = property->getValue("filter")) {
            filter = cResultFilterType::get(filterName)->create();
        }

        // find figure
        const char *figurePath = property->getValue("figure");
        if (!figurePath)
            figurePath = property->getIndex();
        cFigure *figure = module->getCanvas()->getFigureByPath(figurePath);
        if (!figure)
            throw cRuntimeError("Figure '%s' not found", figurePath);
        IIndicatorFigure *meterFigure = check_and_cast<IIndicatorFigure *>(figure);
        indicatorFigures.push_back(meterFigure);

        // instantiate figure recorder
        const char *seriesAttr = property->getValue("series");
        int series = seriesAttr ? utils::atoul(seriesAttr) : 0;
        if (series > meterFigure->getNumSeries())
            throw cRuntimeError("series=%d is out of bounds, figure supports %d series", series, meterFigure->getNumSeries());
        FigureRecorder *figureRecorder = new FigureRecorder(meterFigure, series);
        figureRecorder->init(module, nullptr, nullptr, property);

        // hook it up on the signal
        cResultListener *listener = figureRecorder;
        if (filter) {
            filter->addDelegate(figureRecorder);
            listener = filter;
        }
        sourceModule->subscribe(signal, listener);
    }
    catch (std::exception& e) {
        throw cRuntimeError("While processing @%s on %s: %s", property->getFullName(), module->getFullPath().c_str(), e.what());
    }
}

void DelegateSignalConfigurator::configureDelegateSignal(cModule *module, cProperty *property)
{
    try {
        EV_DETAIL << "Processing @" << property->getFullName() << " on " << module->getFullPath() << endl;

        // parse signals
        cModule *sourceModule, *targetModule;
        simsignal_t sourceSignal, targetSignal;
        parseSignalPath(property->getValue("source"), module, sourceModule, sourceSignal);
        parseSignalPath(property->getValue("target"), module, targetModule, targetSignal);

        // add delegatingListener
        cIListener *listener = new DelegatingListener(targetModule, targetSignal);
        sourceModule->subscribe(sourceSignal, listener);
    }
    catch (std::exception& e) {
        throw cRuntimeError("While processing @%s on %s: %s", property->getFullName(), module->getFullPath().c_str(), e.what());
    }
}

void DelegateSignalConfigurator::parseSignalPath(const char *signalPath, cModule *context, cModule *& module, simsignal_t& signal)
{
    const char *signalName;
    if (strchr(signalPath, '.') == nullptr) {
        module = context;
        signalName = signalPath;
    }
    else {
        const char *lastDot = strrchr(signalPath, '.');
        std::string modulePath = std::string(".") + std::string(signalPath, lastDot - signalPath);
        module = context->getModuleByPath(modulePath.c_str());
        if (!module)
            throw cRuntimeError("Module '%s' not found", modulePath.c_str());
        signalName = lastDot + 1;
    }
    signal = registerSignal(signalName);
}

void DelegateSignalConfigurator::refreshDisplay() const
{
    for (IIndicatorFigure *figure : indicatorFigures)
        figure->refreshDisplay();
}
}    // namespace inet

