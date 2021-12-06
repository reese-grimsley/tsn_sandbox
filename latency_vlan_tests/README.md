# Latency Testing

The C files in this directory are used to experiment with VLAN configurations within the switch and endpoints to understand their impact on the latency and packet-drop rate.

The network in this application consists of 3 endpoints (11th gen NUC mini-PC's with Intel i225-LM NICs) running Ubuntu 20.04 LTS. Each endpoint is directly connected to a TSN-enabled switch. One endpoint is a data **source**, producing information that needs to get to another endpoint, acting as the **sink**. At the same time, a **jammer** is injecting meaningless data into the network, ideally towards the sink to saturate that link and prevent source-data from getting there. Note that in the network used for testing, there is only 1 switch.

To get a measure of latency, the source sends data with a recent timestamp and the sink retrieves a hardware timestamp from when that message was received (note that the source is not using the actual time of sending, but just before - using the real TX time would require a follow-up message containing that value). The sink will store TX-RX latency, along with a frame-counter, experiment ID, and priority into a CSV file, which can be analyzed and plotted using some [latency processing python scripts](latency_processing/README.md)

## Ethernet

## UDP

## TCP

