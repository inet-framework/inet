//
// Module that implements a StatesApplication.
//
// This application is an application system responsible for managing the states of the hardware devices
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute;
// @date 2013-03-04

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.SystemApps.StatesApplication;

simple StatesApplication
{
    @class(icancloud::StatesApplication);
    gates:
        input fromOS;			// Input gate from OS (Operating System)
        output toOS;			// Output gate to OS (Operating System)

}


