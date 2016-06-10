//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.visualization.instruments;

import inet.common.misc.SignalSource;


network InstrumentsExample
{
    parameters:
        @display("bgb=800,400");

        @figure[text](type=indicatorText; pos=20,50; anchor=w; textFormat="value=%g [text]"; initialValue=0);
        @statistic[text](source=signalSourceOutput; record=figure; targetFigure=text);

        @figure[label](type=indicatorLabel; pos=20,80; anchor=w; textFormat="value=%g [label]"; initialValue=0);
        @statistic[label](source=signalSourceOutput; record=figure; targetFigure=label);

        @figure[gauge](type=gauge; pos=50,100; size=150,150);
        @statistic[gauge](source=signalSourceOutput; record=figure; targetFigure=gauge);

        @figure[lineargauge](type=linearGauge; pos=300,50; size=300,50);
        @statistic[lineargauge](source=signalSourceOutput; record=figure; targetFigure=lineargauge);

        @figure[progress](type=progressMeter; pos=300,150; size=300,20; max=100);
        @statistic[progress](source=signalSourceOutput; record=figure; targetFigure=progress);

        @figure[counter](type=counter; pos=300,250; decimalPlaces=5);
        @statistic[counter](source=signalSourceOutput; record=figure; targetFigure=counter);

        @figure[thermometer](type=thermometer; pos=700,50; size=50,300);
        @statistic[thermometer](source=signalSourceOutput; record=figure; targetFigure=thermometer);

    submodules:
        signalSource: SignalSource {
            value = dropUnit(simTime());
            @display("p=50,350;is=s");
        }
}
