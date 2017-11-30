#!/bin/env python3

import os
import sys
import time
import subprocess

import boto3
from botocore.exceptions import ClientError

ec2 = boto3.resource("ec2")
cloudformation = boto3.resource("cloudformation")
autoscaling = boto3.client("autoscaling")
cloudwatch = boto3.resource("cloudwatch")


def import_key_pair(key_file_name, key_pair_name):
    """ imports an existing local key into aws """
    pass


def create_add_key_pair(key_pair_name):
    key_pair = ec2.create_key_pair(KeyName=key_pair_name)

    old_umask = os.umask(0o177)
    try:
        with open(key_pair_name + ".pem", "wt", 0o600) as key_file:
            key_file.write(key_pair.key_material)
    except IOError:
        pass
    finally:
        os.umask(old_umask)

    subprocess.call(["ssh-add", key_pair_name + ".pem"])


# which_asg is either "manager", or "worker".
# It is compared case-insensitively, because the official template
# named them "Manager" and "worker". And it bothers me.
def _get_asg_name(stack_name, which_asg):

    asgs = autoscaling.describe_auto_scaling_groups()

    # find the real name of the first AutoScalingGroup which
    # has the tag "Name" on it with the value stack_name + "-" + which_asg
    asg_name = [
        asg['AutoScalingGroupName']
        for asg in asgs['AutoScalingGroups']
        if [
            tag
            for tag in asg['Tags']
            if tag['Key'] == 'Name' and tag['Value'].lower() == (stack_name + '-' + which_asg).lower()
        ]
    ][0]

    return asg_name


def create_swarm(stack_name, key_pair_name, num_workers=3, instance_type="c4.xlarge"):
    """ creates the various resources on AWS using the official "Docker for AWS" CloudFormation Template """

    stack = cloudformation.create_stack(
        StackName=stack_name,
        TemplateURL='https://editions-us-east-1.s3.amazonaws.com/aws/stable/Docker.tmpl',
        Capabilities=['CAPABILITY_IAM'],
        Parameters=[
            {'ParameterKey': 'KeyName',
             'ParameterValue': key_pair_name},
            {'ParameterKey': 'InstanceType',
             'ParameterValue': instance_type},
            {'ParameterKey': 'ManagerInstanceType',
             'ParameterValue': instance_type},
            {'ParameterKey': 'ManagerSize',
             'ParameterValue': '1'},
            {'ParameterKey': 'ClusterSize',
             'ParameterValue': str(num_workers)},
        ])

    print("Waiting for resources to be created... This will take about 10 minutes")
    while stack.stack_status != 'CREATE_COMPLETE':
        time.sleep(5)

        stack.reload()
        resources = list(stack.resource_summaries.iterator())

        num_ready = sum(
            1 for r in resources if r.resource_status == 'CREATE_COMPLETE')
        num_creating = sum(
            1 for r in resources if r.resource_status == 'CREATE_IN_PROGRESS')

        # erase the current output line
        sys.stdout.write(u"\u001b[1000D\u001b[0K")

        print("|" + num_ready * "=" + num_creating * "+" +
              (42 - num_creating - num_ready) * " " + "|", end="", flush=True)

    print()

    print("Creating alarm and scaling policy to terminate all instances after a long period of inactivity")

    manager_asg_name = _get_asg_name(stack_name, "manager")
    worker_asg_name = _get_asg_name(stack_name, "worker")

    # shut down all instances if the alarm is fired
    manager_scaling_policy = autoscaling.put_scaling_policy(
        AutoScalingGroupName=manager_asg_name,
        PolicyName='terminate-manager', PolicyType="SimpleScaling",
        AdjustmentType="ExactCapacity", ScalingAdjustment=0)

    worker_scaling_policy = autoscaling.put_scaling_policy(
        AutoScalingGroupName=worker_asg_name,
        PolicyName='terminate-workers', PolicyType="SimpleScaling",
        AdjustmentType="ExactCapacity", ScalingAdjustment=0)

    # alarm if in the last 12 consecutive 5-minute periods the maximum CPU utilization of the manager was below 10%
    # and execute both terminate policies
    metric = cloudwatch.Metric(namespace='AWS/EC2', name='CPUUtilization')
    metric.put_alarm(
        AlarmName=stack_name + "-manager-idle",
        Dimensions=[{'Name': 'AutoScalingGroupName',
                     'Value': manager_asg_name}],
        Statistic="Maximum",
        AlarmActions=[manager_scaling_policy["PolicyARN"],
                      worker_scaling_policy["PolicyARN"]],
        Period=5 * 60, EvaluationPeriods=12, ComparisonOperator='LessThanThreshold', Threshold=10)

    print("Done.")


def connect_to_swarm(stack_name):
    """ creates an SSH tunnel to the manager machine, forwarding all necessary ports """

    manager_asg_name = _get_asg_name(stack_name, "manager")

    group_info = autoscaling.describe_auto_scaling_groups(
        AutoScalingGroupNames=[manager_asg_name])

    manager_instance_id = group_info['AutoScalingGroups'][0]['Instances'][0]['InstanceId']
    manager_instance = ec2.Instance(manager_instance_id)
    manager_public_ip = manager_instance.public_ip_address

    subprocess.call([
        "ssh", "docker@" + manager_public_ip, "-N", "-o", "StrictHostKeyChecking=no",
        "-L", "localhost:2374:/var/run/docker.sock", "-L", "9181:172.17.0.1:9181",
        "-L", "8080:172.17.0.1:8080", "-L", "27017:172.17.0.1:27017", "-L", "6379:172.17.0.1:6379"])

    # from now on you can do:  docker -H tcp://localhost:2374 info

    print("SSH exited, tunnel collapsed.")


def deploy_app():
    subprocess.call(["docker", "-H", "tcp://localhost:2374", "stack", "up",
                     "--compose-file", "docker-compose-fingerprints.yml", "inet"])


def halt_swarm():
    pass


def resume_swarm():
    pass


def remove_swarm(stack_name):
    stack = cloudformation.Stack(stack_name)

    stack.delete()
    print("Waiting for resources to be deleted... This will take a couple minutes.")

    while stack.stack_status == 'DELETE_IN_PROGRESS':
        time.sleep(5)

        try:
            stack.reload()
            resources = list(stack.resource_summaries.iterator())
        except ClientError:
            # the stack might not be there anymore
            break

        num_deleting = sum(
            1 for r in resources if r.resource_status == 'DELETE_IN_PROGRESS')
        num_remaining = sum(
            1 for r in resources if r.resource_status == 'CREATE_COMPLETE')

        # erase the current output line
        sys.stdout.write(u"\u001b[1000D\u001b[0K")

        print("|" + num_remaining * "=" + num_deleting * "-" +
              (42 - num_remaining - num_deleting) * " " + "|", end="", flush=True)

    print()

    # remove the cpu idle termination metrics/alarms/policies

    print("Done.")
