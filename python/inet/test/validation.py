"""
This module provides functionality for validation testing of multiple simulations.

The main function is :py:func:`run_validation_tests`. It allows running multiple validation tests matching
the provided filter criteria. Validation tests check simulations results against analytical model results,
often cited in research papers, or simulation results of models created for other simulation frameworks.
"""

import itertools
import logging
import math
import numpy
import pandas as pd
import random
from statistics import fmean

from omnetpp.scave.results import *

from inet.project.inet import *
from inet.simulation.project import *
from inet.test.simulation import *

_logger = logging.getLogger(__name__)

class ValidationTestTask(TaskTestTask):
    def __init__(self, check_function=None, name="validation test", **kwargs):
        super().__init__(name=name, **kwargs)
        self.check_function = check_function

    def check_task_result(self, **kwargs):
        return self.check_function(**kwargs)

def compare_test_results(result1, result2, accuracy=0.01):
    return abs(result1 - result2) / result1 < accuracy

############################
# TSN frame replication test

# Observed result
# 0.6578411405295316
def compute_frame_replication_success_rate_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND ((module =~ "*.destination.udp" AND name =~ packetReceived:count) OR (module =~ "*.source.udp" AND name =~ packetSent:count))"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/framereplication/results/*.sca"), filter_expression=filter_expression)
    df = get_scalars(df)
    packetSent = float(df[df.name == "packetSent:count"].value.iloc[0])
    packetReceived = float(df[df.name == "packetReceived:count"].value.iloc[0])
    return packetReceived / packetSent

def compute_frame_replication_success_rate_analytically1():
    combinations = numpy.array(list(itertools.product([0, 1], repeat=7)))
    probabilities = numpy.array([0.8, 0.8, 0.64, 0.8, 0.64, 0.8, 0.8])
    solutions = numpy.array([[1, 1, 1, 0, 0, 0, 0], [1, 1, 0, 0, 1, 1, 0], [1, 0, 0, 1, 1, 0, 0,], [1, 0, 1, 1, 0, 0, 1]])
    p = 0
    for combination in combinations:
        probability = (combination * probabilities + (1 - combination) * (1 - probabilities)).prod()
        for solution in solutions:
            if (solution * combination == solution).all():
                p += probability
                break
    return p

def compute_frame_replication_success_rate_analytically2():
    successful = 0
    n = 1000000
    for i in range(n):
        s1 = random.random() < 0.8
        s1_s2a = random.random() < 0.8
        s1_s2b = random.random() < 0.8
        s2a_s2b = random.random() < 0.8
        s2b_s2a = random.random() < 0.8
        s2a = (s1 and s1_s2a) or (s1 and s1_s2b and s2b_s2a)
        s2b = (s1 and s1_s2b) or (s1 and s1_s2a and s2a_s2b)
        s3a = s2a and (random.random() < 0.8)
        s3b = s2b and (random.random() < 0.8)
        s3a_s4 = random.random() < 0.8
        s3b_s4 = random.random() < 0.8
        s4 = (s3a and s3a_s4) or (s3b and s3b_s4)
        if s4:
            successful += 1
    return successful / n

def compute_tsn_framereplication_validation_test_results(test_accuracy=0.01, **kwargs):
    ps = compute_frame_replication_success_rate_from_simulation_results(**kwargs)
    pa1 = compute_frame_replication_success_rate_analytically1()
    pa2 = compute_frame_replication_success_rate_analytically2()
    test_result1 = compare_test_results(ps, pa1, test_accuracy)
    test_result2 = compare_test_results(ps, pa2, test_accuracy)
    return TestTaskResult(task=TestTask(), bool_result=test_result1 and test_result2)

def get_tsn_framereplication_simulation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(working_directory_filter="tests/validation/tsn/framereplication", sim_time_limit="0.2s", **kwargs).tasks[0]
    return ValidationTestTask(tested_task=simulation_task, check_function=compute_tsn_framereplication_validation_test_results, **kwargs)

