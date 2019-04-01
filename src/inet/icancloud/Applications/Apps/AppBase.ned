// This Module groups application parts.
// Each application is defined by an app module, a set of files (if app needs any) and the route where the
// files are allocated.  
//
// @author Gabriel Gonzalez Castañé
// @date 2014-12-15

package inet.icancloud.Applications.Apps;

import inet.icancloud.Applications.Apps.IAPP;
import inet.icancloud.Users.UserGenerator.PreloadFilesDefinition.PreloadFilesDefinition;
import inet.icancloud.Users.UserGenerator.FSUserDefinition.FSUserDefinition;

module AppBase
{
    parameters:
        string appType;					// The name of ned / folder / class of the application
        string name;					// A name for the application
        int copies = default(0);		// Number of replicas that will be launched at system 
        int numFSRoutes;				// Number of different routes that applications will need  
        int numFiles;					// Number of different files that applications will need 

    submodules:

        app: <appType> like IAPP {
            @display("p=65,103");
        }
        
        FSConfig[numFSRoutes]: FSUserDefinition {
            @display("p=332,103");
        }
        
        preloadFiles[numFiles]: PreloadFilesDefinition {
            @display("p=186,103");
        }
}

