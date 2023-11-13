# System Info Browser - C++ Machine Information Browsing and Collection Tool

## Overview

System Info Browser is a C++ project designed to gather comprehensive information about the host machine and display it in a user-friendly UI with the option to save it to a CSV file. This tool is aimed at providing a detailed snapshot of the host system, including hardware specifications, operating system details, system resources, and other relevant information.

## Features

- Hardware tempratures.
- System utilization.
- Memory information.
- Storage devices information.
- Volumes information.
- Network adapters information.
- Processes utilization and information.
- Saves the gathered information to a CSV file for easy analysis and sharing.
- Small size and fast execution.

## Platform Compatibility

The project is currently developed using Microsoft Visual C++ and is intended for Windows platforms. However, there are plans to make the application cross-platform:

- **Linux**: Cross-platform support for Linux will begin very soon.
- **macOS**: Support for macOS is under consideration for future releases.

## Helpful Resources

The following resources were key for this project success:

- [The library providing hardware and sensors information for Windows](https://github.com/openhardwaremonitor/openhardwaremonitor)
- [This question guided me to choosing the library above](https://stackoverflow.com/questions/23314886/get-cpu-temperature)
- [Oracle curses library function documentation](https://docs.oracle.com/cd/E36784_01/html/E36880/makehtml-id-6.html)
- [ncurses programming how-to](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html)
- [Reading this article helped me to come up with an idea for a problem I was facing](https://stackoverflow.com/questions/29370377/scrolling-in-c-with-ncurses-pad)
- [This repositry is what makes me able to get the CPU usage of the processes in Windows with only minor changes](https://github.com/vikyd/cpu_usage)

## Building the Project

Follow these steps to build the project using Microsoft Visual C++:

1. Open the project in Microsoft Visual Studio.
2. Build the project.

**Note: A Cmake implementation is going to be added in the future.**

## Future plans

- Add more process specific info.
- Implement the processes information to the session recording.
- Add operating system information.
- Make application cross-platform.
- Configure Cmake.

## Usage

1. Build the project using Microsoft Visual C++.
2. Run the executable to collect and browse information about the host machine.
3. Follow the on-screen guide menu to fill your needs.

## Issues

If you come across a bug or have any issues overall with the project please head to the [Issues page](https://github.com/Amremad719/System-Info-Browser/issues) or [create an issue](https://github.com/Amremad719/System-Info-Browser/issues/new)