def run_tsn_framereplication_validation_test(test_accuracy=0.01, **kwargs):
    return get_tsn_framereplication_simulation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

###################################################
# TSN traffic shaping asynchronous shaper ICCT test

# Observed result
# name                      max
# module
# Flow 1, CDT            505.16
# Flow 2, Class A       1579.72
# Flow 3, Class B       3371.72
# Flow 4, Class A        932.04
# Flow 5, Class B       2162.12
# Flow 6, Class A        585.16
# Flow 7, Class B       1963.24
# Flow 8, Best Effort  34913.16
def compute_asynchronousshaper_icct_endtoend_delay_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND name =~ meanBitLifeTimePerPacket:histogram:max"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/asynchronousshaper/icct/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    df["name"] = df["name"].map(lambda name: re.sub(r".*(min|max)", "\\1", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*N6.app\[[0-4]\].*", "Flow 4, Class A", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*N6.app\[[5-9]\].*", "Flow 5, Class B", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*N7.app\[[0-9]\].*", "Flow 1, CDT", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*N7.app\[1[0-9]\].*", "Flow 2, Class A", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*N7.app\[2[0-9]\].*", "Flow 3, Class B", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*N7.app\[3[0-4]\].*", "Flow 6, Class A", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*N7.app\[3[5-9]\].*", "Flow 7, Class B", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*N7.app\[40\].*", "Flow 8, Best Effort", name))
    df = pd.pivot_table(df, index="module", columns="name", values="value", aggfunc="max")
    return df * 1000000

def compute_asynchronousshaper_icct_endtoend_delay_alternatively():
    # This validation test compares simulation results to analytical results presented
    # in the paper titled "The Delay Bound Analysis Based on Network Calculus for
    # Asynchronous Traffic Shaping under Parameter Inconsistency" from the 2020 IEEE
    # 20th International Conference on Communication Technology
    return pd.DataFrame(index = ["Flow 1, CDT", "Flow 2, Class A", "Flow 3, Class B", "Flow 4, Class A", "Flow 5, Class B", "Flow 6, Class A", "Flow 7, Class B", "Flow 8, Best Effort"],
                        data = {"max": [867.6, 4665.5, 10902, 2505.2, 5678.3, float('inf'), float('inf'), float('inf')]})

def compute_tsn_trafficshaping_asynchronousshaper_icct_validation_test_results(**kwargs):
    df1 = compute_asynchronousshaper_icct_endtoend_delay_from_simulation_results(**kwargs)
    df2 = compute_asynchronousshaper_icct_endtoend_delay_alternatively()
    test_result = (df1["max"] < df2["max"]).all()
    return TestTaskResult(task=TestTask(), bool_result=test_result)

def get_tsn_trafficshaping_asynchronousshaper_icct_simulation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(working_directory_filter="tests/validation/tsn/trafficshaping/asynchronousshaper/icct", sim_time_limit="0.1s", **kwargs).tasks[0]
    return ValidationTestTask(tested_task=simulation_task, check_function=compute_tsn_trafficshaping_asynchronousshaper_icct_validation_test_results, **kwargs)

def run_tsn_trafficshaping_asynchronousshaper_icct_validation_test(**kwargs):
    return get_tsn_trafficshaping_asynchronousshaper_icct_simulation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

########################################################
# TSN traffic shaping asynchronous shaper Core4INET test

# Observed result
# name          max        mean      min      stddev
# module
# Critical  375.780  298.477239  252.900   36.668923
# High      307.260  161.309427   60.900   73.625945
# Medium    535.939  247.258720   88.259  106.588325
def compute_asynchronousshaper_core4inet_endtoend_delay_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND (name =~ meanBitLifeTimePerPacket:histogram:min OR name =~ meanBitLifeTimePerPacket:histogram:max OR name =~ meanBitLifeTimePerPacket:histogram:mean OR name =~ meanBitLifeTimePerPacket:histogram:stddev)"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/asynchronousshaper/core4inet/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    df["name"] = df["name"].map(lambda name: re.sub(r".*(min|max|mean|stddev)", "\\1", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*app\[0\].*", "Best effort", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*app\[1\].*", "Medium", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*app\[2\].*", "High", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*app\[3\].*", "Critical", name))
    df = df.loc[df["module"]!="Best effort"]
    df = pd.pivot_table(df, index="module", columns="name", values="value")
    return df * 1000000

