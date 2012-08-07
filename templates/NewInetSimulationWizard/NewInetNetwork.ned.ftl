<@setoutput path=targetMainFile?default("network.ned")/>
${bannerComment}

<#if nedPackageName!="">
package ${nedPackageName};
</#if>

<#if ipv4Layer>
    <#assign inetX = "inet"> 
    <#assign hostModule = "StandardHost"> 
    <#assign routerModule = "Router">
    <#assign configuratorModule = "IPv4NetworkConfigurator">
import inet.networklayer.autorouting.ipv4.${configuratorModule};
</#if>

<#if ipv6Layer> 
    <#assign inetX = "ipv6"> 
    <#assign hostModule = "StandardHost6"> 
    <#assign routerModule = "Router6"> 
    <#assign configuratorModule = "FlatNetworkConfigurator6">
import inet.networklayer.autorouting.ipv6.${configuratorModule};
</#if>

<#if (numClients?number <= 3) >
    <#assign srvXpos = 100> 
<#else>
    <#assign srvXpos = 10+numClients?number*30> 
</#if>

import inet.nodes.${inetX}.${routerModule};
import inet.nodes.${inetX}.${hostModule};
import ned.DatarateChannel;

network ${targetTypeName}
{
    types:
        channel Channel extends DatarateChannel
        {
            datarate = 100Mbps;
            delay = 0.1us;
        }

    submodules:
        configurator: ${configuratorModule}
        {
            parameters:
                @display("p=40,40");
        }

        server: ${hostModule}
        {
            parameters:
                @display("p=${srvXpos},40;i=device/pc2");
        }

        router: ${routerModule}
        {
            parameters:
                @display("p=${srvXpos},100");
        }

<#list 1..numClients?number as i>
        client${i}: ${hostModule}
        {
            parameters:
                @display("p=${i*60-20},160;i=device/pc3");
        }
</#list>

    connections:
        server.ethg++  <--> Channel <--> router.ethg++;
        <#list 1..numClients?number as i>
        client${i}.ethg++ <--> Channel <--> router.ethg++;
        </#list>
}
