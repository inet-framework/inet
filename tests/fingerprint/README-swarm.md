# Introduction

## Docker Swarm



# How it works

This is a distributed application, built on top of Docker Swarm[1]. It is composed of
7 different services.



[1] The new, integrated "Swarm Mode", not the legacy docker-swarm utility.

## Services

-  redis:
-  mongo:
-  builder:
-  runner:
-  visualizer:
-  dashboard:
-  distcc:

## Networks
  interlink:
  buildnet:


# Deployment on local swarm



# Deployment on AWS

## What you will need

- AWS account
- AMI tokens
- SSH keypair
- python3, ssh client, docker (client), pip3
    - pip packages: aws, boto3

## Preparation

- create AMI app token
- aws configure



## Creating the swarm

Deploying the CloudFormation template cupplied by Docker, called Docker for AWS.


## Connecting to the swarm



