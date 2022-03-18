import logging
import os

logger = logging.getLogger(__name__)

try:
    from py4j.clientserver import JavaParameters, PythonParameters, ClientServer
except ImportError as e:
    logger.warn(str(e) + ": optional inet.common.ide package will not work") # optional feature

client_server = None

def get_client_server():
    global client_server
    if client_server is None:
        client_server = ClientServer(java_parameters=JavaParameters(auth_token=os.environ["AUTH_TOKEN"], port=3071+653, auto_field=True, auto_convert=True, auto_close=True),
                          python_parameters=PythonParameters(port=0, daemonize=True, daemonize_connections=True),
                          python_server_entry_point=None)
    return client_server

def open_editor(path_name):
    client_server = get_client_server()
    org = client_server.jvm.org
    workbench = org.eclipse.ui.PlatformUI.getWorkbench()
    page = workbench.getWorkbenchWindows()[0].getActivePage()
    workspace_root = org.eclipse.core.resources.ResourcesPlugin.getWorkspace().getRoot()
    path = org.eclipse.core.runtime.Path(path_name)
    file = workspace_root.getFile(path)
    editor_descriptor = org.eclipse.ui.PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor(file.getName())
    wrapped_page = org.omnetpp.remoting.py4j.DisplayThreadInvocationHandler.wrap(page)
    return wrapped_page.openEditor(org.eclipse.ui.part.FileEditorInput(file), editor_descriptor.getId())

def goto_event_number(editor, event_number):
    client_server = get_client_server()
    org = client_server.jvm.org
    workbench = org.eclipse.ui.PlatformUI.getWorkbench()
    sequence_chart = editor.getSequenceChart()
    eventlog = sequence_chart.getEventLog()
    event = eventlog.getEventForEventNumber(event_number)
    wrapped_sequence_chart = org.omnetpp.remoting.py4j.DisplayThreadInvocationHandler.wrap(sequence_chart)
    wrapped_sequence_chart.gotoElement(event)