# Observed result
# 2.0
def compute_asynchronousshaper_core4inet_max_queuelength_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND module =~ \"*.switch.eth[4].macLayer.queue.queue[5..7]\" AND name =~ queueLength:max"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/asynchronousshaper/core4inet/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    return numpy.max(df["value"])

def compute_asynchronousshaper_core4inet_endtoend_delay_alternatively(**kwargs):
    # This validation test compares simulation results to analytical results and also
    # to results from a different simulation using Core4INET.
    # https://github.com/CoRE-RG/CoRE4INET/tree/master/examples/tsn/medium_network
    df = pd.DataFrame(index=["Medium", "High", "Critical"],
                      data={"min": [88.16, 60.8, 252.8],
                            "max": [540, 307.2, 375.84],
                            "mean": [247.1, 161.19, 298.52],
                            "stddev": [106.53, 73.621, 36.633]})
    df["min"] -= 0.001 # 1 ns initial production offset, see INI file
    df = df + 0.05 * 2 # 50 ns propagation delay per hop
    df.index.set_names(["trafficclass"], inplace=True)
    df.columns.set_names(["name"], inplace=True)
    return df

def compute_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_results(test_accuracy=0.01, **kwargs):
    df1 = compute_asynchronousshaper_core4inet_endtoend_delay_from_simulation_results(**kwargs)
    df2 = compute_asynchronousshaper_core4inet_endtoend_delay_alternatively(**kwargs)
    df1 = df1.sort_index(axis=0).sort_index(axis=1)
    df2 = df2.sort_index(axis=0).sort_index(axis=1)
    maxQueueLength = compute_asynchronousshaper_core4inet_max_queuelength_from_simulation_results()
    test_result = maxQueueLength < 4 and \
                  (df1["min"] >= df2["min"]).all() and \
                  (df1["max"] <= df2["max"]).all() and \
                  numpy.allclose(df1["min"], df2["min"], rtol=test_accuracy, atol=0) and \
                  numpy.allclose(df1["max"], df2["max"], rtol=test_accuracy, atol=0) and \
                  numpy.allclose(df1["mean"], df2["mean"], rtol=test_accuracy * 7, atol=0) and \
                  numpy.allclose(df1["stddev"], df2["stddev"], rtol=test_accuracy * 30, atol=0)
    return TestTaskResult(task=TestTask(), bool_result=test_result)

def get_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(working_directory_filter="tests/validation/tsn/trafficshaping/asynchronousshaper/core4inet", sim_time_limit="1s", **kwargs).tasks[0]
    return ValidationTestTask(tested_task=simulation_task, check_function=compute_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_results, **kwargs)

def run_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test(test_accuracy=0.01, **kwargs):
    return get_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

##############################################
# TSN traffic shaping credit-based shaper test

# Observed result
# name          max        mean      min      stddev
# module
# Critical  375.780  298.477239  252.900   36.668923
# High      307.260  161.309427   60.900   73.625945
# Medium    535.939  247.258720   88.259  106.588325
def compute_creditbasedshaper_endtoend_delay_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND (name =~ meanBitLifeTimePerPacket:histogram:min OR name =~ meanBitLifeTimePerPacket:histogram:max OR name =~ meanBitLifeTimePerPacket:histogram:mean OR name =~ meanBitLifeTimePerPacket:histogram:stddev)"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/creditbasedshaper/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    df["name"] = df["name"].map(lambda name: re.sub(r".*(min|max|mean|stddev)", "\\1", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*app\[0\].*", "Best effort", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*app\[1\].*", "Medium", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*app\[2\].*", "High", name))
    df["module"] = df["module"].map(lambda name: re.sub(r".*app\[3\].*", "Critical", name))
    df = df.loc[df["module"]!="Best effort"]
    df = pd.pivot_table(df, index="module", columns="name", values="value")
    return df * 1000000

