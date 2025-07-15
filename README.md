<div id="top">
  <p>
  <h2>This project is work in progress and is not for use or testing yet! Really, you don't want to use it at the moment!</h2>
  </p>
  <br />
</div>

<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]


<!-- PROJECT LOGO -->
<br />
<div align="center">
   <a href="https://github.com/OpenAstroTech/OpenAstroFirmware">
   <img src="assets/images/logo.png" alt="Logo" width="80" height="80">
   </a>
   <h3 align="center">OpenAstroFirmware</h3>
   <p align="center">
      Official Firmware for DIY astronomical telescope mounts. This firmware is the 2.x successor of the <a href="https://github.com/OpenAstroTech/OpenAstroTracker-Firmware">OpenAstroTracker-Firmware</a>.
   </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#supported-hardware">Supported hardware</a></li>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#configuration">Configuration</a></li>
        <li><a href="#build">Build</a></li>
        <li><a href="#upload">Upload</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

It was a very long and educational time developing, testing and improving [OpenAstroTracker-Firmware](https://github.com/OpenAstroTech/OpenAstroTracker-Firmware) for all of us. It evolved and grew over time as did our hardware support. Amount and type of supported mounts, components, addons and software tools keep increasing. This is why dev team decided to go one step back and redesign the firmware based on the experience with v1 and community feedback and requests. This firmware aims to bring following improvements over time compared to OpenAstroTracker-Firmware:

### Usage
* Easier configuration, flashing and updates
* Improved stability
* Improved/Automated calibration
* New types of addons (these could be among other things Touch display, Mobile app etc.)
* Ability to track "custom" objects (e.g. Sun, Moon, ISS, Comets etc.)
* Support for more types of mounts (OAT, OAM, any further Mounts and their versions designed by OpenAstroTech and retrofitted 3rd party mounts)
* Several new QoL features

### Development
* Easier maintenability and extendability
* Automated testing
* Support for modern 32-bit boards to improve performance, accuracy and reduce the need for hardcore optimizations.
* In hardware debugging
* Higher code quality by following best practices and a predefined architecture design
* Ability to test and run main code and test directly on the host pc to simplify issue analysis
* Support and usage of modern c++ features to improve readability and clarity of the code

<p align="right">(<a href="#top">back to top</a>)</p>



### Built With

* [Zephyr RTOS](https://docs.zephyrproject.org/) - Real-time operating system for connected, resource-constrained devices
* [West](https://docs.zephyrproject.org/latest/guides/west/index.html) - Zephyr's meta-tool for managing project repositories
* [CMake](https://cmake.org/) - Cross-platform build system
* C++ - Modern C++ for telescope mount control implementation

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

To get started with OpenAstroFirmware development or to build and deploy the firmware for your telescope mount, follow these steps. The project supports both hardware deployment (MKS Robin Nano) and native simulation for development and testing.

### Supported hardware

- **MKS Robin Nano** (STM32F407xx-based controller board) - Production target
- **native_sim** - Development and testing platform
- STM32F407xx microcontroller family support
- Stepper motor drivers:
  - TMC stepper motor controllers
  - GPIO-based stepper motor drivers  
  - Testing/simulation drivers for development

### Development Environment Setup

Before getting started, make sure you have a proper Zephyr development environment. Follow the official [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

#### Prerequisites

- **Zephyr SDK** - Required for cross-compilation and board support
- **West** - Zephyr's meta-tool for project management and building
- **CMake** (version 3.20.0 or higher) - Build system
- **Ninja** - Fast build tool
- **Python 3** (with pip) - For build scripts and utilities
- **Git** - For version control and West manifest management

#### Installation

1. **Install the Zephyr SDK**: Follow the [official Zephyr getting started guide](https://docs.zephyrproject.org/latest/getting_started/index.html) for your operating system.

2. **Set up Python virtual environment and install West**:
   ```bash
   python3 -m venv ~/.venv
   source ~/.venv/bin/activate
   pip install west
   ```

#### Workspace Initialization

The first step is to initialize the workspace folder where the `OpenAstroFirmware` and all Zephyr modules will be cloned. Run the following command:

```bash
# Initialize workspace for OpenAstroFirmware
west init -m https://github.com/OpenAstroTech/OpenAstroFirmware --mr main OpenAstroTech-workspace
# Update Zephyr modules
cd OpenAstroTech-workspace
west update
```

After initialization, your workspace structure will look like:
```
OpenAstroTech-workspace/
├── OpenAstroFirmware/     # This repository
├── zephyr/                # Zephyr RTOS
└── modules/               # Zephyr modules (HAL, libraries, etc.)
```

**Important**: Remember to activate your Python virtual environment each time you start working on the project:
```bash
source ~/.venv/bin/activate
```

### Configuration

The firmware uses Zephyr's Kconfig system for configuration. Key configuration files:

- `app/prj.conf` - Main project configuration
- `app/boards/native_sim.conf` - Native simulator specific settings  
- `app/boards/native_sim.overlay` - Device tree overlay for simulation
- Board-specific configurations in `boards/` directory

Configuration can be modified using:
```bash
west build -t menuconfig    # Interactive configuration menu
west build -t guiconfig     # GUI configuration tool
```

### Build

#### For Native Simulation (Development/Testing)
```bash
cd OpenAstroTech-workspace/OpenAstroFirmware/app
west build -b native_sim
```

#### For MKS Robin Nano (Production Hardware)
```bash
cd OpenAstroTech-workspace/OpenAstroFirmware/app  
west build -b mks_robin_nano
```

#### Clean Build
```bash
west build -t pristine     # Clean all build artifacts
west build -b <board_name> # Rebuild from scratch
```

The build system supports:
- Cross-compilation for STM32F407xx targets
- Native compilation for PC-based testing
- Comprehensive testing with automated test suites

### Upload

#### Native Simulation
```bash
west build -t run          # Run the firmware in simulation
```

#### Hardware Targets (MKS Robin Nano)
```bash
west flash                  # Flash firmware to connected hardware
west debug                  # Start debugging session
west attach                 # Attach debugger to running target
```

Additional upload options:
- `west build -t flash` - Alternative flash command
- Custom flash runners supported via Zephyr's runner system

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

OpenAstroFirmware provides telescope mount control with the following capabilities:

### Core Features
- **LX200 Protocol Support** - Compatible with Meade LX200 command set for telescope control software
- **Stepper Motor Control** - Precise control of RA and DEC axes with multiple driver support
- **Mount Coordinate Management** - Right Ascension and Declination positioning
- **Cross-Platform Development** - Native simulation for testing without hardware

### LX200 Command Interface
The firmware implements the standard LX200 protocol for communication with planetarium software like:
- SkySafari
- Stellarium  
- TheSkyX
- Cartes du Ciel
- Any ASCOM-compatible software

### Hardware Control
- Stepper motor drivers (TMC, GPIO-based)
- STM32F407xx microcontroller support
- Extensible driver architecture for additional hardware components

### Development and Testing
- Native simulation support for PC-based development
- Comprehensive logging system
- Automated testing framework
- In-hardware debugging capabilities

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [x] Build environment setup
    - [x] Zephyr RTOS integration  
    - [x] West workspace configuration
    - [x] Cross-compilation support
    - [x] Native simulation target
- [x] Core Foundation
    - [ ] LX200 protocol library implementation
    - [ ] Mount coordinate management (RA/DEC)
    - [ ] Stepper motor driver framework
    - [x] Board support for MKS Robin Nano
    - [x] Logging and debugging infrastructure
- [ ] MVP (In Progress)
    - [ ] Basic mount control interface
    - [ ] Complete LX200 command set implementation
    - [ ] Motor calibration and homing
    - [ ] Real-time telescope tracking
    - [ ] Hardware-in-the-loop testing
- [ ] Future Enhancements
    - [ ] Touch display interface
    - [ ] Mobile app connectivity
    - [ ] Custom object tracking (Sun, Moon, ISS, Comets)
    - [ ] Advanced calibration procedures
    - [ ] Extended mount type support

See the [open issues](https://github.com/OpenAstroTech/OpenAstroFirmware/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Project Link: [https://github.com/OpenAstroTech/OpenAstroFirmware](https://github.com/OpenAstroTech/OpenAstroFirmware)

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/OpenAstroTech/OpenAstroFirmware.svg?style=for-the-badge
[contributors-url]: https://github.com/OpenAstroTech/OpenAstroFirmware/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/OpenAstroTech/OpenAstroFirmware.svg?style=for-the-badge
[forks-url]: https://github.com/OpenAstroTech/OpenAstroFirmware/network/members
[stars-shield]: https://img.shields.io/github/stars/OpenAstroTech/OpenAstroFirmware.svg?style=for-the-badge
[stars-url]: https://github.com/OpenAstroTech/OpenAstroFirmware/stargazers
[issues-shield]: https://img.shields.io/github/issues/OpenAstroTech/OpenAstroFirmware.svg?style=for-the-badge
[issues-url]: https://github.com/OpenAstroTech/OpenAstroFirmware/issues
[license-shield]: https://img.shields.io/github/license/OpenAstroTech/OpenAstroFirmware.svg?style=for-the-badge
[license-url]: https://github.com/OpenAstroTech/OpenAstroFirmware/blob/master/LICENSE
