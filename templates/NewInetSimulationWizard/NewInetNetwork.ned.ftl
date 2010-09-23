<@setoutput path=targetMainFile?default("network.ned")/>
${bannerComment}

<#if nedPackageName!="">
package ${nedPackageName};
</#if>

<#if ipv4Layer>
    <#assign inetX = "inet"> 
    <#assign hostModule = "StandardHost"> 
    <#assign routerModule = "Router">
    <#assign configuratorModule = "FlatNetworkConfigurator">
</#if>

<#if ipv6Layer> 
    <#assign inetX = "ipv6"> 
    <#assign hostModule = "StandardHost6"> 
    <#assign routerModule = "Router6"> 
    <#assign configuratorModule = "FlatNetworkConfigurator6">
</#if>

<#if (numClients?number <= 3) >
    <#assign srvXpos = 100> 
<#else>
    <#assign srvXpos = 10+numClients?number*30> 
</#if>

import inet.networklayer.autorouting.${configuratorModule};
import inet.nodes.${inetX}.${routerModule};
import inet.nodes.${inetX}.${hostModule};
import ned.DatarateChannel;

network ${targetTypeName}
{
    types:
        channel Channel extends DatarateChannel
        {
<#if ipv4Layer>
            datarate = 100Mbps;
</#if>
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

<#if ipv4Layer>
    connections:
        server.pppg++  <--> Channel <--> router.pppg++;
        <#list 1..numClients?number as i>
        client${i}.pppg++ <--> Channel <--> router.pppg++;
        </#list>
</#if>

<#if ipv6Layer>
    connections:
        server.ethg++  <--> Channel <--> router.ethg++;
        <#list 1..numClients?number as i>
        client${i}.ethg++ <--> Channel <--> router.ethg++;
        </#list>
</#if>
}
