// Module interface of an Application.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Applications.Apps;

moduleinterface IAPP
{
    parameters:
		
    gates:
        input fromOS;			// Input gate from OS (Operating System)
        output toOS;			// Output gate to OS (Operating System)
           
}

