import argparse
import csv
import numpy
import os
import re
import sys
import matplotlib
import matplotlib.patches as patches
import matplotlib.pyplot as plt
import matplotlib.transforms as transforms
from pathlib import Path
from PyQt5.QtCore import Qt
from PyQt5.QtSvg import QSvgWidget
from PyQt5.QtWidgets import QWidget, QApplication, QPushButton, QLabel, QFileDialog, QGridLayout, QCheckBox, QHBoxLayout, QVBoxLayout, QScrollArea, QFormLayout, QGroupBox, QComboBox, QLineEdit, QDoubleSpinBox, QStackedWidget, QMessageBox, QDialog, QDialogButtonBox

if sys.version_info < (3, 5):
	print("Python 3.5+ is required!")
	sys.exit(-1)

# Extracts data for a single series from a LabRadar CSV file
def extract_labradar_series_data(csvfile):
	csv_data = {"m_velocs": []}

	for idx, row in enumerate(csvfile):
		if idx == 4:
			csv_data["total_shots"] = int(row[1])
		elif idx == 6:
			csv_data["v_units"] = row[1]
		elif idx > 17:
			if idx == 18:
				csv_data["first_date"] = row[15]
				csv_data["first_time"] = row[16]
			csv_data["m_velocs"].append(int(row[1]))

	csv_data["v_lowest"] = min(csv_data["m_velocs"])
	csv_data["v_highest"] = max(csv_data["m_velocs"])

	return csv_data

# Maybe make these two funcs return a list of series datas, then abstract out the series tuple creation

# Extracts data for all single series from a MagnetoSpeed CSV file
# Each series has a summary in the first six rows, followed by one row for each shot
# Series are separated by four columns of ---- and immediately follow the previous one
def extract_magnetospeed_series_data(csvfile):
	csv_datas = []

	cur = {"m_velocs": [], "first_date": "", "first_time": ""}

	for idx, row in enumerate(csvfile):
		# They just love their whitespace
		for i in range(len(row)):
			row[i] = row[i].strip()

		if len(row) > 0:
			if row[0] == "Series" and row[2] == "Shots:":
				cur["series_num"] = int(row[1])
				cur["total_shots"] = int(row[3])
			elif str(row[0]).isdigit():
				print("Adding velocity %d" % int(row[2]))
				cur["m_velocs"].append(int(row[2]))
				if len(cur["m_velocs"]) == 1:
					cur["v_units"] = row[3]
			elif row[0] == "----":
				cur["v_lowest"] = min(cur["m_velocs"])
				cur["v_highest"] = max(cur["m_velocs"])
				csv_datas.append(cur)
				cur = {"m_velocs": [], "first_date": "", "first_time": ""}

	return csv_datas

