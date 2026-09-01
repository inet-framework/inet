//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BARCHARTFIGURE_H
#define __INET_BARCHARTFIGURE_H

#include <string>
#include <vector>

#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

/**
 * A small bar chart indicator figure. Its items are bars: the height and
 * optionally the color of a bar represent the last value of that item, and its
 * label is displayed under it, rotated.
 *
 * In contrast with the other indicator figures, the number of bars is not fixed:
 * setNumItems() sets it, the same way ~PlotFigure::setNumSeries() sets the number
 * of its series. This makes the chart usable for a quantity that exists per peer,
 * per source or per flow, where the set of them only becomes known while the
 * simulation is running. Bars are displayed in index order, so whoever sets them
 * decides the order.
 *
 * A value is mapped to a bar height between zero and barHeight over the
 * minValue..maxValue range. If maxValue is not greater than minValue, then the
 * chart is autoscaled to its own current maximum. The same range is used to
 * interpolate the bar color if more than one bar color is given.
 *
 * The chart is laid out in pixels, from its position towards the bottom right,
 * except for the rotated bar labels, which are kept inside the bounding box
 * returned by getSize() by indenting the chart accordingly.
 */
class INET_API BarChartFigure : public cGroupFigure, public IIndicatorFigure
{
  protected:
    class Bar {
      public:
        std::string label;
        double value = NaN;
        Point labelExtent = Point(0, 0); // the size of the label text, measured when it changes
        cRectangleFigure *barFigure = nullptr;
        cLabelFigure *valueFigure = nullptr;
        cLabelFigure *labelFigure = nullptr;
    };

    Point position = Point(0, 0);
    double minValue = 0;
    double maxValue = 0; // autoscale if not greater than minValue
    double barWidth = 12;
    double barSpacing = 8;
    double barHeight = 60; // height of a bar displaying maxValue
    std::vector<Color> barColors = { Color("blue") };
    Color barLineColor = Color("grey30");
    std::string valueFormat = "%g"; // empty means no value labels
    Font valueFont = Font("", 8);
    Color valueColor = Color("black");
    Font labelFont = Font("", 8);
    Color labelColor = Color("black");
    double labelAngle = M_PI / 4; // radians, counterclockwise
    std::string title;
    Font titleFont = Font("", 9);
    Color titleColor = Color("black");

    std::vector<Bar> bars;
    cLabelFigure *titleFigure = nullptr;
    cLineFigure *baselineFigure = nullptr;
    Point size = Point(0, 0);
    Point titleExtent = Point(0, 0);
    double valueTextHeight = 0;

  protected:
    virtual void parse(cProperty *property) override;
    virtual const char **getAllowedPropertyKeys() const override;

    // Measures the texts the layout depends on; called when a text or a font changes.
    virtual void refreshTextMetrics();
    virtual void layout();
    virtual std::string formatValue(double value) const;
    virtual Color getBarColor(double value, double maxForScale) const;
    virtual Point getTextExtent(const Font& font, const char *text) const;

  public:
    BarChartFigure(const char *name = nullptr);

    virtual const Point getSize() const override { return size; }
    virtual int getNumItems() const override { return bars.size(); }
    virtual void setValue(int index, simtime_t timestamp, double value) override;
    virtual void refreshDisplay() override { layout(); }

    // Sets the number of bars, adding or removing them at the end as needed.
    virtual void setNumItems(int numItems);
    virtual void setItemLabel(int index, const char *label);
    virtual const char *getItemLabel(int index) const;

    const Point& getPosition() const { return position; }
    void setPosition(const Point& position);

    double getMinValue() const { return minValue; }
    void setMinValue(double value);

    double getMaxValue() const { return maxValue; }
    void setMaxValue(double value);

    double getBarWidth() const { return barWidth; }
    void setBarWidth(double width);

    double getBarSpacing() const { return barSpacing; }
    void setBarSpacing(double spacing);

    double getBarHeight() const { return barHeight; }
    void setBarHeight(double height);

    const std::vector<Color>& getBarColors() const { return barColors; }
    void setBarColors(const std::vector<Color>& colors);

    const Color& getBarLineColor() const { return barLineColor; }
    void setBarLineColor(const Color& color);

    const char *getValueFormat() const { return valueFormat.c_str(); }
    void setValueFormat(const char *format);

    const Font& getValueFont() const { return valueFont; }
    void setValueFont(const Font& font);

    const Color& getValueColor() const { return valueColor; }
    void setValueColor(const Color& color);

    const Font& getLabelFont() const { return labelFont; }
    void setLabelFont(const Font& font);

    const Color& getLabelColor() const { return labelColor; }
    void setLabelColor(const Color& color);

    double getLabelAngle() const { return labelAngle; }
    void setLabelAngle(double angle);

    const char *getTitle() const { return title.c_str(); }
    void setTitle(const char *title);

    const Font& getTitleFont() const { return titleFont; }
    void setTitleFont(const Font& font);

    const Color& getTitleColor() const { return titleColor; }
    void setTitleColor(const Color& color);
};

} // namespace inet

#endif