# Observed result
# 2.0
def compute_creditbasedshaper_max_queuelength_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND module =~ \"*.switch.eth[4].macLayer.queue.queue[5..7]\" AND name =~ queueLength:max"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/creditbasedshaper/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    return numpy.max(df["value"])

def compute_creditbasedshaper_endtoend_delay_alternatively(**kwargs):
    # This validation test compares simulation results to analytical results and also
    # to results from a different simulation using Core4INET.
    # https://github.com/CoRE-RG/CoRE4INET/tree/master/examples/tsn/medium_network
    df = pd.DataFrame(index=["Medium", "High", "Critical"],
                      data={"min": [88.16, 60.8, 252.8],
                            "max": [540, 307.2, 375.84],
                            "mean": [247.1, 161.19, 298.52],
                            "stddev": [106.53, 73.621, 36.633]})
    df["min"] -= 0.001 # 1 ns initial production offset, see INI file
    df = df + 0.05 * 2 # 50 ns propagation delay per hop
    df.index.set_names(["trafficclass"], inplace=True)
    df.columns.set_names(["name"], inplace=True)
    return df

def compute_tsn_trafficshaping_creditbasedshaper_validation_test_results(test_accuracy=0.01, **kwargs):
    df1 = compute_creditbasedshaper_endtoend_delay_from_simulation_results(**kwargs)
    df2 = compute_creditbasedshaper_endtoend_delay_alternatively(**kwargs)
    df1 = df1.sort_index(axis=0).sort_index(axis=1)
    df2 = df2.sort_index(axis=0).sort_index(axis=1)
    maxQueueLength = compute_creditbasedshaper_max_queuelength_from_simulation_results()
    test_result = maxQueueLength < 4 and \
                  (df1["min"] >= df2["min"]).all() and \
                  (df1["max"] <= df2["max"]).all() and \
                  numpy.allclose(df1["min"], df2["min"], rtol=test_accuracy, atol=0) and \
                  numpy.allclose(df1["max"], df2["max"], rtol=test_accuracy, atol=0) and \
                  numpy.allclose(df1["mean"], df2["mean"], rtol=test_accuracy * 7, atol=0) and \
                  numpy.allclose(df1["stddev"], df2["stddev"], rtol=test_accuracy * 30, atol=0)
    return TestTaskResult(task=TestTask(), bool_result=test_result)

def get_tsn_trafficshaping_creditbasedshaper_validation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(working_directory_filter="tests/validation/tsn/trafficshaping/creditbasedshaper", sim_time_limit="1s", **kwargs).tasks[0]
    return ValidationTestTask(tested_task=simulation_task, check_function=compute_tsn_trafficshaping_creditbasedshaper_validation_test_results, **kwargs)

def run_tsn_trafficshaping_creditbasedshaper_validation_test(**kwargs):
    return get_tsn_trafficshaping_creditbasedshaper_validation_test_task(**kwargs).run(**kwargs)

##############################
# QUIC link utilization test

def _quic_get_throughput_from_vec_file(file, module_name, start, end):
    """Helper function to compute throughput from a vector file using omnetpp.scave.results."""
    filter_expression = f"type =~ vector AND module =~ \"{module_name}\" AND name =~ \"packetReceived:vector(packetBytes)\""
    df = read_result_files(file, filter_expression=filter_expression)
    df = get_vectors(df)

    if df.empty:
        return 0

    # Get the vector data
    vector_data = df.iloc[0]
    times = vector_data['vectime']
    values = vector_data['vecvalue']

    # Filter by time range
    mask = (times >= start) & (times <= end)
    filtered_times = times[mask]
    filtered_values = values[mask]

    if len(filtered_times) == 0:
        return 0

    first = filtered_times[0]
    last = filtered_times[-1]
    data = sum(filtered_values[1:])  # Skip first value as in original code

    tp = data * 8 / 1000000 / (last - first) if last > first else 0

    return tp

