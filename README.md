<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Thanks again! Now go create something AMAZING! :D
***
***
***
*** To avoid retyping too much info. Do a search and replace for the following:
*** yamaha-bps, cbr_utils, twitter_handle, thomas_gurriet@yamaha-motor.com, Cyber Utilities, Collection of various programmatic tools.
-->



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
[![Coverage][coverage-shield]][coverage-url]


<!-- PROJECT LOGO -->
<br />
<p align="center">
  <a href="https://github.com/yamaha-bps/cbr_utils">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">Cyber Utilities</h3>

  <p align="center">
    Collection of various C++ programmatic tools.
    <br />
    <a href="https://github.com/yamaha-bps/cbr_utils/"><strong>Explore the repo»</strong></a>
    <br />
    <a href="https://yamaha-bps.github.io/cbr_utils/"><strong>Explore the docs»</strong></a>
    <br />
    <br />
    <a href="https://github.com/yamaha-bps/cbr_utils/issues">Report Bug</a>
    ·
    <a href="https://github.com/yamaha-bps/cbr_utils/issues">Request Feature</a>
  </p>
</p>



<!-- TABLE OF CONTENTS -->
## Table of Contents
<ol>
  <li><a href="#content-overview">Content Overview</a></li>
  <li><a href="#dependencies">Dependencies</a></li>
  <li>
    <a href="#getting-started">Getting Started</a>
    <ul>
      <li><a href="#prerequisites">Prerequisites</a></li>
      <li><a href="#build-and-install">Installation</a></li>
    </ul>
  </li>
  <li><a href="#use-with-cmake">Usage</a></li>
  <li><a href="#roadmap">Roadmap</a></li>
  <li><a href="#contributing">Contributing</a></li>
  <li><a href="#license">License</a></li>
  <li><a href="#contact">Contact</a></li>
  <li><a href="#acknowledgements">Acknowledgements</a></li>
</ol>




## Content overview
All the provided utilities are in the `cbr` namespace and are `C++17` compatible, except for [matplotlibcpp.hpp](include/cbr_utils/matplotlibcpp.hpp) for which things have been left in the original `matplotlibcpp` namespace, and which leverages `C++20` constructs.
### Clocks and timers
### Compile time loop
### Digitset
### Enum
### Integer sequence
### List of types
### Plotting
### Synchronization
### Thead pool
### Type traits
### Yaml
### Misc

## Dependencies

### Required
* [Libboost](https://www.boost.org/)
* [yaml-cpp](https://github.com/jbeder/yaml-cpp)

### Optional
* [GTest](https://github.com/google/googletest) (to build tests)
* [Python](https://www.python.org/), [Numpy](https://numpy.org/), [Matplotlib](https://matplotlib.org/) (to use matplotlibcpp.hpp header)


<!-- GETTING STARTED -->
## Getting Started

To get a local copy up and running follow these simple steps.


### Prerequisites

* libboost
  ```sh
  sudo apt install libboost-dev
  ```

* yaml-cpp
  ```sh
  sudo apt install libyaml-cpp-dev
  ```
#### Optional
* GTest (only necessary to build tests)
  ```sh
  sudo apt install libgtest-dev
  ```

* Python, Numpy, Matplotlib (only necessary to use matplotlibcpp)
  ```sh
  sudo apt install libpython3-dev python3-matplotlib python3-numpy
  ```

### Build and Install

1. Clone the repo
   ```sh
   git clone https://github.com/yamaha-bps/cbr_utils.git
   ```
2. Make build directory
   ```sh
   mkdir cbr_utils/build && cd cbr_utils/build
   ```
3. Build
   ```sh
   cmake ..
   make
   ```

4. To build examples (optional)
   ```sh
   cmake .. -DBUILD_EXAMPLES=ON
   make
   ```

5. Install
   ```sh
   sudo make install
   ```

6. To uninstall if you don't like it
   ```sh
   sudo make uninstall
   ```

7. To build and run the tests:
   ```sh
   cmake .. -DBUILD_TESTING=ON
   make
   make test
   ```

## Use with cmake

To utilize `cbr_utils` in your own project, include something along these lines in your `CMakeLists.txt`
```cmake
find_package(cbr_utils)

add_executable(my_executable main.cpp)
target_link_libraries(my_executable cbr_utils::cbr_utils)
```

<!-- ROADMAP -->
## Roadmap

See the [open issues](https://github.com/yamaha-bps/cbr_utils/issues) for a list of proposed features (and known issues).

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE` for more information.



<!-- CONTACT -->
## Contact

Thomas Gurriet - thomas_gurriet@yamaha-motor.com

Project Link: [https://github.com/yamaha-bps/cbr_utils](https://github.com/yamaha-bps/cbr_utils)



<!-- ACKNOWLEDGEMENTS -->
## Acknowledgements

* [Yamaha Motor Corporation](https://yamaha-motor.com/)


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[coverage-shield]: https://img.shields.io/codecov/c/github/yamaha-bps/cbr_utils?style=for-the-badge&token=EA1KEBJWBU
[coverage-url]: https://codecov.io/gh/yamaha-bps/cbr_utils
[contributors-shield]: https://img.shields.io/github/contributors/yamaha-bps/cbr_utils.svg?style=for-the-badge
[contributors-url]: https://github.com/yamaha-bps/cbr_utils/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/yamaha-bps/cbr_utils.svg?style=for-the-badge
[forks-url]: https://github.com/yamaha-bps/cbr_utils/network/members
[stars-shield]: https://img.shields.io/github/stars/yamaha-bps/cbr_utils.svg?style=for-the-badge
[stars-url]: https://github.com/yamaha-bps/cbr_utils/stargazers
[issues-shield]: https://img.shields.io/github/issues/yamaha-bps/cbr_utils.svg?style=for-the-badge
[issues-url]: https://github.com/yamaha-bps/cbr_utils/issues
[license-shield]: https://img.shields.io/github/license/yamaha-bps/cbr_utils.svg?style=for-the-badge
[license-url]: https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE
