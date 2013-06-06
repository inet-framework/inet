package org.omnetpp.inet.installer;

import java.io.File;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.runtime.IProgressMonitor;
import org.omnetpp.common.installer.AbstractProjectInstaller;
import org.omnetpp.common.installer.ProjectDescription;


public class INETProjectInstaller extends AbstractProjectInstaller {
    public INETProjectInstaller(ProjectDescription projectDescription, ProjectInstallationOptions projectInstallationOptions) {
        super(projectDescription, projectInstallationOptions);
    }

    @Override
    public void run(IProgressMonitor progressMonitor) {
        progressMonitor.beginTask("Installing project " + projectDescription.getName() + " into workspace", 5);
        File projectDistributionFile = downloadProjectDistribution(progressMonitor, projectDescription.getDistributionURL());
        if (progressMonitor.isCanceled()) return;
        File projectDirectory = extractProjectDistribution(progressMonitor, projectDistributionFile);
        if (progressMonitor.isCanceled()) return;
        IProject project = importProjectIntoWorkspace(progressMonitor, projectDirectory);
        if (progressMonitor.isCanceled()) return;
        openProject(progressMonitor, project);
        if (progressMonitor.isCanceled()) return;
        buildProject(progressMonitor, project);
        if (progressMonitor.isCanceled()) return;
        String welcomePage = projectDescription.getWelcomePage();
        if (welcomePage != null)
            openEditor(progressMonitor, project.getFile(welcomePage));
        progressMonitor.done();
    }
}