def compute_quic_link_utilization_from_simulation_results(**kwargs):
    results_path = inet_project.get_full_path("examples/quic/link_utilization/results/")
    bandwidth = 10  # Mb/s
    mtus = ['1280', '1452', '1500', '9000']  # B
    start = 4
    end = 5
    results = []
    for mtu in mtus:
        tp = _quic_get_throughput_from_vec_file(
            results_path + "/link_utilization_mtu=" + mtu + ".vec",
            'bottleneck.receiver.quic', start, end)
        results.append({'mtu': int(mtu), 'throughput': tp})
    return results

def compute_quic_link_utilization_analytically():
    bandwidth = 10  # Mb/s
    mtus = [1280, 1452, 1500, 9000]
    results = []
    for mtu in mtus:
        # QUIC packet is 28B smaller than MTU (IP packet minus IP header and minus UDP header)
        # Packet on the wire is 7B larger than MTU (link layer overhead)
        tp_should = (mtu - 28) / (mtu + 7) * bandwidth
        results.append({'mtu': mtu, 'throughput': tp_should})
    return results

def compute_quic_link_utilization_validation_test_results(test_accuracy=0.02, **kwargs):
    sim_results = compute_quic_link_utilization_from_simulation_results(**kwargs)
    analytical_results = compute_quic_link_utilization_analytically()
    all_pass = True
    for sim, ana in zip(sim_results, analytical_results):
        if not compare_test_results(ana['throughput'], sim['throughput'], test_accuracy):
            all_pass = False
            break
    return TestTaskResult(task=TestTask(), bool_result=all_pass)

def get_quic_link_utilization_validation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(working_directory_filter="examples/quic/link_utilization", **kwargs)
    simulation_task.build = False
    simulation_task.simulation_config = simulation_task.tasks[0].simulation_config
    return ValidationTestTask(name="QUIC link utilization validation test", tested_task=simulation_task, check_function=compute_quic_link_utilization_validation_test_results, **kwargs)

def run_quic_link_utilization_validation_test(test_accuracy=0.02, **kwargs):
    return get_quic_link_utilization_validation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

####################################
# QUIC throughput packet loss test

def compute_quic_throughput_packet_loss_from_simulation_results(**kwargs):
    results_path = inet_project.get_full_path("examples/quic/throughput_packet_loss/results/throughput_validation_")
    ps = [".008", ".01", ".02"]
    rtts_measured = [20, 40, 60]
    start = 4
    end = 14
    results = []
    for p_str in ps:
        for rtt in rtts_measured:
            file = results_path + "p=" + p_str + "_delay=" + str(int(rtt / 2)) + ".vec"
            tp = _quic_get_throughput_from_vec_file(file, 'bottleneck.receiver.quic', start, end)
            results.append({'p': float(p_str), 'rtt': rtt, 'throughput': tp})
    return results

def compute_quic_throughput_packet_loss_analytically():
    # Mathis formula: throughput = (MSS * 8) / (RTT * sqrt(2*p/3))
    S = 1252  # segment size
    ps = [0.008, 0.01, 0.02]
    rtts = [20, 40, 60]
    results = []
    for p in ps:
        for rtt in rtts:
            tp_should = (S * 8) / (1000 * rtt) * 1 / math.sqrt(2 * p / 3)
            results.append({'p': p, 'rtt': rtt, 'throughput': tp_should})
    return results

def compute_quic_throughput_packet_loss_validation_test_results(test_accuracy=0.15, **kwargs):
    sim_results = compute_quic_throughput_packet_loss_from_simulation_results(**kwargs)
    analytical_results = compute_quic_throughput_packet_loss_analytically()
    all_pass = True
    for sim, ana in zip(sim_results, analytical_results):
        if not compare_test_results(ana['throughput'], sim['throughput'], test_accuracy):
            all_pass = False
            break
    return TestTaskResult(task=TestTask(), bool_result=all_pass)

