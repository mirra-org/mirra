<style>
  .md-typeset h1,
  .md-content__button {
    display: none;
  }
</style>
<p align="center"><img src="./assets/logo.svg#only-light" width=650><img src="./assets/logo_dark.svg#only-dark" width=650></p>
<p align="center"><em>Microclimate Instrument for Real-time Remote Applications</em></p>

---

<p align="center">This is a research project. Limited testing has been performed and reliability of data storage, sensor readout and communication is not guaranteed.</p>
<p align="center"><b> ⚠️ Use with caution. ⚠️</b></p>

---

MIRRA is a fully integrated solution for the managing of large-scale sensor systems, specifically geared towards the study of microclimates.
Sensor data is collected and uploaded in near real-time to a centralised server, abolishing the need for laborious manual readouts of sensors.

To accomplish this, MIRRA's architecture is compromised of a three-tier system:

- **Nodes**: perform intermittent readouts of attached sensors and forwards data to the gateway
- **Gateways**: collect data from nearby sensor nodes over LoRa and upload them to the server
- **Server**: collects data from gateways over MQTT and provides an interface for viewing this data and managing gateways

The MIRRA project maintains three separate open-source codebases that serve to enable this system.

- [**Hardware**](https://github.com/Olocool17/MIRRA/tree/main/pcb_boards): PCBs designed in KiCad for the nodes and gateways, powered by an ESP32 microcontroller
- [**Firmware**](https://github.com/Olocool17/MIRRA/tree/main/firmware): C++ code for the nodes and gateway that orchestrates sensor readout and communication
- [**Server**](https://github.com/Olocool17/MIRRA/tree/main/webserver): Python code for the scripting and Docker deployment of the server

To learn more on how to deploy MIRRA yourself or how to interface with an already existing MIRRA installationm please view the [User Manual](./user_manual).

Developers who wish to contribute to the project or achieve a deeper understanding of the underlying technology may wish to consult the [Developer Manual](./developer_manual).
