---
trigger: glob
glob: **/*.ini
description: Rules and best practices for editing and creating INI files in the INET Framework
---
# INET Framework .ini File Rules

## 1. General Structure

- **[General] Section**: Every `.ini` file MUST have a `[General]` section at the top. This section should contain:
    - `network`: The name of the NED network to be simulated (unless specified in sub-configs).
    - Global simulation limits (e.g., `sim-time-limit`, `cpu-time-limit`).
    - Global random number generator settings (`num-rngs`, seeds).
    - Shared module parameters (e.g., visualizer settings, global physical environment).

- **Config Sections**: Use `[Config <Name>]` to define specific simulation scenarios.
    - **Naming**: Use PascalCase for config names (e.g., `[Config SimpleRREQ]`).
    - **Description**: Every config MUST have a `description = "..."` entry explaining its purpose.
    - **Inheritance**: Use `extends = <ParentConfig>` to inherit settings from other configurations. This reduces duplication and keeps files organized.

- **Abstract Configs**: If a configuration is intended as a base for others and not to be run directly, set `abstract = true`.

## 2. Parameter Assignment

- **Wildcards**: Use wildcards (`**`, `*`) judiciously to set parameters for multiple modules.
    - `**` matches any number of hierarchy levels.
    - `*` matches one level or a part of a name.
    - Example: `**.mobility.constraintAreaMaxX = 600m` (sets it for all mobility modules anywhere).

- **Specific Parameters**: Be specific when necessary to avoid unintended side effects.
    - Example: `*.host[0].app[0].destAddr = "host[1]"`

- **Units**: ALWAYS include units for physical quantities (time, distance, bitrate, etc.).
    - Time: `s`, `ms`, `us`, `ns`
    - Distance: `m`, `km`
    - Data rate: `bps`, `kbps`, `Mbps`, `Gbps`
    - Data size: `B`, `kB`, `MB`
    - Power: `W`, `mW`, `dBm`

- **Random Distributions**: Use OMNeT++ distribution functions for variability.
    - Example: `uniform(1s, 5s)`, `exponential(100ms)`, `normal(10mps, 2mps)`.

## 3. Organization and Style

- **Comments**: Use `#` for comments.
    - Group related settings with section headers (e.g., `# Mobility`, `# Application`, `# Wi-Fi`).
    - Explain complex parameter choices.

- **Ordering**:
    1.  `[General]`
    2.  Base configurations (Abstract)
    3.  Specific configurations (derived from base)

- **Formatting**:
    - Use spaces around `=` (e.g., `key = value`).
    - Indentation is not strictly required but consistent indentation improves readability.

## 4. Common Patterns

- **Visualizers**: Enable visualizers in the `[General]` or base config to aid debugging and demonstration.
    - Example: `*.visualizer.interfaceTableVisualizer.displayInterfaceTables = true`

- **Scenario Manager**: Use `ScenarioManager` with XML scripts for dynamic events (node failure, mobility changes).
    - `*.scenarioManager.script = xmldoc("scenario.xml")`

- **Configurators**: Use `Ipv4NetworkConfigurator` (or IPv6 equivalent) for automatic IP address assignment and routing.
    - `*.configurator.config = xml(...)` can be used for custom configuration.

## 5. Example Template

```ini
[General]
network = MyNetwork
sim-time-limit = 100s
num-rngs = 3
# ... global settings ...

[Config Base]
description = "Base configuration with common settings"
abstract = true
# ... common parameters ...

[Config Scenario1]
description = "Scenario 1: Low traffic"
extends = Base
# ... specific settings ...

[Config Scenario2]
description = "Scenario 2: High traffic"
extends = Base
# ... specific settings ...
```
