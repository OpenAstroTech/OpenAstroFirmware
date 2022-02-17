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
   <img src="images/logo.png" alt="Logo" width="80" height="80">
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

* [PlatformIO](https://platformio.org/)

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

This is an example of how you may give instructions on setting up your project locally.
To get a local copy up and running follow these simple example steps.

### Supported hardware

*TBD*

### Prerequisites

*TBD*

### Configuration

*TBD*

### Build

*TBD*

### Upload

*TBD*

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

*TBD*

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [ ] Build environment setup
    - [ ] *TBD*
- [ ] MVP
    - [ ] *TBD*

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
[issues-shield]: https://img.shields.io/github/issuesOpenAstroTech/OpenAstroFirmware.svg?style=for-the-badge
[issues-url]: https://github.com/OpenAstroTech/OpenAstroFirmware/issues
[license-shield]: https://img.shields.io/github/license/OpenAstroTech/OpenAstroFirmware.svg?style=for-the-badge
[license-url]: https://github.com/OpenAstroTech/OpenAstroFirmware/blob/master/LICENSE
