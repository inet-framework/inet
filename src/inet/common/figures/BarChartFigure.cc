//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/BarChartFigure.h"

#include <algorithm>

namespace inet {

Register_Figure("barChart", BarChartFigure);

static const char *PKEY_POS = "pos";
static const char *PKEY_MIN_VALUE = "minValue";
static const char *PKEY_MAX_VALUE = "maxValue";
static const char *PKEY_BAR_WIDTH = "barWidth";
static const char *PKEY_BAR_SPACING = "barSpacing";
static const char *PKEY_BAR_HEIGHT = "barHeight";
static const char *PKEY_BAR_COLOR = "barColor";
static const char *PKEY_BAR_LINE_COLOR = "barLineColor";
static const char *PKEY_VALUE_FORMAT = "valueFormat";
static const char *PKEY_VALUE_FONT = "valueFont";
static const char *PKEY_VALUE_COLOR = "valueColor";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_LABEL_ANGLE = "labelAngle";
static const char *PKEY_TITLE = "title";
static const char *PKEY_TITLE_FONT = "titleFont";
static const char *PKEY_TITLE_COLOR = "titleColor";

BarChartFigure::BarChartFigure(const char *name) : cGroupFigure(name)
{
    baselineFigure = new cLineFigure("baseline");
    baselineFigure->setLineColor(Color("grey50"));
    baselineFigure->setVisible(false);
    addFigure(baselineFigure);
    titleFigure = new cLabelFigure("title");
    titleFigure->setAnchor(ANCHOR_N);
    titleFigure->setFont(titleFont);
    titleFigure->setColor(titleColor);
    titleFigure->setVisible(false);
    addFigure(titleFigure);
    refreshTextMetrics();
}

void BarChartFigure::refreshTextMetrics()
{
    titleExtent = title.empty() ? Point(0, 0) : getTextExtent(titleFont, title.c_str());
    valueTextHeight = getTextExtent(valueFont, "0").y;
    for (auto& bar : bars)
        bar.labelExtent = getTextExtent(labelFont, bar.label.c_str());
}

const char **BarChartFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_POS, PKEY_MIN_VALUE, PKEY_MAX_VALUE, PKEY_BAR_WIDTH, PKEY_BAR_SPACING,
            PKEY_BAR_HEIGHT, PKEY_BAR_COLOR, PKEY_BAR_LINE_COLOR, PKEY_VALUE_FORMAT,
            PKEY_VALUE_FONT, PKEY_VALUE_COLOR, PKEY_LABEL_FONT, PKEY_LABEL_COLOR,
            PKEY_LABEL_ANGLE, PKEY_TITLE, PKEY_TITLE_FONT, PKEY_TITLE_COLOR, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void BarChartFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);

    const char *s;
    if (property->getNumValues(PKEY_POS) != 0)
        setPosition(parsePoint(property, PKEY_POS, 0));
    if ((s = property->getValue(PKEY_MIN_VALUE)) != nullptr)
        setMinValue(opp_atof(s));
    if ((s = property->getValue(PKEY_MAX_VALUE)) != nullptr)
        setMaxValue(opp_atof(s));
    if ((s = property->getValue(PKEY_BAR_WIDTH)) != nullptr)
        setBarWidth(opp_atof(s));
    if ((s = property->getValue(PKEY_BAR_SPACING)) != nullptr)
        setBarSpacing(opp_atof(s));
    if ((s = property->getValue(PKEY_BAR_HEIGHT)) != nullptr)
        setBarHeight(opp_atof(s));
    if (property->getNumValues(PKEY_BAR_COLOR) != 0) {
        // a single color, or a value to color gradient given as a list of colors
        std::vector<Color> colors;
        for (int i = 0; i < property->getNumValues(PKEY_BAR_COLOR); i++) {
            cStringTokenizer tokenizer(property->getValue(PKEY_BAR_COLOR, i), " ,");
            while (tokenizer.hasMoreTokens())
                colors.push_back(parseColor(tokenizer.nextToken()));
        }
        setBarColors(colors);
    }
    if ((s = property->getValue(PKEY_BAR_LINE_COLOR)) != nullptr)
        setBarLineColor(parseColor(s));
    if ((s = property->getValue(PKEY_VALUE_FORMAT)) != nullptr)
        setValueFormat(s);
    if ((s = property->getValue(PKEY_VALUE_FONT)) != nullptr)
        setValueFont(parseFont(s));
    if ((s = property->getValue(PKEY_VALUE_COLOR)) != nullptr)
        setValueColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL_ANGLE)) != nullptr)
        setLabelAngle(math::deg2rad(opp_atof(s)));
    if ((s = property->getValue(PKEY_TITLE)) != nullptr)
        setTitle(s);
    if ((s = property->getValue(PKEY_TITLE_FONT)) != nullptr)
        setTitleFont(parseFont(s));
    if ((s = property->getValue(PKEY_TITLE_COLOR)) != nullptr)
        setTitleColor(parseColor(s));
}

