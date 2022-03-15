# home-energy-monitor-esp32
This project simplifies the previous attempt at a home-energy-monitor (which used RPI & AWS Greengrass)
and instead uses a single ESP32 and allows prometheus to scrape the metrics.

This allows the metrics to get ingested by an external prometheus system on the network, without the significant complexity
of AWS GreenGrass, including but not limited to: development environment, deployments, OS updates, Python upgrades, etc.

Network based services (water monitoring and heating monitoring) will be reimplemented into docker images running on Kubernetes.

This reduces the coupling between ingestion.

# Components
## Power Monitor
The power monitor is based off an Ardiuno Nano and a stripboard based from Open Energy Monitors Buffered Voltage Bias
design. The code is included within the `power-monitor\ardiuno` directory and uses the `EmonLib` library for calculate the real power usage.
This code can be compiled using https://create.arduino.cc or PlatformIO.
The data is then sent over bluetooth (using a HC-05) to the ESP32, where it is made available for scraping.

Buffered Voltage Schematic: https://learn.openenergymonitor.org/electricity-monitoring/ctac/acac-buffered-voltage-bias

_Note: It's possible to make the core HEM also perform power monitoring, however, this board and stripboard is being reused
from the previous project and no value would be added by reimplementing this work._

## HEM (Home Energy Monitor)
A ESP32 DevKit board is connected to 3 one-wire sensors, which are each located within temperature probe locations
within a Heat Store water unit. The sensors are polled periodically, before the metrics are made available for scraping.

The HEM also listens for Bluetooth messages from the Power Monitor, making the metrics available for scraping.

# Getting Started

## Tooling
- Visual Code w/ PlatformIO Plugin

