# ChronoPlotter

Easily generate graphs using recorded data from your chronograph. Supports LabRadar and MagnetoSpeed with SD card. Create customizable graphs and charts comparing charge weight and velocity without manually entering your chronograph data.

Table of contents:
* [Quick Start](#quick-start)
* [Running ChronoPlotter with Python](#running-chronoplotter-with-python)

## Quick Start

If you're looking to quickly get started with ChronoPlotter, download and run the prebuilt program:

* **Windows**: [ChronoPlotter.exe download](https://github.com/mncoppola/ChronoPlotter/releases/latest/download/ChronoPlotter-Windows.exe)
* **MacOS**: Coming soon
* **Linux**: [ChronoPlotter download](https://github.com/mncoppola/ChronoPlotter/releases/latest/download/ChronoPlotter-Linux) (Ubuntu 18.04+)

Nerds and power users should reference the [Running ChronoPlotter with Python](#running-chronoplotter-with-python) section.

The prebuilt program may take a few moments to start. The program's main screen will then show:

![Home screen](https://github.com/mncoppola/ChronoPlotter2/blob/main/images/1.png?raw=true)

Make sure the SD card from your LabRadar or MagnetoSpeed is plugged into your computer. Then click `Select directory`, navigate to your SD card, and click `Select Folder` to confirm. The following example shows a LabRadar SD card:

![Directory selection](https://github.com/mncoppola/ChronoPlotter2/blob/main/images/2.png?raw=true)

The program should now be populated with all of your recorded data:

![Populated series data](https://github.com/mncoppola/ChronoPlotter2/blob/main/images/3.png?raw=true)

Enter the charge weights for each series you want to include in the graph. Make sure to uncheck any series you don't want to include. To include details in the graph about the load being tested, additionally fill in the text fields on the right:

![Full details filled out](https://github.com/mncoppola/ChronoPlotter2/blob/main/images/4.png?raw=true)

Clicking `Show graph` pops open a new window with a preview of the generated graph. **Note: This does not save your graph, only displays it.**

The graph will look something like this:

![Scatter plot](https://github.com/mncoppola/ChronoPlotter2/blob/main/images/scatter.png?raw=true)

You can also create a line chart with standard deviation error bars by selecting it from the `Graph type` drop-down.

![Line chart + SD](https://github.com/mncoppola/ChronoPlotter2/blob/main/images/line.png?raw=true)

By default, ChronoPlotter includes ES (Extreme Spread), SD (standard deviation), and the series-to-series difference in velocity for each series in the graph. You can remove these from the graph by unchecking the `Show ES + SD above shot strings` and `Show velocity deltas below shot strings` options.

Close the preview window, making sure not to close ChronoPlotter itself. Finally, click `Save graph as image` to save your graph as a .PNG, .SVG, or .PDF file.

## Running ChronoPlotter with Python

Users can directly run `ChronoPlotter.py` as a Python script instead of using the prebuilt binaries. These binaries make portability and distribution easier, but are built with PyInstaller which is not the most efficient with file size or program startup time.

Python 3.5+ is required (for PyQt5)

Install the following dependencies:
```
pip3 install --upgrade install pip
pip3 install numpy matplotlib PyQt5
```

Then run:
```
python3 ChronoPlotter.py
```