def get_quic_throughput_packet_loss_validation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(working_directory_filter="examples/quic/throughput_packet_loss", **kwargs)
    simulation_task.build = False
    simulation_task.simulation_config = simulation_task.tasks[0].simulation_config
    return ValidationTestTask(name="QUIC throughput validation test", tested_task=simulation_task, check_function=compute_quic_throughput_packet_loss_validation_test_results, **kwargs)

def run_quic_throughput_packet_loss_validation_test(test_accuracy=0.15, **kwargs):
    return get_quic_throughput_packet_loss_validation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

############################
# QUIC shared link test

def compute_quic_shared_link_from_simulation_results(**kwargs):
    results_path = inet_project.get_full_path("examples/quic/shared_link/results/")
    t0 = 3
    t1 = 13
    senders = 2
    runs = 4
    diffs = []
    for run in range(runs):
        file = results_path + "/" + str(run) + ".vec"
        tps = []
        for sender in range(senders):
            module_name = 'shared_link.receiver' + str(sender + 1) + '.quic'
            filter_expression = f"type =~ vector AND module =~ \"{module_name}\" AND name =~ \"packetReceived:vector(packetBytes)\""
            df = read_result_files(file, filter_expression=filter_expression)
            df = get_vectors(df)

            if not df.empty:
                vector_data = df.iloc[0]
                times = vector_data['vectime']
                values = vector_data['vecvalue']

                # Filter by time range
                mask = (times >= t0) & (times <= t1)
                filtered_values = values[mask]

                received_data = int(sum(filtered_values))
                tps.append(received_data * 8 / ((t1 - t0) * 1000))
            else:
                tps.append(0)

        larger_tp = max(tps)
        mean_tp = (tps[0] + tps[1]) / 2
        relative_diff_to_mean = larger_tp / mean_tp - 1
        diffs.append(relative_diff_to_mean)
    return fmean(diffs)

def compute_quic_shared_link_validation_test_results(test_accuracy=0.05, **kwargs):
    diffs_mean = compute_quic_shared_link_from_simulation_results(**kwargs)
    test_result = diffs_mean <= test_accuracy
    return TestTaskResult(task=TestTask(), bool_result=test_result)

def get_quic_shared_link_validation_test_task(**kwargs):
    multiple_simulation_tasks = get_simulation_tasks(working_directory_filter="examples/quic/shared_link", **kwargs)
    multiple_simulation_tasks.build = False
    multiple_simulation_tasks.simulation_config = multiple_simulation_tasks.tasks[0].simulation_config
    return ValidationTestTask(name="QUIC shared link validation test", tested_task=multiple_simulation_tasks, check_function=compute_quic_shared_link_validation_test_results, **kwargs)

def run_quic_shared_link_validation_test(test_accuracy=0.05, **kwargs):
    return get_quic_shared_link_validation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

#################################
# QUIC flow control limited test

def compute_quic_flow_control_from_simulation_results(**kwargs):
    results_path = inet_project.get_full_path("examples/quic/flow_control_limited/results/flow_control_validation_")
    rtts = [10, 20]
    initial_max_data_sizes = [65, 100, 140]
    start = 3
    stop = 4
    results = []
    for initial_max_data_size in initial_max_data_sizes:
        for rtt in rtts:
            file = results_path + "initialMaxData=" + str(initial_max_data_size) + "kB_delay=" + str(int(rtt / 2)) + ".vec"
            filter_expression = "type =~ vector AND module =~ \"bottleneck.receiver.quic\" AND name =~ \"streamRcvAppData_cid=0_sid=0:vector\""
            df = read_result_files(file, filter_expression=filter_expression)
            df = get_vectors(df)

            if not df.empty:
                vector_data = df.iloc[0]
                times = vector_data['vectime']
                values = vector_data['vecvalue']

                # Filter by time range
                mask = (times >= start) & (times < stop)
                filtered_values = values[mask]

                received_bits = sum(filtered_values)
                gp = (received_bits / 1000000 / (stop - start))
                results.append({'initial_max_data': initial_max_data_size, 'rtt': rtt, 'goodput': gp})
            else:
                results.append({'initial_max_data': initial_max_data_size, 'rtt': rtt, 'goodput': 0})
    return results

