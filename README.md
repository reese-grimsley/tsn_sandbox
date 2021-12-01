# Time Sensitive Networking (TSN) Testing and Sandbox Codebase

This is a repository to explore some of the dimensions of Time Sensitive Networking (TSN) on Linux machines, interconnected with a TSN-capable switch.

This repository is focused on configuring time synchronization, VLAN configuration for traffic prioritization, and LAN stress tests to verify the two latter points. The READMEs will also include information on how we configure our TSN switch. This codebase is developed as an exercise for understanding of TSN and how it can be used with the [ARENA](https://arenaxr.org/) [project](https://wise.ece.cmu.edu/projects/arena.html) at Carnegie Mellon to introduce more deterministic network to distributed runtime environments for applications like industrial automation.

## System Description

We have small network of three endpoint devices running linux, connected to a TSN-capable Ethernet Switch.

Each endpoint is an Intel NUC (**NUC11TNHv5**) running stock **Ubuntu 20.04 LTS**. They each have an Intel **i225-LM** Ethernet NICs (Network Interface Card) for hardware timestamping, queue management for QoS, and a few other TSN capabilities.  

The switch is a Belden/Hirschmann **BRS40-8TX** managed layer 2 switch for industrial applications (which we'll refer to as the 'switch' or 'TSN switch'). During the development of this codebase, it was running HiOS 8.7.02-S.

Please find a few additional notes on the hardware capabilities and constraints in <TODO!>.

## Repo Contents

* [PTP Configuration](ptp/README.md)
  * [gPTP Configuration](ptp/gptpREADME.md)
* [VLAN Configuration](VLAN_setup.md)
* [VLAN Latency Testing](latency_vlan_tests/README.md)
  * [Over Ethernet](latency_vlan_tests/README.md#Ethernet)
  * [Over UDP](latency_vlan_tests/README.md#UDP)
  * [Over TCP](latency_vlan_tests/README.md#TCP)
  * [Latency Log Processing](latency_vlan_tests/latency_processing/README.md)
* [Errata and other Mentionables](info_and_errata.md)  

## What is TSN?

As a whole, TSN is composed of 3 elements: time synchronization, traffic shaping / Quality of Service, and stream reservation / enforcement.  

These are generally encompassed in the 802.1Q standards from IEEE; TSN is not a standard in-and-of-itself, but an umbrella term for a *collection* of standards. TSN was borne from [Audio-Video Bridging](https://en.wikipedia.org/wiki/Audio_Video_Bridging) as the standards evolved and industrial domains realized their value in applications like factory automation. Note that time sychronization in TSN is effectively just the [Precision Time Protocol](https://en.wikipedia.org/wiki/Precision_Time_Protocol), which is governed under IEEE 1588 (and a sister specification for a simplified PTP configuration called generic PTP or [802.1AS](https://www.ieee802.org/1/pages/802.1as.html))

You may find a fairly technical slide deck from [Onera](https://www.onera.fr/en) that conscisely describes these standards and their purposes [here](https://www.onera.fr/sites/default/files/323/Slides-TSN-Training-public.pdf). A [white paper for Belden](https://www.belden.com/hubfs/resources/knowledge/white-papers/tsn-time-sensitive-networking.pdf) is also a useful read on the motivations and components of TSN, and the [AVNU Alliance](https://avnu.org/) similarly has helpful resources (albeit geared towards TSN product design) on concepts and best practices.

TSN is exists almost exclusively at the Data Link Layer (more specifcally, Ethernet). This is in large part due to the maturity of NICs that can add or manipulate tags in the Ethernet frame's header, which is a necessity for sub-microsecond time synchronization and data stream reservation.

### Is There a Catch?

Yes. In our experience (as of 2021), time synchronization and QoS management is mature in both Linux and the TSN switch. Stream reservation (argueably the most interesting element of TSN for networking with deterministic latency), has little evidence of a working, open-source implementation.  

Existing implementations like [OpenAVNU](https://github.com/Avnu/OpenAvnu) and [iotg_tsn_ref_sw](https://github.com/intel/iotg_tsn_ref_sw) have very specific hardware and software requirements, including custom linux kernels and out-of-date network drivers. A somewhat dated set of documents for [TSN for Linux](https://tsn.readthedocs.io/) covers VLANs, traffic shaping, and time synchronization, but does not include any note of stream reservation.

Further, the i225 NIC and TSN switch do not indicate they support this technology (standards 802.1Qat and 802.1Qcc for resevation and centralized coordination, respecitvely) as of December 2021, although future software versions might introduce this.  

This repository is instead focused on time synchronization, VLAN configuration for traffic prioritization, and LAN stress tests to verify the two latter points.  

## Disclaimer

This codebase is not intended for production level systems. It was developed as an exploration of TSN capabilities both in standard Linux machines and a designated switch.  

There is no guarantee this will work on an arbitrary Linux distribution or with an arbitrary TSN-compatible switch.

## License

TBD, but MIT is always a great option.  
