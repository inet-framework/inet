import logging
from py4j.clientserver import JavaParameters, PythonParameters, ClientServer

logger = logging.getLogger(__name__)

cs = ClientServer(java_parameters=JavaParameters(port=3071+653, auto_field=True, auto_convert=True, auto_close=True),
                  python_parameters=PythonParameters(port=0, daemonize=True, daemonize_connections=True),
                  python_server_entry_point=None)
org = cs.jvm.org

workbench = org.eclipse.ui.PlatformUI.getWorkbench()

def open_editor(path_name):
    page = workbench.getWorkbenchWindows()[0].getActivePage()
    workspace_root = org.eclipse.core.resources.ResourcesPlugin.getWorkspace().getRoot()
    path = org.eclipse.core.runtime.Path(path_name)
    file = workspace_root.getFile(path)
    editor_descriptor = org.eclipse.ui.PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor(file.getName())
    wrapped_page = org.omnetpp.remoting.py4j.DisplayThreadInvocationHandler.wrap(page)
    return wrapped_page.openEditor(org.eclipse.ui.part.FileEditorInput(file), editor_descriptor.getId())

def goto_event_number(editor, event_number):
    sequence_chart = editor.getSequenceChart()
    eventlog = sequence_chart.getEventLog()
    event = eventlog.getEventForEventNumber(event_number)
    wrapped_sequence_chart = org.omnetpp.remoting.py4j.DisplayThreadInvocationHandler.wrap(sequence_chart)
    wrapped_sequence_chart.gotoElement(event)
