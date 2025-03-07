import IPython
import logging
import os

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

try:
    from py4j.clientserver import JavaParameters, PythonParameters, ClientServer
except ImportError as e:
    _logger.warn(str(e) + ": optional inet.common.ide package will not work") # optional feature

client_server = None

def get_client_server():
    global client_server
    if client_server is None:
        client_server = ClientServer(java_parameters=JavaParameters(auth_token=os.environ["AUTH_TOKEN"], port=3071+653, auto_field=True, auto_convert=True, auto_close=True),
                          python_parameters=PythonParameters(port=0, daemonize=True, daemonize_connections=True),
                          python_server_entry_point=None)
    return client_server

def get_org_package():
    return get_client_server().jvm.org

def open_editor(path_name):
    org = get_org_package()
    workbench = org.eclipse.ui.PlatformUI.getWorkbench()
    page = workbench.getWorkbenchWindows()[0].getActivePage()
    workspace_root = org.eclipse.core.resources.ResourcesPlugin.getWorkspace().getRoot()
    path = org.eclipse.core.runtime.Path(path_name)
    file = workspace_root.getFile(path)
    editor_descriptor = org.eclipse.ui.PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor(file.getName())
    wrapped_page = org.omnetpp.python.DisplayThreadInvocationHandler.wrap(page)
    editor_input = org.eclipse.ui.part.FileEditorInput(file)
    return wrapped_page.openEditor(editor_input, editor_descriptor.getId())

def goto_event_number(editor, event_number):
    org = get_org_package()
    workbench = org.eclipse.ui.PlatformUI.getWorkbench()
    sequence_chart = editor.getSequenceChart()
    eventlog = sequence_chart.getEventLog()
    event = eventlog.getEventForEventNumber(event_number)
    wrapped_sequence_chart = org.omnetpp.python.DisplayThreadInvocationHandler.wrap(sequence_chart)
    wrapped_sequence_chart.gotoElement(event)

def launch_program(name, program, args, working_directory, remove_launch=True):
    org = get_org_package()
    launch_configuration = org.omnetpp.dsp.DSPUtils.createLaunchConfiguration(name, program, args, working_directory)
    return org.omnetpp.dsp.DSPUtils.runConfiguration(launch_configuration, remove_launch)

def debug_program(name, program, args, working_directory, remove_launch=True, debugger_init_commands=[]):
    org = get_org_package()
    launch_configuration = org.omnetpp.dsp.DSPUtils.createDebugConfiguration(name, program, args, working_directory, debugger_init_commands)
    return org.omnetpp.dsp.DSPUtils.debugConfiguration(launch_configuration, remove_launch)

def register_key_bindings():
    ip = IPython.get_ipython()
    @ip.pt_app.key_bindings.add('c-e')
    def _(event):
        get_org_package().omnetpp.python.Utils.executeActivateEditorAction()
    @ip.pt_app.key_bindings.add('c-n')
    def _(event):
        get_org_package().omnetpp.python.Utils.executeToggleMaximizeViewAction()