class ChronoPlotter(QWidget):

	def __init__(self):
		super().__init__()

		# graph types
		self.SCATTER = 0
		self.LINE_SD = 1

		# weight units
		self.GRAIN = 0
		self.GRAM = 1

		# velocity units
		self.FPS = 0
		self.MPS = 1

		self.series = []
		self.scroll_area = None

		self.initUI()

	def initUI(self):

		# Left-hand panel of user-populated series data
		self.label = QLabel("Select LabRadar or MagnetoSpeed directory\nto populate series data\n")
		self.label.setAlignment(Qt.AlignCenter)

		self.dir_btn = QPushButton("Select directory", self)
		self.dir_btn.clicked.connect(self.dirDialog)
		self.dir_btn.setMinimumWidth(300)
		self.dir_btn.setMaximumWidth(300)
		self.dir_btn.setMinimumHeight(50)
		self.dir_btn.setMaximumHeight(50)

		self.placeholder_vbox = QVBoxLayout()
		self.placeholder_vbox.addStretch(0)
		self.placeholder_vbox.addWidget(self.label)
		self.placeholder_vbox.setAlignment(self.label, Qt.AlignCenter)
		self.placeholder_vbox.addWidget(self.dir_btn)
		self.placeholder_vbox.setAlignment(self.dir_btn, Qt.AlignCenter)
		self.placeholder_vbox.addStretch(0)

		self.placeholder_vbox_widget = QWidget()
		self.placeholder_vbox_widget.setLayout(self.placeholder_vbox)

		self.stacked_widget = QStackedWidget()
		self.stacked_widget.addWidget(self.placeholder_vbox_widget)
		self.stacked_widget.setCurrentIndex(0)

		scroll_vbox = QVBoxLayout()
		scroll_vbox.addWidget(self.stacked_widget)
		groupBox = QGroupBox("Chronograph data to include:")
		groupBox.setLayout(scroll_vbox)

		# Right-hand panel of graph options
		self.options_layout = QVBoxLayout()

		# Form class
		form_layout = QFormLayout()
		self.graph_title = QLineEdit()
		form_layout.addRow(QLabel("Graph title:"), self.graph_title)
		self.rifle = QLineEdit()
		form_layout.addRow(QLabel("Rifle:"), self.rifle)
		self.projectile = QLineEdit()
		form_layout.addRow(QLabel("Projectile:"), self.projectile)
		self.propellant = QLineEdit()
		form_layout.addRow(QLabel("Propellant:"), self.propellant)
		self.brass = QLineEdit()
		form_layout.addRow(QLabel("Brass:"), self.brass)
		self.primer = QLineEdit()
		form_layout.addRow(QLabel("Primer:"), self.primer)

		self.graph_type = QComboBox()
		self.graph_type.addItem("Scatter plot")
		self.graph_type.addItem("Line chart + SD bars")
		form_layout.addRow(QLabel("Graph type:"), self.graph_type)

		self.weight_units = QComboBox()
		self.weight_units.addItem("grain (gr)")
		self.weight_units.addItem("gram (g)")
		form_layout.addRow(QLabel("Weight units:"), self.weight_units)

		self.v_units = QComboBox()
		self.v_units.addItem("feet per second (fps)")
		self.v_units.addItem("meters per second (m/s)")
		form_layout.addRow(QLabel("Velocity units:"), self.v_units)

		self.options_layout.addLayout(form_layout)

		# Show ES+SD text checkbox
		essd_layout = QHBoxLayout()
		self.essd_checkbox = QCheckBox()
		self.essd_checkbox.setChecked(True)
		# QCheckBoxes are shorter than QLineEdit and QComboBoxes. Set it manually to match the other rows.
		self.essd_checkbox.setFixedHeight(self.graph_title.sizeHint().height())
		essd_layout.addWidget(self.essd_checkbox, 0)
		essd_layout.addWidget(QLabel("Show ES + SD above shot strings"), 1)
		self.options_layout.addLayout(essd_layout)

		# Show velocity data checkbox
		vd_layout = QHBoxLayout()
		self.vd_checkbox = QCheckBox()
		self.vd_checkbox.setChecked(True)
		self.vd_checkbox.setFixedHeight(self.graph_title.sizeHint().height())
		vd_layout.addWidget(self.vd_checkbox, 0)
		vd_layout.addWidget(QLabel("Show velocity deltas below shot strings"), 1)
		self.options_layout.addLayout(vd_layout)

		self.create_graph_btn = QPushButton("Show graph", self)
		self.create_graph_btn.clicked.connect(self.showGraph)
		self.options_layout.addWidget(self.create_graph_btn)

		self.save_graph_btn = QPushButton("Save graph as image", self)
		self.save_graph_btn.clicked.connect(self.saveGraph)
		self.options_layout.addWidget(self.save_graph_btn)

		# Don't resize row heights if window height changes
		self.options_layout.addStretch(0)

		groupBox_options = QGroupBox("Graph options:")
		groupBox_options.setLayout(self.options_layout)

		# Layout to horizontally position series data and options
		bottom_layout = QHBoxLayout()
		bottom_layout.addWidget(groupBox, stretch=2)
		bottom_layout.addWidget(groupBox_options, stretch=1)

		# Create a hidden file dialog button in the top-left. It'll be revealed once the left panel
		# is filled with chrono series data.
		self.dir_btn2 = QPushButton("Select directory", self)
		self.dir_btn2.clicked.connect(self.dirDialog)
		self.dir_btn2.setMinimumWidth(300)
		self.dir_btn2.setMaximumWidth(300)
		self.dir_btn2.hide()

		# About link
		self.about = QPushButton("About this app", self)
		self.about.clicked.connect(self.showAbout)

		top_layout = QHBoxLayout()
		top_layout.addWidget(self.dir_btn2)
		top_layout.addStretch()
		top_layout.addWidget(self.about)

		# Layout to vertically position file dialog button and panels
		layout_vert = QVBoxLayout(self)
		layout_vert.addLayout(top_layout)
		layout_vert.addLayout(bottom_layout)

		self.setGeometry(300, 300, 1000, layout_vert.sizeHint().height())
		self.setWindowTitle("ChronoPlotter")
		self.show()

	def dirDialog(self):
		path = QFileDialog.getExistingDirectory(None, "Select directory")
		print("Selected directory: %s" % path)

		if path == "":
			print("User didn't select a directory, bail")
			return

		self.series = []

		# We don't know if we're looking at a LabRadar or MagnetoSpeed yet
		# LabRadar has a LBR/ directory in the root of its drive filled with SR####/ directories
		# MagnetoSpeed has a single LOG.CSV file in the root of its drive

		lr_data_found = False

		# We have a couple extra checks for LabRadar data. We handle the cases where the user selected the root of the
		# SD card (and the series directories are actually in LBR/), and if we're inside one of the actual series
		# directories and we need to be one directory up to enumerate all of them.

		lbr_path = os.path.join(path, "LBR")
		if os.path.exists(lbr_path) and os.path.isdir(lbr_path):
			path = lbr_path
			lr_data_found = True
			print("Detected LabRadar directory '%s'. Using that directory instead." % lbr_path)

			msg = QMessageBox()
			msg.setIcon(QMessageBox.Information)
			msg.setText("Detected LabRadar data\n\nUsing '%s'" % path)
			msg.setWindowTitle("Success")
			msg.exec_()

		trk_path = os.path.join(path, "TRK")
		if os.path.exists(trk_path) and os.path.isdir(trk_path):
			path = os.path.abspath(os.path.join(path, ".."))
			lr_data_found = True
			print("Detected LabRadar directory '%s'. Using one directory level up instead." % trk_path)

			msg = QMessageBox()
			msg.setIcon(QMessageBox.Information)
			msg.setText("Detected LabRadar data\n\nUsing '%s'" % path)
			msg.setWindowTitle("Success")
			msg.exec_()

		# Regex for LabRadar series directory
		pattern = re.compile("^SR\d\d\d\d")

		for fname in os.listdir(path):
			#print(fname)

			fpath = os.path.join(path, fname)

			if fname == "LOG.CSV":
				csv_path = fpath
				print("Detected MagnetoSpeed file '%s'" % csv_path)

				msg = QMessageBox()
				msg.setIcon(QMessageBox.Information)
				msg.setText("Detected MagnetoSpeed data\n\nUsing '%s'" % path)
				msg.setWindowTitle("Success")
				msg.exec_()

				try:
					f = open(csv_path)
				except:
					print("%s does not exist" % csv_path)
					continue

				csvfile = csv.reader(f, delimiter=',')
				csv_datas = extract_magnetospeed_series_data(csvfile)

				for csv_data in csv_datas:
					checkbox = QCheckBox()
					checkbox.setChecked(True)

					charge_weight = QDoubleSpinBox()
					charge_weight.setDecimals(1)
					charge_weight.setSingleStep(0.1)
					charge_weight.setMinimumWidth(100)
					charge_weight.setMaximumWidth(100)

					self.series.append((csv_data["series_num"], "Series %d" % csv_data["series_num"], csv_data, charge_weight, checkbox))
				f.close()

			elif os.path.isdir(fpath) and pattern.match(fname):
				print("Detected LabRadar series directory '%s'" % fpath)

				if not lr_data_found:
					msg = QMessageBox()
					msg.setIcon(QMessageBox.Information)
					msg.setText("Detected LabRadar data\n\nUsing '%s'" % path)
					msg.setWindowTitle("Success")
					msg.exec_()

					lr_data_found = True

				csv_path = os.path.join(fpath, "%s Report.csv" % fname)
				try:
					f = open(csv_path)
				except:
					print("%s does not exist" % csv_path)
					continue

				series_num = int(fname[2:])
				csvfile = csv.reader((x.replace('\0', '') for x in f), delimiter=';')
				csv_data = extract_labradar_series_data(csvfile)

				checkbox = QCheckBox()
				checkbox.setChecked(True)

				charge_weight = QDoubleSpinBox()
				charge_weight.setDecimals(1)
				charge_weight.setSingleStep(0.1)
				charge_weight.setMinimumWidth(100)
				charge_weight.setMaximumWidth(100)

				self.series.append((series_num, fname, csv_data, charge_weight, checkbox))
				f.close()

		# Sort the list by series number
		self.series = sorted(self.series, key=lambda x: x[0])

		# Only continue if chrono data was found
		if len(self.series) == 0:
			print("Didn't find any chrono data in this directory, bail")

			msg = QMessageBox()
			msg.setIcon(QMessageBox.Critical)
			msg.setText("Unable to find chronograph data in '%s'" % path)
			msg.setWindowTitle("Error")
			msg.exec_()
			return

		# If we already have series data displayed, clear it out first
		if self.scroll_area:
			self.stacked_widget.removeWidget(self.scroll_area)

		# Wrap grid in a widget to make the grid scrollable
		scroll_area_widget = QWidget()

		self.series_grid = QGridLayout(scroll_area_widget)
		self.series_grid.setColumnStretch(0, 0)
		self.series_grid.setColumnStretch(1, 1)
		self.series_grid.setColumnStretch(2, 2)
		self.series_grid.setColumnStretch(3, 3)
		self.series_grid.setColumnStretch(4, 3)
		self.series_grid.setHorizontalSpacing(25)

		self.scroll_area = QScrollArea()
		self.scroll_area.setWidget(scroll_area_widget)
		self.scroll_area.setWidgetResizable(True)

		self.stacked_widget.addWidget(self.scroll_area)
		self.stacked_widget.setCurrentWidget(self.scroll_area)

		# Now that we've hidden the placeholder text + button, reveal the file dialog button in the top-left
		self.dir_btn2.show()

		# Headers for series data
		name_header = QLabel("Series Name")
		#name_header.setStyleSheet("text-decoration: underline")
		#name_header.setStyleSheet("font-weight: bold")
		self.series_grid.addWidget(name_header, 0, 1)
		cw_header = QLabel("Charge Weight")
		#cw_header.setStyleSheet("text-decoration: underline")
		#cw_header.setStyleSheet("font-weight: bold")
		self.series_grid.addWidget(cw_header, 0, 2)
		result_header = QLabel("Series Result")
		#result_header.setStyleSheet("text-decoration: underline")
		#result_header.setStyleSheet("font-weight: bold")
		self.series_grid.addWidget(result_header, 0, 3)
		date_header = QLabel("Series Date")
		#date_header.setStyleSheet("text-decoration: underline")
		#date_header.setStyleSheet("font-weight: bold")
		self.series_grid.addWidget(date_header, 0, 4)

		for i, v in enumerate(self.series):
			series_name = v[1]
			csv_data = v[2]
			charge_weight = v[3]
			checkbox = v[4]

			checkbox.stateChanged.connect(self.seriesCheckBoxChangedCb(i + 1))

			self.series_grid.addWidget(checkbox, i + 1, 0)
			self.series_grid.addWidget(QLabel(series_name), i + 1, 1)

			charge_weight_layout = QHBoxLayout()
			charge_weight_layout.addWidget(charge_weight)
			charge_weight_layout.addStretch(0)
			self.series_grid.addLayout(charge_weight_layout, i + 1, 2)

			v_label = QLabel("%d shots, %d-%d %s" % (csv_data["total_shots"], csv_data["v_lowest"], csv_data["v_highest"], csv_data["v_units"]))
			self.series_grid.addWidget(v_label, i + 1, 3)
			datetime_label = QLabel("%s %s" % (csv_data["first_date"], csv_data["first_time"]))
			self.series_grid.addWidget(datetime_label, i + 1, 4)

		self.series_grid.setRowStretch(len(self.series) + 1, 1)

		self.scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
		self.scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
		self.scroll_area.setWidgetResizable(True)

	# Lambdas in loops are hard
	def seriesCheckBoxChangedCb(self, idx):
		return lambda: self.seriesCheckBoxChanged(idx)

	def seriesCheckBoxChanged(self, idx):
		checkbox = self.series_grid.itemAtPosition(idx, 0).widget()
		series_name = self.series_grid.itemAtPosition(idx, 1).widget()
		charge_weight = self.series_grid.itemAtPosition(idx, 2).layout().itemAt(0).widget()
		series_result = self.series_grid.itemAtPosition(idx, 3).widget()
		series_date = self.series_grid.itemAtPosition(idx, 4).widget()

		if checkbox.isChecked():
			action = "checked"
			series_name.setStyleSheet("")
			charge_weight.setEnabled(True)
			series_result.setStyleSheet("")
			series_date.setStyleSheet("")
		else:
			action = "unchecked"
			series_name.setStyleSheet("color: #878787")
			charge_weight.setEnabled(False)
			series_result.setStyleSheet("color: #878787")
			series_date.setStyleSheet("color: #878787")

		print("Grid row %d was %s" % (idx, action))


	def showGraph(self, save_without_showing=False):
		print("showGraph clicked!")

		num_enabled = 0

		for val in self.series:
			series_num = val[0]
			series_name = val[1]
			csv_data = val[2]
			charge = val[3].value()
			checkbox = val[4]
			if checkbox.isChecked():
				num_enabled += 1
				if charge == 0:
					msg = QMessageBox()
					msg.setIcon(QMessageBox.Critical)
					msg.setText("'%s' is missing charge weight!" % series_name)
					msg.setWindowTitle("Error")
					msg.exec_()
					return

		if num_enabled < 2:
			print("Only %d series enabled, bailing", num_enabled)

			msg = QMessageBox()
			msg.setIcon(QMessageBox.Critical)
			msg.setText("At least two series are required to graph!")
			msg.setWindowTitle("Error")
			msg.exec_()
			return

		plt.style.use("seaborn-whitegrid")
		matplotlib.rcParams['font.family'] = "DejaVu Sans"
		figure, ax = plt.subplots(1, figsize=(20,8))

		xticks = []
		averages = []

		last_average = None

		# Create a copy of the series data we'll actually use to draw the graph. We need to sort the data
		# by chage weight so it draws correctly if the user-inputted charge weights aren't in order.
		sorted_series = sorted(self.series.copy(), key=lambda x: x[3].value())

		for i, val in enumerate(sorted_series):
			series_num = val[0]
			series_name = val[1]
			csv_data = val[2]
			charge = val[3].value()
			checkbox = val[4]

			if not checkbox.isChecked():
				print("Series %s is unchecked, skipping..." % series_name)
				continue

			print("Series %04d (%.1fgr)" % (series_num, charge))

			m_velocs = csv_data["m_velocs"]
			highest = max(m_velocs)
			lowest = min(m_velocs)
			average = numpy.mean(m_velocs)
			es = highest - lowest
			stdev = numpy.std(m_velocs, ddof=1)
			print("Velocities: %s" % m_velocs)
			print("Average: %.1f" % average)
			print("Highest: %d" % highest)
			print("Lowest: %d" % lowest)
			print("ES: %d" % es)
			print("SD: %.1f" % stdev)
			print("")

			averages.append((charge, average))
			xticks.append(charge)

			if self.graph_type.currentIndex() == self.SCATTER:
				points = []

				for v in m_velocs:
					points.append((charge, v))

				# Draw scatter plot
				scatter_x, scatter_y = list(zip(*points))
				plt.plot(scatter_x, scatter_y, "o", color="#0536b0", markeredgewidth=0, markersize=6)

			else:
				plt.errorbar(charge, average, fmt="o", yerr=stdev, markersize=5, capsize=3, elinewidth=1, ecolor="black", markerfacecolor="#0536b0", markeredgecolor="#0536b0")

		# Now that we've plotted all the points, we can draw the bounding boxes around each string of shots (if we're doing a scatter plot)
		# We couldn't do it above since each loop iteration adds another string of shots which alters the pixel positions of the previous ones

		last_average = None

		for i, val in enumerate(sorted_series):
			series_num = val[0]
			series_name = val[1]
			csv_data = val[2]
			charge = val[3].value()
			checkbox = val[4]

			if not checkbox.isChecked():
				print("Series %s is unchecked, skipping..." % series_name)
				continue

			print("Series %04d (%.1fgr)" % (series_num, charge))

			m_velocs = csv_data["m_velocs"]
			highest = max(m_velocs)
			lowest = min(m_velocs)
			average = numpy.mean(m_velocs)
			es = highest - lowest
			stdev = numpy.std(m_velocs, ddof=1)

			points = []

			for v in m_velocs:
				points.append((charge, v))

			# Obtain display coordinates for points. We need to convert points for both highest/lowest shots and stdev
			b_l_scatter_pix = ax.transData.transform_point((charge, lowest))
			t_r_scatter_pix = ax.transData.transform_point((charge, highest))

			b_l_stdev_pix = ax.transData.transform_point((charge, average - stdev))
			t_r_stdev_pix = ax.transData.transform_point((charge, average + stdev))

			print("bottom left pix: ", b_l_scatter_pix)
			print("top right pix: ", t_r_scatter_pix)

			inv = ax.transData.inverted()
			left, bottom = inv.transform((b_l_scatter_pix[0] - 10, b_l_scatter_pix[1] - 10))
			right, top = inv.transform((t_r_scatter_pix[0] + 10, t_r_scatter_pix[1] + 10))

			center, top_scatter_label = inv.transform((t_r_scatter_pix[0], t_r_scatter_pix[1] + 25))
			center, bottom_scatter_label = inv.transform((b_l_scatter_pix[0], b_l_scatter_pix[1] - 20))

			center, top_stdev_label = inv.transform((t_r_stdev_pix[0], t_r_stdev_pix[1] + 20))
			center, bottom_stdev_label = inv.transform((b_l_stdev_pix[0], b_l_stdev_pix[1] - 15))

			if self.graph_type.currentIndex() == self.SCATTER:
				top_label = top_scatter_label
				bottom_label = bottom_scatter_label

				print("Placing scatter group bounding box")
				ax.add_patch(patches.Rectangle(xy=(left, bottom), width=(right - left), height=(top - bottom), fill=False, color="#0536b0", linewidth=1.1))
			else:
				top_label = top_stdev_label
				bottom_label = bottom_stdev_label

			if self.essd_checkbox.isChecked():
				print("Placing ES+SD annotation")
				plt.annotate("ES: %d\nSD: %.1f" % (es, stdev), xy=(charge, top_label), ha="center", va="baseline", bbox=dict(boxstyle="square", facecolor="white", linewidth=0))

			if self.vd_checkbox.isChecked():
				if last_average != None:
					delta = int(average - last_average)
					if delta < 0:
						sign = "-"
					else:
						sign = "+"

					abs_delta = abs(delta)

					if abs_delta < 25:
						facecolor = "#00e810"
						alpha = (1 - (abs_delta / 25.0)) * 0.6
					else:
						facecolor = "white"
						alpha = 1.0

					print("Placing velocity delta annotation for delta %d" % delta)
					plt.annotate("%s%d" % (sign, abs_delta), xy=(charge, bottom_label), ha="center", va="top", bbox=dict(boxstyle="square", facecolor=facecolor, alpha=alpha, linewidth=0))

				last_average = average

		# Draw average line
		line_x, line_y = list(zip(*averages))

		if self.graph_type.currentIndex() == self.SCATTER:
			plt.plot(line_x, line_y, linewidth=1.5, color="#1c57eb", alpha=0.65)
		else:
			plt.plot(line_x, line_y, "-", linewidth=1.5, color="#1c57eb")
			ax.xaxis.grid(False)

		# Titles and axis labels
		graph_title = self.graph_title.text()
		rifle = self.rifle.text()
		propellant = self.propellant.text()
		projectile = self.projectile.text()
		brass = self.brass.text()
		primer = self.primer.text()

		if self.weight_units.currentIndex() == self.GRAIN:
			weight_units = "gr"
		else:
			weight_units = "g"

		if self.v_units.currentIndex() == self.FPS:
			v_units = "fps"
		else:
			v_units = "m/s"

		plt.subplots_adjust(top=0.8)
		plt.title("%s\n\n" % graph_title, fontsize=28, color="#4d4d4d")
		mid = (figure.subplotpars.right + figure.subplotpars.left) / 2
		plt.suptitle(", ".join(filter(None, [rifle, propellant, projectile, brass, primer])), fontsize=14, color="#4d4d4d", y=0.88, x=mid)
		plt.xlabel("Powder Charge (%s)" % weight_units, fontsize=14, labelpad=15, color="#4d4d4d")
		plt.ylabel("Velocity (%s)" % v_units, fontsize=14, labelpad=15, color="#4d4d4d")

		plt.xticks(xticks)

		if save_without_showing:
			filters = "PNG image (*.png);;SVG image (*.svg);;PDF file (*.pdf)"
			path = QFileDialog().getSaveFileName(self, "Save graph as image", "graph.png", filters)[0]
			print("User selected save path '%s'" % path)

			# User canceled file dialog
			if path == "":
				print("No path selected, bailing")

				# Close the figure and window
				ax.cla()
				figure.clf()
				plt.close("all")
				return

			p = Path(path)
			# If user specified no file extension, or an invalid file extension was given, append .png and call it a day
			if (p.suffix == "") or (p.suffix.lower() not in [".png", ".svg", ".pdf"]):
				path = "%s.png" % path

			print("Using save path '%s'" % path)
			figure.savefig(path, dpi=200)

			msg = QMessageBox()
			msg.setIcon(QMessageBox.Information)
			msg.setText("Graph saved to %s" % path)
			msg.setWindowTitle("Success")
			msg.exec_()
		else:
			plt.show()

		# Close the figure and window
		ax.cla()
		figure.clf()
		plt.close("all")

	def saveGraph(self):
		print("saveGraph clicked!")
		self.showGraph(save_without_showing=True)

	def showAbout(self):
		print("showAbout clicked!")

		svg_str = """<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   viewBox="0 0 475 475"
   height="475"
   width="475"
   xml:space="preserve"
   version="1.1"
   id="svg2"><metadata
     id="metadata8"><rdf:RDF><cc:Work
         rdf:about=""><dc:format>image/svg+xml</dc:format><dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" /><dc:title></dc:title></cc:Work></rdf:RDF></metadata><defs
     id="defs6" /><g
     transform="matrix(1.25,0,0,-1.25,0,475)"
     id="g10"><g
       transform="scale(0.1,0.1)"
       id="g12"><path
         id="path14"
         style="fill:#100f0d;fill-opacity:1;fill-rule:nonzero;stroke:none"
         d="m 1950.9,102.539 0,847.461 -40.71,0 0,122.82 c 17.52,4.53 30.53,20.33 30.53,39.27 0,18.95 -13.01,34.73 -30.53,39.27 l 0,124.27 c 17.52,4.53 30.53,20.32 30.53,39.27 0,18.95 -13.01,34.74 -30.53,39.27 l 0,124.28 c 17.52,4.53 30.53,20.31 30.53,39.27 0,18.94 -13.01,34.73 -30.53,39.27 l 0,124.26 c 17.52,4.53 30.53,20.33 30.53,39.27 0,18.94 -13.01,34.74 -30.53,39.27 l 0,130.03 130.02,0 c 4.54,-17.52 20.33,-30.54 39.27,-30.54 18.95,0 34.73,13.02 39.28,30.54 l 124.26,0 c 4.54,-17.52 20.33,-30.54 39.27,-30.54 18.95,0 34.73,13.02 39.28,30.54 l 124.26,0 c 4.54,-17.52 20.33,-30.54 39.27,-30.54 18.95,0 34.74,13.02 39.28,30.54 l 124.26,0 c 4.54,-17.52 20.33,-30.54 39.28,-30.54 18.94,0 34.72,13.02 39.27,30.54 l 122.82,0 0,-40.72 847.45,0 C 3670.97,896.43 2903.57,129.031 1950.9,102.539 Z M 102.535,1849.1 l 847.469,0 0,40.72 122.816,0 c 4.54,-17.52 20.33,-30.54 39.27,-30.54 18.95,0 34.74,13.02 39.28,30.54 l 124.26,0 c 4.54,-17.52 20.33,-30.54 39.28,-30.54 18.95,0 34.74,13.02 39.28,30.54 l 124.26,0 c 4.54,-17.52 20.32,-30.54 39.28,-30.54 18.93,0 34.73,13.02 39.27,30.54 l 124.26,0 c 4.54,-17.52 20.33,-30.54 39.27,-30.54 18.94,0 34.73,13.02 39.27,30.54 l 130.03,0 0,-130.03 c -17.53,-4.55 -30.54,-20.33 -30.54,-39.27 0,-18.94 13.01,-34.72 30.54,-39.27 l 0,-124.26 c -17.53,-4.55 -30.54,-20.33 -30.54,-39.27 0,-18.96 13.01,-34.74 30.54,-39.27 l 0,-124.28 c -17.53,-4.53 -30.54,-20.32 -30.54,-39.27 0,-18.95 13.01,-34.74 30.54,-39.27 l 0,-124.27 c -17.53,-4.54 -30.54,-20.32 -30.54,-39.27 0,-18.94 13.01,-34.73 30.54,-39.27 l 0,-122.82 -40.72,0 0,-847.461 C 896.438,129.02 129.02,896.43 102.535,1849.1 Z m 1746.575,1848.35 0,-847.45 40.72,0 0,-122.82 c -17.53,-4.55 -30.54,-20.33 -30.54,-39.28 0,-18.94 13.01,-34.73 30.54,-39.28 l 0,-124.25 c -17.53,-4.55 -30.54,-20.33 -30.54,-39.28 0,-18.94 13.01,-34.73 30.54,-39.27 l 0,-124.26 c -17.53,-4.55 -30.54,-20.33 -30.54,-39.28 0,-18.94 13.01,-34.73 30.54,-39.27 l 0,-124.26 c -17.53,-4.55 -30.54,-20.33 -30.54,-39.28 0,-18.94 13.01,-34.73 30.54,-39.27 l 0,-130.02 -130.03,0 c -4.54,17.52 -20.33,30.53 -39.27,30.53 -18.94,0 -34.73,-13.01 -39.27,-30.53 l -124.26,0 c -4.54,17.52 -20.34,30.53 -39.27,30.53 -18.96,0 -34.74,-13.01 -39.28,-30.53 l -124.26,0 c -4.54,17.52 -20.33,30.53 -39.28,30.53 -18.95,0 -34.74,-13.01 -39.28,-30.53 l -124.26,0 c -4.54,17.52 -20.33,30.53 -39.28,30.53 -18.94,0 -34.73,-13.01 -39.27,-30.53 l -122.816,0 0,40.71 -847.469,0 c 26.481,952.67 793.899,1720.08 1746.575,1746.56 z m 1848.35,-1746.56 -847.45,0 0,-40.71 -122.82,0 c -4.55,17.52 -20.33,30.53 -39.27,30.53 -18.95,0 -34.74,-13.01 -39.28,-30.53 l -124.26,0 c -4.54,17.52 -20.33,30.53 -39.28,30.53 -18.94,0 -34.73,-13.01 -39.27,-30.53 l -124.26,0 c -4.55,17.52 -20.33,30.53 -39.28,30.53 -18.94,0 -34.73,-13.01 -39.27,-30.53 l -124.26,0 c -4.55,17.52 -20.33,30.53 -39.28,30.53 -18.94,0 -34.73,-13.01 -39.27,-30.53 l -130.02,0 0,130.02 c 17.52,4.54 30.53,20.33 30.53,39.27 0,18.95 -13.01,34.73 -30.53,39.28 l 0,124.26 c 17.52,4.54 30.53,20.33 30.53,39.27 0,18.95 -13.01,34.73 -30.53,39.28 l 0,124.26 c 17.52,4.54 30.53,20.33 30.53,39.27 0,18.95 -13.01,34.73 -30.53,39.28 l 0,124.25 c 17.52,4.55 30.53,20.34 30.53,39.28 0,18.95 -13.01,34.73 -30.53,39.28 l 0,122.82 40.71,0 0,847.45 c 952.67,-26.49 1720.08,-793.9 1746.56,-1746.56 z M 1900.01,3800 C 850.664,3800 0,2949.34 0,1900 0,850.648 850.664,0 1900.01,0 2949.35,0 3800,850.648 3800,1900 c 0,1049.34 -850.65,1900 -1899.99,1900" /></g></g></svg>"""

		about1 = QLabel()
		about1.setTextFormat(Qt.RichText)
		about1.setText("<center><h1>ChronoPlotter v0.0.1</h1>")

		svg_bytes = bytearray(svg_str, encoding="utf-8")
		svg = QSvgWidget()
		svg.renderer().load(svg_bytes)
		svg.setFixedSize(100, 100)

		about2 = QLabel()
		about2.setTextFormat(Qt.RichText)
		about2.setText("""<center>By Michael Coppola<br><a href="https://github.com/mncoppola/ChronoPlotter">github.com/mncoppola/ChronoPlotter</a><br><br>If you found this tool helpful, share some primers with a friend in need.<br><br>Or consider contributing to:<br><a href="https://www.doctorswithoutborders.org/">Doctors Without Borders</a><br><a href="https://www.navysealfoundation.org/">The Navy SEAL Foundation</a><br><a href="https://eji.org/">Equal Justice Initiative</a><br><a href="https://www.mskcc.org/">Memorial Sloan Kettering Cancer Center</a>""")
		about2.setOpenExternalLinks(True)

		msg = QDialog()

		button_box = QDialogButtonBox()
		button_box.addButton("OK", QDialogButtonBox.AcceptRole)
		button_box.accepted.connect(msg.accept)

		about_vbox = QVBoxLayout()
		about_vbox.addWidget(about1)
		about_vbox.addSpacing(10)
		about_vbox.addWidget(svg)
		about_vbox.addSpacing(10)
		about_vbox.addWidget(about2)
		about_vbox.addSpacing(10)
		about_vbox.addWidget(button_box)
		about_vbox.setAlignment(svg, Qt.AlignHCenter)

		msg.setLayout(about_vbox)
		msg.setWindowTitle("About ChronoPlotter")
		msg.setFixedSize(msg.minimumSizeHint())
		msg.exec_()



app = QApplication([])
chpl = ChronoPlotter()
sys.exit(app.exec_())
