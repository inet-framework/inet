import math
import optimparallel
import scipy.optimize
import time

from omnetpp.scave.results import *

from inet.simulation.config import *
from inet.simulation.project import *
from inet.simulation.task import *
from inet.simulation.cffi import *

def cost_function(parameter_values, simulation_task, expected_result_names, expected_result_values, fixed_parameter_names, fixed_parameter_values, fixed_parameter_assignments, fixed_parameter_units, parameter_names, parameter_assignments, parameter_units, kwargs):
    all_parameter_assignments = [*fixed_parameter_assignments, *parameter_assignments]
    all_parameter_values = [*fixed_parameter_values, *parameter_values]
    all_parameter_units = [*fixed_parameter_units, *parameter_units]
    all_parameter_assignment_args = list(map(lambda name, value, unit: "--" + name + "=" + str(value) + unit, all_parameter_assignments, all_parameter_values, all_parameter_units))
    output_vector_file = "results/" + simulation_task.simulation_config.config + "-" + "-".join(map(str, all_parameter_values)) + ".vec"
    extra_args = ["--output-vector-file=" + output_vector_file, *all_parameter_assignment_args]
    simulation_result = simulation_task.run(extra_args=extra_args, **kwargs)
    if simulation_result.result == "DONE":
        filter_expression = """name =~ packetErrorRate:vector"""
        result_file = simulation_task.simulation_config.simulation_project.get_full_path(os.path.join(simulation_task.simulation_config.working_directory, output_vector_file))
        df = read_result_files(result_file, filter_expression=filter_expression, include_fields_as_scalars=True)
        result_values = [df[df["type"]=="vector"]["vecvalue"].iloc[0][0]]
    else:
        result_values = [math.nan]
    result_value_absolute_differences = list(map(lambda x, y: abs(x - y), expected_result_values, result_values))
    print(f"  {parameter_names} = {parameter_values}, {expected_result_names} = {result_values}, result_value_absolute_differences = {result_value_absolute_differences}")
    return sum(result_value_absolute_differences)

def optimize_simulation_parameters(simulation_task, expected_result_names, expected_result_values,
                                   fixed_parameter_names, fixed_parameter_values, fixed_parameter_assignments, fixed_parameter_units,
                                   parameter_names, parameter_assignments, parameter_units,
                                   initial_values, min_values, max_values, tol=1E-3,
                                   concurrent=False, simulation_runner=inprocess_simulation_runner if "inprocess_simulation_runner" in locals() else subprocess_simulation_runner, **kwargs):
    # TODO unfortunately we cannot run simulations concurrently in the same process right now
    if concurrent:
        simulation_runner = subprocess_simulation_runner
    start_time = time.time()
    xs = np.array(initial_values)
    bounds = list(map(lambda min, max: (min, max), min_values, max_values))
    args = (simulation_task, expected_result_names, expected_result_values, fixed_parameter_names, fixed_parameter_values, fixed_parameter_assignments, fixed_parameter_units, parameter_names, parameter_assignments, parameter_units, dict(kwargs, simulation_runner=simulation_runner))
    if concurrent:
        result = optimparallel.minimize_parallel(cost_function, xs, args=args, bounds=bounds, tol=tol)
    else:
        result = scipy.optimize.minimize(cost_function, xs, args=args, bounds=bounds, tol=tol)
    end_time = time.time()
    elapsed_wall_time = end_time - start_time
    print(f"Elapsed time: {elapsed_wall_time}")
    print(result)
    return result.x[0]

def optimize_wifi_distance(bitrate=54, expected_packet_error_rate=0.3, mode="release", **kwargs):
    simulation_task = get_simulation_task(mode=mode, working_directory_filter="showcases/wireless/errorrate", config_filter="General", run=0, sim_time_limit="1s")
    return optimize_simulation_parameters(
        simulation_task, expected_result_names=["packetErrorRate:vector"], expected_result_values=[expected_packet_error_rate],
        fixed_parameter_names=["bitrate"], fixed_parameter_values=[bitrate], fixed_parameter_assignments=["**.bitrate"], fixed_parameter_units=["Mbps"],
        parameter_names=["distance"], parameter_assignments=["*.destinationHost.mobility.initialX"], parameter_units=["m"],
        initial_values=[50], min_values=[20], max_values=[100], **kwargs)
