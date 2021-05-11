# ChronoPlotter

![ChronoPlotter logo](https://github.com/mncoppola/ChronoPlotter/blob/main/images/logo.png?raw=true)

Easily generate graphs using recorded data from your chronograph. Supports LabRadar and MagnetoSpeed with SD card. Create customizable graphs and charts comparing charge weight and velocity without manually entering your chronograph data. Additionally supports creating graphs for seating depth testing.

Table of contents:
* [Quick Start](#quick-start)
* [Round-Robin (OCW) and Satterlee Testing](#round-robin-ocw-and-satterlee-testing)
* [Seating Depth Testing](#seating-depth-testing)
* [Building ChronoPlotter from Source](#building-chronoplotter-from-source)

## Quick Start

First, download and run ChronoPlotter for your operating system:

* **Windows**: [ChronoPlotter.exe download](https://github.com/mncoppola/ChronoPlotter/releases/latest/download/ChronoPlotter.exe)
* **MacOS**: [ChronoPlotter.dmg download](https://github.com/mncoppola/ChronoPlotter/releases/latest/download/ChronoPlotter.dmg)

If your system is not listed above, check out the [Building ChronoPlotter from Source](#building-chronoplotter-from-source) section.

The program's main screen will show:

![Home screen](https://github.com/mncoppola/ChronoPlotter/blob/main/images/1.png?raw=true)

Make sure the SD card from your LabRadar or MagnetoSpeed is plugged into your computer.

* For **LabRadar**: Click the `Select LabRadar directory` button, navigate to your SD card, and click `Select Folder` to confirm.
* For **MagnetoSpeed**: Click the `Select MangetoSpeed file` button, navigate to your SD card, and select the `LOG.CSV` file.

The following example shows a LabRadar SD card:

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

By default, ChronoPlotter displays ES (Extreme Spread), SD (Standard Deviation), and the series-to-series difference in velocity for each series in the graph. The average (mean) velocity for each series can also be displayed but is unchecked by default. You can add, remove, or change the location of these details in the graph under the `Graph options` section.

Close the preview window, making sure not to close ChronoPlotter itself. Finally, click `Save graph as image` to save your graph as a .PNG, .JPG, or .PDF file.

## Round-Robin (OCW) and Satterlee Testing

ChronoPlotter supports chronograph data recorded using the "round-robin" method popular with [OCW testing](http://www.ocwreloading.com/). First, uncheck any series not part of the round-robin:

![Populated OCW series data](https://github.com/mncoppola/ChronoPlotter/blob/main/images/ocw_1.png?raw=true)

Click `Convert from round-robin` to open this dialog:

![Convert OCW series dialog](https://github.com/mncoppola/ChronoPlotter/blob/main/images/ocw_2.png?raw=true)

Then click `OK` to convert the series data into traditional series suitable for graphing. Note that ChronoPlotter does *not* alter your CSV files, this only converts the data loaded in the program:

![Converted OCW series data](https://github.com/mncoppola/ChronoPlotter/blob/main/images/ocw_3.png?raw=true)

Likewise, this same feature can be used to convert data recorded using the [Satterlee method](http://www.65guys.com/10-round-load-development-ladder-test/). This is because at its core Satterlee is simply a "one shot round-robin" test. To do so, uncheck all other series except for the one series containing Satterlee data, then click `Convert from round-robin` and proceed as usual.

Thanks to [TallMikeSTL](https://www.reddit.com/user/TallMikeSTL) for contributing test data.

## Seating Depth Testing

In addition to powder charge graphs, ChronoPlotter also supports creating graphs comparing bullet seating depth and group size. This data is not recorded on the chronograph SD card and must be inputted by the shooter.

Begin by clicking the `Seating depth` tab at top:

![Seating depth screen](https://github.com/mncoppola/ChronoPlotter/blob/main/images/seating_1.png?raw=true)

By default, a single empty series is created. Click the `Add new group` button to create additional series for each depth being tested. Then enter a cartridge length for each series, or click the `Auto-fill cartridge lengths` button to automate the process:

![Auto-fill cartridge lengths](https://github.com/mncoppola/ChronoPlotter/blob/main/images/seating_2.png?raw=true)

Then proceed to enter the group size for each series. **NOTE:** ChronoPlotter defaults to CBTO (Cartridge Base to Ogive) for cartridge length and ES (Extreme Spread) for group size measurements. Other measurements may instead be selected under the `Graph options` section on the right, if desired.

To include details in the graph about the components used in testing, additionally fill in the text fields on the right:

![Full seating depth details filled out](https://github.com/mncoppola/ChronoPlotter/blob/main/images/seating_3.png?raw=true)

Clicking `Show graph` pops open a new window with a preview of the generated graph. **Note: This does not save your graph, only displays it.**

The graph will look something like this:

![Seating depth graph](https://github.com/mncoppola/ChronoPlotter/blob/main/images/seating_graph.png?raw=true)

By default, ChronoPlotter displays the group size for each series in the graph. The series-to-series difference in group size can also be displayed but is unchecked by default. You can add, remove, or change the location of these details in the graph under the `Graph options` section.

Close the preview window, making sure not to close ChronoPlotter itself. Finally, click `Save graph as image` to save your graph as a .PNG, .JPG, or .PDF file.

## Building ChronoPlotter from Source

For most general purposes, use the prebuilt executable files available for download at the top of the [quick start](#quick-start) section.

If your operating system is not listed, or if you'd like to make changes to the program, ChronoPlotter can alternatively be built from source. The project is written using the [Qt](https://www.qt.io/) framework. It targets Qt version 5.12.2, but other Qt versions may potentially work as well.

### Linux

To build for Linux, run the following commands:

```
$ qmake -config release
$ make
```

The binary `ChronoPlotter` will then be created.

### MacOS

To build for MacOS, run the following commands:

```
$ qmake -config release
$ make
```

The app directory `ChronoPlotter.app/` will then be created.

### Windows

To build for Windows, run the following commands:

```
> qmake -config release
> mingw32-make
```

The binary `release\ChronoPlotter.exe` will then be created.

At this time, only MingGW has been tested for building on Windows. It may be possible to use MSVC.
