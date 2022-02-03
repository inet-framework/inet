import scipy.optimize

from omnetpp.scave.results import *

from inet.simulation.config import *
from inet.simulation.project import *
from inet.simulation.run import *

def run_wifi_error_rate_simulation(bitrate, distance, **kwargs):
    working_directory = "/showcases/wireless/errorrate"
    simulation_config = SimulationConfig(inet_project, working_directory, "omnetpp.ini", "General", 1, False, None)
    simulation_run = SimulationRun(simulation_config, 0)
    extra_args = ["--output-vector-file=${resultdir}/${configname}-" + str(bitrate) + "-" + str(distance) + ".vec", "--**.bitrate=" + str(bitrate) + "Mbps", "--*.destinationHost.mobility.initialX=" + str(distance) + "m"]
    simulation_result = simulation_run.run_simulation(mode="release", extra_args=extra_args, sim_time_limit="1s", **kwargs)
    print(simulation_result)
    filter_expression = """name =~ packetErrorRate:vector"""
    df = read_result_files(inet_project.get_full_path(working_directory + "/results/*-" + str(bitrate) + "-" + str(distance) + ".vec"), filter_expression=filter_expression, include_fields_as_scalars=True)
    packet_error_rate = df[df["type"]=="vector"]["vecvalue"].iloc[0][0]
    return packet_error_rate

def optimize_wifi_distance(bitrate, packet_error_rate, **kwargs):
    def f(args):
        distance = args[0]
        if distance > 0:
            actual_packet_error_rate = run_wifi_error_rate_simulation(bitrate, distance, **kwargs)
            packet_error_rate_difference = abs(actual_packet_error_rate - packet_error_rate)
            print(f"  distance = {distance}, packet_error_rate = {actual_packet_error_rate}, packet_error_rate_difference = {packet_error_rate_difference}")
            return packet_error_rate_difference
        else:
            return 1000000000000
    x0 = np.array([50])
    return scipy.optimize.minimize(f, x0, bounds=[(20, 100)], tol=1E-3)

def optimize_simulation_parameters(simulation_run, parameters, boundaries, cost_function):
    # TODO extract from above
    pass
