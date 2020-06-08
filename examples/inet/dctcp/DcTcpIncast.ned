//
// author: Marcel Marek
//

package inet.examples.inet.dctcp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network DcTcpIncast
{
    parameters:
        @display("bgb=740,580");

    types:
        channel NormalP extends DatarateChannel
        {
            parameters:
                per = 0;
                ber = 0;
        }

    submodules:
        routerCore: Router {
            parameters:
                @display("p=315,141;i=abstract/router");
        }
        client11: StandardHost {
            @display("p=50,100");
        }
        client12: StandardHost {
            @display("p=50,176");
        }
        client13: StandardHost {
            @display("p=32,239");
        }
        client14: StandardHost {
            @display("p=38,307");
        }
        client15: StandardHost {
            @display("p=60.000004,359.23077");
        }
        client16: StandardHost {
            @display("p=110.00001,404.6154");
        }
        client17: StandardHost {
            @display("p=160.76924,430.00003");
        }
        client18: StandardHost {
            @display("p=215.38463,455.38464");
        }
        client19: StandardHost {
            @display("p=263.84616,463.07693");
        }
        client110: StandardHost {
            @display("p=313.84616,468.46155");
        }
        client111: StandardHost {
            @display("p=375.38464,469.23077");
        }
        client112: StandardHost {
            @display("p=424.6154,470.00003");
        }
        client113: StandardHost {
            @display("p=461.53848,457.69232");
        }
        client114: StandardHost {
            @display("p=516.9231,429.23077");
        }
        client115: StandardHost {
            @display("p=557.6923,406.15387");
        }
        client116: StandardHost {
            @display("p=598,377");
        }
        client117: StandardHost {
            @display("p=591,307");
        }
        client118: StandardHost {
            @display("p=591,245");
        }
        client119: StandardHost {
            @display("p=591,176");
        }
        client120: StandardHost {
            @display("p=580,119");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=523.8462,35.384617");
        }
        client21: StandardHost {
            @display("p=311.53848,27.692308");
        }
    connections:
        routerCore.ethg++ <--> e1: NormalP <--> client11.ethg++;
        routerCore.ethg++ <--> e2: NormalP <--> client12.ethg++;
        routerCore.ethg++ <--> e3: NormalP <--> client13.ethg++;
        routerCore.ethg++ <--> e4: NormalP <--> client14.ethg++;
        routerCore.ethg++ <--> e5: NormalP <--> client15.ethg++;
        routerCore.ethg++ <--> e6: NormalP <--> client16.ethg++;
        routerCore.ethg++ <--> e7: NormalP <--> client17.ethg++;
        routerCore.ethg++ <--> e8: NormalP <--> client18.ethg++;
        routerCore.ethg++ <--> e9: NormalP <--> client19.ethg++;
        routerCore.ethg++ <--> e10: NormalP <--> client110.ethg++;
        routerCore.ethg++ <--> e11: NormalP <--> client111.ethg++;
        routerCore.ethg++ <--> e12: NormalP <--> client112.ethg++;
        routerCore.ethg++ <--> e13: NormalP <--> client113.ethg++;
        routerCore.ethg++ <--> e14: NormalP <--> client114.ethg++;
        routerCore.ethg++ <--> e15: NormalP <--> client115.ethg++;
        routerCore.ethg++ <--> e16: NormalP <--> client116.ethg++;
        routerCore.ethg++ <--> e17: NormalP <--> client117.ethg++;
        routerCore.ethg++ <--> e18: NormalP <--> client118.ethg++;
        routerCore.ethg++ <--> e19: NormalP <--> client119.ethg++;
        routerCore.ethg++ <--> e20: NormalP <--> client120.ethg++;
        routerCore.ethg++ <--> e21: NormalP <--> client21.ethg++;
}