void BarChartFigure::setNumItems(int numItems)
{
    while ((int)bars.size() > numItems) {
        auto& bar = bars.back();
        delete removeFigure(bar.barFigure);
        delete removeFigure(bar.valueFigure);
        delete removeFigure(bar.labelFigure);
        bars.pop_back();
    }
    while ((int)bars.size() < numItems) {
        Bar bar;
        bar.barFigure = new cRectangleFigure("bar");
        bar.barFigure->setFilled(true);
        bar.barFigure->setLineColor(barLineColor);
        bar.barFigure->setVisible(false);
        addFigure(bar.barFigure);
        bar.valueFigure = new cLabelFigure("value");
        bar.valueFigure->setAnchor(ANCHOR_S);
        bar.valueFigure->setFont(valueFont);
        bar.valueFigure->setColor(valueColor);
        bar.valueFigure->setVisible(false);
        addFigure(bar.valueFigure);
        bar.labelFigure = new cLabelFigure("label");
        bar.labelFigure->setAnchor(ANCHOR_NE);
        bar.labelFigure->setAngle(labelAngle);
        bar.labelFigure->setFont(labelFont);
        bar.labelFigure->setColor(labelColor);
        addFigure(bar.labelFigure);
        bars.push_back(bar);
    }
    layout();
}

void BarChartFigure::setItemLabel(int index, const char *label)
{
    auto& bar = bars.at(index);
    bar.label = label;
    bar.labelFigure->setText(label);
    bar.labelExtent = getTextExtent(labelFont, label);
    layout();
}

const char *BarChartFigure::getItemLabel(int index) const
{
    return bars.at(index).label.c_str();
}

void BarChartFigure::setValue(int index, simtime_t timestamp, double value)
{
    if (index < 0 || index >= (int)bars.size())
        throw cRuntimeError(this, "Item index %d is out of bounds, the figure displays %d items", index, (int)bars.size());
    bars[index].value = value;
    layout();
}

void BarChartFigure::layout()
{
    double titleHeight = title.empty() ? 0 : titleExtent.y + 3;
    double valueHeight = valueFormat.empty() ? 0 : valueTextHeight + 2;
    double chartWidth = bars.empty() ? 0 : bars.size() * barWidth + (bars.size() - 1) * barSpacing;
    double baselineY = position.y + titleHeight + valueHeight + barHeight;

    // the rotated bar labels are anchored at the bar center and hang below and
    // to the left of the baseline; the chart is indented to make room for them
    double labelTextWidth = 0;
    double labelTextHeight = 0;
    for (auto& bar : bars) {
        labelTextWidth = std::max(labelTextWidth, bar.labelExtent.x);
        labelTextHeight = std::max(labelTextHeight, bar.labelExtent.y);
    }
    double cosAngle = std::abs(std::cos(labelAngle));
    double sinAngle = std::abs(std::sin(labelAngle));
    double labelWidth = labelTextWidth * cosAngle + labelTextHeight * sinAngle;
    double labelHeight = labelTextWidth * sinAngle + labelTextHeight * cosAngle;
    double indent = std::max(0.0, labelWidth - barWidth / 2);
    double chartX = position.x + indent;

    size = Point(indent + chartWidth + 2, titleHeight + valueHeight + barHeight + labelHeight + 2);

    titleFigure->setVisible(!title.empty());
    titleFigure->setText(title.c_str());
    titleFigure->setPosition(Point(chartX + chartWidth / 2, position.y));

    baselineFigure->setVisible(!bars.empty());
    baselineFigure->setStart(Point(chartX - 1, baselineY));
    baselineFigure->setEnd(Point(chartX + chartWidth + 1, baselineY));

    // the value range the bar heights and colors are mapped to
    double effectiveMaxValue = maxValue;
    if (!(effectiveMaxValue > minValue)) {
        effectiveMaxValue = minValue;
        for (auto& bar : bars)
            if (!std::isnan(bar.value) && bar.value > effectiveMaxValue)
                effectiveMaxValue = bar.value;
    }
    double range = effectiveMaxValue - minValue;

    for (size_t i = 0; i < bars.size(); i++) {
        auto& bar = bars[i];
        double x = chartX + i * (barWidth + barSpacing);
        double fraction = std::isnan(bar.value) || range <= 0 ? 0 : (bar.value - minValue) / range;
        fraction = std::max(0.0, std::min(1.0, fraction));
        double height = barHeight * fraction;

        bar.barFigure->setVisible(!std::isnan(bar.value) && height > 0);
        bar.barFigure->setBounds(Rectangle(x, baselineY - height, barWidth, height));
        bar.barFigure->setFillColor(getBarColor(bar.value, effectiveMaxValue));
        bar.barFigure->setTooltip((bar.label + ": " + formatValue(bar.value)).c_str());

        bar.valueFigure->setVisible(!valueFormat.empty() && !std::isnan(bar.value));
        bar.valueFigure->setText(formatValue(bar.value).c_str());
        bar.valueFigure->setPosition(Point(x + barWidth / 2, baselineY - height - 1));

        bar.labelFigure->setPosition(Point(x + barWidth / 2, baselineY + 2));
    }
}