def compute_quic_flow_control_analytically():
    rtts = [10, 20]
    initial_max_data_sizes = [65, 100, 140]
    results = []
    for initial_max_data_size in initial_max_data_sizes:
        for rtt in rtts:
            gp_should = (initial_max_data_size * 8) / rtt
            results.append({'initial_max_data': initial_max_data_size, 'rtt': rtt, 'goodput': gp_should})
    return results

def compute_quic_flow_control_validation_test_results(test_accuracy=0.02, **kwargs):
    sim_results = compute_quic_flow_control_from_simulation_results(**kwargs)
    analytical_results = compute_quic_flow_control_analytically()
    all_pass = True
    for sim, ana in zip(sim_results, analytical_results):
        if not compare_test_results(ana['goodput'], sim['goodput'], test_accuracy):
            all_pass = False
            break
    return TestTaskResult(task=TestTask(), bool_result=all_pass)

def get_quic_flow_control_validation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(working_directory_filter="examples/quic/flow_control_limited", **kwargs)
    simulation_task.build = False
    simulation_task.simulation_config = simulation_task.tasks[0].simulation_config
    return ValidationTestTask(name="QUIC flow control validation test", tested_task=simulation_task, check_function=compute_quic_flow_control_validation_test_results, **kwargs)

def run_quic_flow_control_validation_test(test_accuracy=0.02, **kwargs):
    return get_quic_flow_control_validation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

###########################
# Multiple validation tests

def get_validation_test_tasks(**kwargs):
    """
    Returns multiple validation test tasks matching the provided filter criteria. The returned tasks can be run by
    calling the :py:meth:`run <inet.common.task.MultipleTasks.run>` method.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:meth:`get_simulation_tasks <inet.simulation.task.get_simulation_tasks>` method.

    Returns (:py:class:`MultipleTestTasks`):
        an object that contains a list of :py:class:`ValidationTestTask` objects matching the provided filter criteria.
        The result can be run (and re-run) without providing additional parameters.
    """
    validation_test_task_functions = [get_tsn_framereplication_simulation_test_task,
                                      get_tsn_trafficshaping_creditbasedshaper_validation_test_task,
                                      get_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_task,
                                      get_tsn_trafficshaping_asynchronousshaper_icct_simulation_test_task,
                                      get_quic_link_utilization_validation_test_task,
                                      get_quic_throughput_packet_loss_validation_test_task,
                                      get_quic_shared_link_validation_test_task,
                                      get_quic_flow_control_validation_test_task]
    validation_test_tasks = []
    for validation_test_task_function in validation_test_task_functions:
        validation_test_task_function_kwargs = kwargs.copy()
        for key in ["filter", "working_directory_filter", "ini_file_filter", "config_filter", "run_filter"]:
            validation_test_task_function_kwargs.pop(key, None)
        validation_test_task = validation_test_task_function(**validation_test_task_function_kwargs)
        simulation_config = validation_test_task.tested_task.simulation_config
        if simulation_config.matches_filter(**kwargs):
            validation_test_tasks.append(validation_test_task)
    return MultipleTestTasks(tasks=validation_test_tasks, name="validation test", **kwargs)

def run_validation_tests(**kwargs):
    """
    Runs one or more validation tests that match the provided filter criteria.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:func:`get_validation_test_tasks` function.

    Returns (:py:class:`MultipleTestTaskResults`):
        an object that contains a list of :py:class:`TestTaskResult` objects. Each object describes the result of running one test task.
    """
    return get_validation_test_tasks(**kwargs).run(**kwargs)
