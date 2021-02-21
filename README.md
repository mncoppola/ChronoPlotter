# ChronoPlotter

Easily generate graphs using recorded data from your chronograph. Supports LabRadar and MagnetoSpeed with SD card. Create customizable graphs and charts comparing charge weight and velocity without manually entering your chronograph data.

Table of contents:
* [Quick Start](#quick-start)
* [Running ChronoPlotter with Python](#running-chronoplotter-with-python)
* [Notes About PyInstaller](#notes-about-pyinstaller)

## Quick Start

If you're looking to quickly get started with ChronoPlotter, download and run the prebuilt program:

* **Windows**: [ChronoPlotter.exe download](https://github.com/mncoppola/ChronoPlotter/releases/latest/download/ChronoPlotter.exe)
* **MacOS**: [ChronoPlotter.dmg download](https://github.com/mncoppola/ChronoPlotter/releases/latest/download/ChronoPlotter.dmg)
* **Linux**: [ChronoPlotter download](https://github.com/mncoppola/ChronoPlotter/releases/latest/download/ChronoPlotter) (Ubuntu 18.04+)

Nerds and power users should reference the [Running ChronoPlotter with Python](#running-chronoplotter-with-python) section.

**Note: The program may take a few moments to start.** The program's main screen will then show:

![Home screen](https://github.com/mncoppola/ChronoPlotter/blob/main/images/1.png?raw=true)

Make sure the SD card from your LabRadar or MagnetoSpeed is plugged into your computer. Then click `Select directory`, navigate to your SD card, and click `Select Folder` to confirm. The following example shows a LabRadar SD card:

![Directory selection](https://github.com/mncoppola/ChronoPlotter/blob/main/images/2.png?raw=true)

The program should now be populated with all of your recorded data:

![Populated series data](https://github.com/mncoppola/ChronoPlotter/blob/main/images/3.png?raw=true)

Uncheck any series you don't want to include in the graph. Then enter a charge weight for each series, or click the `Auto-fill charge weights` button to automate the process:

![Auto-fill charge weights](https://github.com/mncoppola/ChronoPlotter/blob/main/images/4.png?raw=true)

To include details in the graph about the components used in testing, additionally fill in the text fields on the right:

![Full details filled out](https://github.com/mncoppola/ChronoPlotter/blob/main/images/5.png?raw=true)

Clicking `Show graph` pops open a new window with a preview of the generated graph. **Note: This does not save your graph, only displays it.**

The graph will look something like this:

![Scatter plot](https://github.com/mncoppola/ChronoPlotter/blob/main/images/scatter.png?raw=true)

You can alternatively create a line chart with standard deviation error bars by selecting it from the `Graph type` drop-down.

![Line chart + SD](https://github.com/mncoppola/ChronoPlotter/blob/main/images/line.png?raw=true)

By default, ChronoPlotter includes ES (Extreme Spread), SD (Standard Deviation), and the series-to-series difference in velocity for each series in the graph. The average (mean) velocity for each series can also be included but is unchecked by default. You can add, remove, or change the location of these details in the graph under the `Graph options` section.

Close the preview window, making sure not to close ChronoPlotter itself. Finally, click `Save graph as image` to save your graph as a .PNG, .SVG, or .PDF file.

## Running ChronoPlotter with Python

Users can directly run `ChronoPlotter.py` as a Python script instead of using the prebuilt binaries. Python 3.5+ is required (for PyQt5)

Install the following dependencies:
```
pip3 install --upgrade pip
pip3 install numpy matplotlib PyQt5
```

Then run:
```
python3 ChronoPlotter.py
```

## Notes About PyInstaller

ChronoPlotter uses PyInstaller to generate single file executables for ease of distribution. It does not come without certain trade-offs, though. PyInstaller works by bundling an entire base Python installation + additional modules in a single file, and unpacking it to a temporary directory every time the executable is run. This results in large executables and slow program start time (up to 15 seconds on slow disks!)

A lot of time has been spent trying to optimize the ChronoPlotter executables. PyInstaller was chosen for its multi-platform support, ability to create single file executables, and current support by project maintainers. It also features a "hooks" subsystem that optimizes out unused portions of common Python modules. Other Python "freezing" solutions such as py2exe and cx_Freeze, as well as Python compilers such as Nuitka, were also explored but either ran into errors or produced even larger files than PyInstaller.

Focus was instead placed on optimizing the PyInstaller-created executables themselves. ChronoPlotter uses matplotlib and PyQt5 which are both notoriously massive, complex dependencies that are difficult to efficiently freeze. By default, PyInstaller bundles all matplotlib backend modules it can find, however modifying PyInstaller to include a hardcoded whitelist provided only marginal improvement. Configuring PyInstaller to pack files with UPX provided the greatest file size savings (almost 10mb smaller) but increased the executable start time by upwards of 20-40% which became unacceptable on slow disks.

At the end of the day, ChronoPlotter ships standard PyInstaller single file executables. The correct solution is to rewrite the entire project in C++ and swap out matplotlib for a different native graphing solution. That's going to be a whole thing though, and there's something nice about the versatility and human-readability of Python. If you have any suggestions to improve ChronoPlotter or the way it's distributed, feel free to open an issue on the tracker.