std::string BarChartFigure::formatValue(double value) const
{
    if (std::isnan(value))
        return "-";
    char buffer[64];
    snprintf(buffer, sizeof(buffer), valueFormat.c_str(), value);
    return buffer;
}

cFigure::Color BarChartFigure::getBarColor(double value, double maxForScale) const
{
    if (barColors.empty())
        return Color("grey");
    if (barColors.size() == 1 || std::isnan(value))
        return barColors[0];
    double range = maxForScale - minValue;
    double fraction = range > 0 ? (value - minValue) / range : 0;
    fraction = std::max(0.0, std::min(1.0, fraction));
    double position = fraction * (barColors.size() - 1);
    int index = (int)std::floor(position);
    if (index >= (int)barColors.size() - 1)
        return barColors.back();
    double weight = position - index;
    auto& color = barColors[index];
    auto& nextColor = barColors[index + 1];
    auto interpolate = [&](uint8_t c, uint8_t nextC) { return (uint8_t)std::round(c + (nextC - c) * weight); };
    return Color(interpolate(color.red, nextColor.red), interpolate(color.green, nextColor.green), interpolate(color.blue, nextColor.blue));
}

cFigure::Point BarChartFigure::getTextExtent(const Font& font, const char *text) const
{
    double width, height, ascent;
    getSimulation()->getEnvir()->getTextExtent(font, text, width, height, ascent);
    return Point(width, height);
}

void BarChartFigure::setPosition(const Point& position)
{
    this->position = position;
    layout();
}

void BarChartFigure::setMinValue(double value)
{
    minValue = value;
    layout();
}

void BarChartFigure::setMaxValue(double value)
{
    maxValue = value;
    layout();
}

void BarChartFigure::setBarWidth(double width)
{
    barWidth = width;
    layout();
}

void BarChartFigure::setBarSpacing(double spacing)
{
    barSpacing = spacing;
    layout();
}

void BarChartFigure::setBarHeight(double height)
{
    barHeight = height;
    layout();
}

void BarChartFigure::setBarColors(const std::vector<Color>& colors)
{
    barColors = colors;
    layout();
}

void BarChartFigure::setBarLineColor(const Color& color)
{
    barLineColor = color;
    for (auto& bar : bars)
        bar.barFigure->setLineColor(color);
}

void BarChartFigure::setValueFormat(const char *format)
{
    valueFormat = format;
    layout();
}

void BarChartFigure::setValueFont(const Font& font)
{
    valueFont = font;
    for (auto& bar : bars)
        bar.valueFigure->setFont(font);
    refreshTextMetrics();
    layout();
}

void BarChartFigure::setValueColor(const Color& color)
{
    valueColor = color;
    for (auto& bar : bars)
        bar.valueFigure->setColor(color);
}

void BarChartFigure::setLabelFont(const Font& font)
{
    labelFont = font;
    for (auto& bar : bars)
        bar.labelFigure->setFont(font);
    refreshTextMetrics();
    layout();
}

void BarChartFigure::setLabelColor(const Color& color)
{
    labelColor = color;
    for (auto& bar : bars)
        bar.labelFigure->setColor(color);
}

void BarChartFigure::setLabelAngle(double angle)
{
    labelAngle = angle;
    for (auto& bar : bars)
        bar.labelFigure->setAngle(angle);
    layout();
}

void BarChartFigure::setTitle(const char *title)
{
    this->title = title;
    refreshTextMetrics();
    layout();
}

void BarChartFigure::setTitleFont(const Font& font)
{
    titleFont = font;
    titleFigure->setFont(font);
    refreshTextMetrics();
    layout();
}

void BarChartFigure::setTitleColor(const Color& color)
{
    titleColor = color;
    titleFigure->setColor(color);
}

} // namespace inet

