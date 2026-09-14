#include <iostream>

#include "inet/common/INETDefs.h"

#ifdef INET_WITH_VISUALIZATIONCOMMON

#include "inet/common/figures/BarChartFigure.h"

namespace inet {

/**
 * Walks the canvas at the end of the simulation and reports the items of the first
 * bar chart figure it finds: how many there are and what they are labelled. That is
 * what the statistic visualizer actually produced, as opposed to the traffic that
 * fed it, which a result file would show even with no visualizer at all.
 */
class BarChartProbe : public cSimpleModule
{
  protected:
    virtual void finish() override;
    static BarChartFigure *findBarChartFigure(cFigure *figure);
};

Define_Module(BarChartProbe);

BarChartFigure *BarChartProbe::findBarChartFigure(cFigure *figure)
{
    if (auto barChartFigure = dynamic_cast<BarChartFigure *>(figure))
        return barChartFigure;
    for (int i = 0; i < figure->getNumFigures(); i++)
        if (auto found = findBarChartFigure(figure->getFigure(i)))
            return found;
    return nullptr;
}

void BarChartProbe::finish()
{
    // std::cout, because a test runs Cmdenv in express mode, where EV is suppressed
    auto figure = findBarChartFigure(getSimulation()->getSystemModule()->getCanvas()->getRootFigure());
    if (figure == nullptr) {
        std::cout << "barChart: none" << std::endl;
        return;
    }
    std::cout << "barChart items=" << figure->getNumItems() << std::endl;
    for (int i = 0; i < figure->getNumItems(); i++)
        std::cout << "barChart item " << i << " label=" << figure->getItemLabel(i) << std::endl;
}

} // namespace inet

#endif // INET_WITH_VISUALIZATIONCOMMON
