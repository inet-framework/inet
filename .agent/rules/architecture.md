---
trigger: always_on
glob:
description: This document provides a comprehensive guide for AI agents and developers to navigate, understand, and work with the INET Framework repository.
---
# INET Framework Workspace Rules & Agent Guide

## 1. Project Overview

The INET Framework is an open-source communication networks simulation package for the OMNeT++ simulation environment. It contains models for the Internet stack (TCP, UDP, IPv4, IPv6, OSPF, BGP, etc.), wired and wireless link layers (Ethernet, IEEE 802.11, etc.), support for mobility, MANET protocols, DiffServ, MPLS with RSVP-TE and LDP, and many other protocols and components.

### Key Directories

- **`src/inet`**: The core source code of the framework.
  - `applications`: Traffic generators and application models (HTTP, Ping, DHCP, etc.).
  - `transportlayer`: Transport protocols (TCP, UDP, SCTP).
  - `networklayer`: Network protocols (IPv4, IPv6, ICMP, ARP).
  - `routing`: Routing protocols (AODV, OSPF, BGP, RIP).
  - `linklayer`: L2 protocols (Ethernet, IEEE 802.11, PPP).
  - `physicallayer`: Radio propagation, antenna, and transmitter/receiver models.
  - `mobility`: Node mobility models.
  - `node`: Pre-assembled node models (StandardHost, Router, AccessPoint).
- **`examples`**: Simulation examples demonstrating various features. Organized by protocol or feature.
- **`tests`**: Regression and unit tests.
- **`showcases`**: More complex, tutorial-style simulation scenarios.
- **`tutorials`**: Step-by-step tutorials for learning INET.

## 2. Development Workflow

### Building the Project

1.  **Environment Setup**: Ensure OMNeT++ and INET is in your PATH. i.e. `opp_run`, `inet` and `inet_dbg` are available in the PATH after sourcing the environment. If not, source them from the root of the repository and from ~/omnetpp-6.3.0

2.  **Compile**: Build the project using `make`. Use `MODE=debug` or `MODE=release` to
    select the build mode. Prefer debug mode for development.
    Use parallel build for speed:
    ```bash
    make -j$(nproc) MODE=debug
    ```
    *Note: The output binary is usually a shared library or the `inet` or `inet_dbg` executable in the `src` directory.*

### Running Simulations

Simulations are defined by `.ned` files (network topology) and `omnetpp.ini` (configuration).

1.  **Navigate** to the directory where the .ini file is located (e.g., `examples/aodv`).
2.  **Run** using the `inet` or `inet_dbg` executable depending on build mode with additional arguments as needed:

    - `-u Cmdenv`: Runs in command-line mode (faster, no GUI, preferred if no user interaction is required).
    - `-c <ConfigName>`: Selects a configuration from `omnetpp.ini`.
    - `-r <RunNumber>`: Selects a specific run (if using parameter studies).

### Modifying Code

- **NED Files (`.ned`)**: Define module structure, parameters, and gates.
- **Message Files (`.msg`)**: Define packet and message structures. These generate `_m.h` and `_m.cc` files.
- **C++ Files (`.cc`, `.h`)**: Implement module behavior.
  - **Simple Modules**: Implement `handleMessage()` or `activity()`.
  - **Channel Models**: Implement transmission logic.

## 3. Navigation & Understanding

### Finding Code

- **Protocols**: Look in `src/inet/<layer>/<protocol>`.
  - Example: AODV is in `src/inet/routing/aodv`.
  - Example: TCP is in `src/inet/transportlayer/tcp`.
- **Interfaces**: Interface definitions (e.g., `IApp`, `IMac`) are often in `contract` subpackages or base classes.

### Understanding NED

- **`simple`**: A basic module implemented in C++.
- **`module`**: A compound module composed of other modules.
- **`network`**: A top-level module that can be run as a simulation.
- **`@display`**: Visual properties (ignore for logic).
- **`gates`**: Input/output ports for connecting modules.

### Understanding `.ini` files

- **`[General]`**: Global settings.
- **`[Config <Name>]`**: Specific simulation scenarios.
- **`extends`**: Inherits settings from another config.
- **`**.param = value`**: Wildcard matching to set module parameters.

## 4. Testing

- **Fingerprint Tests**: Regression tests that check if simulation output matches a reference "fingerprint".
  - Located in `tests/fingerprint`.
  - Run via script or `make check` (if configured).
- **Unit Tests**: C++ unit tests in `tests/unit`.

## 5. Common Issues & Tips

- **"Class not found"**: Usually means the C++ class is not linked or not registered with `Define_Module()`.
- **"Parameter not set"**: A module parameter lacks a default value and wasn't set in `omnetpp.ini`.
- **Dangling Pointers**: Common in C++. Use OMNeT++'s `check_and_cast` or `dynamic_cast` carefully.
- **Simulation Time**: Use `simTime()` to get current simulation time.

## 6. Agent-Specific Instructions
- **Search First**: Before modifying, search for existing implementations or base classes.
- **Check Dependencies**: When modifying a module, check its `.ned` file for dependencies (imports).
- **Verify**: After changes, try to compile (`make`) and run a relevant example to ensure no regressions.

