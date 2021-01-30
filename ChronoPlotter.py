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
from PyQt5 import QtGui, QtCore
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QWidget, QApplication, QPushButton, QLabel, QFileDialog, QGridLayout, QCheckBox, QHBoxLayout, QVBoxLayout, QScrollArea, QFormLayout, QGroupBox, QTableWidget, QTableWidgetItem, QComboBox, QLineEdit, QDoubleSpinBox, QStackedWidget, QMessageBox

# Extracts data for a single series from a LabRadar CSV file
def extract_labradar_series_data(csvfile):
	csv_data = {"m_velocs": []}

	for idx, row in enumerate(csvfile):
		if idx == 4:
			csv_data["total_shots"] = int(row[1])
		elif idx == 6:
			csv_data["v_units"] = row[1]
		elif idx == 12:
			csv_data["v_lowest"] = int(float(row[1]))
		elif idx == 13:
			csv_data["v_highest"] = int(float(row[1]))
		elif idx > 17:
			if idx == 18:
				csv_data["first_date"] = row[15]
				csv_data["first_time"] = row[16]
			csv_data["m_velocs"].append(int(row[1]))

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
			elif row[0] == "Min":
				cur["v_lowest"] = int(row[1])
				cur["v_highest"] = int(row[3])
			elif str(row[0]).isdigit():
				print("Adding velocity %d" % int(row[2]))
				cur["m_velocs"].append(int(row[2]))
				if len(cur["m_velocs"]) == 1:
					cur["v_units"] = row[3]
			elif row[0] == "----":
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
		self.seriesCheckBoxes = []
		self.scroll_area = None

		self.initUI()

	def initUI(self):

		# Left-hand panel of user-populated series data
		self.label = QLabel("Select LabRadar or MagnetoSpeed directory to populate series data")

		self.stacked_widget = QStackedWidget()
		self.stacked_widget.addWidget(self.label)
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
		essd_layout.addWidget(self.essd_checkbox, 0)
		essd_layout.addWidget(QLabel("Show ES + SD above shot strings"), 1)
		self.options_layout.addLayout(essd_layout)

		# Show velocity data checkbox
		vd_layout = QHBoxLayout()
		self.vd_checkbox = QCheckBox()
		self.vd_checkbox.setChecked(True)
		vd_layout.addWidget(self.vd_checkbox, 0)
		vd_layout.addWidget(QLabel("Show velocity deltas below shot strings"), 1)
		self.options_layout.addLayout(vd_layout)

		self.create_graph_btn = QPushButton("Show graph", self)
		self.create_graph_btn.clicked.connect(self.showGraph)
		self.options_layout.addWidget(self.create_graph_btn)

		self.save_graph_btn = QPushButton("Save graph as image", self)
		self.save_graph_btn.clicked.connect(self.saveGraph)
		self.options_layout.addWidget(self.save_graph_btn)

		groupBox_options = QGroupBox("Graph options:")
		groupBox_options.setLayout(self.options_layout)

		# Layout to horizontally position series data and options
		layout = QHBoxLayout()
		layout.addWidget(groupBox, stretch=2)
		layout.addWidget(groupBox_options, stretch=1)

		self.dir_btn = QPushButton("Select directory", self)
		self.dir_btn.clicked.connect(self.dirDialog)
		self.dir_btn.setMinimumWidth(200)
		self.dir_btn.setMaximumWidth(200)

		# Layout to vertically position file dialog button and panels
		layout_vert = QVBoxLayout(self)
		layout_vert.addWidget(self.dir_btn)
		layout_vert.addLayout(layout)

		self.setGeometry(300, 300, 1000, 500)
		self.setWindowTitle("ChronoPlotter")
		self.show()

	def dirDialog(self):
		path = QFileDialog.getExistingDirectory(None, "Select directory")
		print("Selected directory: %s" % path)

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

		self.series = []

		# We don't know if we're looking at a LabRadar or MagnetoSpeed yet
		# LabRadar has a LBR/ directory in the root of its drive filled with SR####/ directories
		# MagnetoSpeed has a single LOG.CSV file in the root of its drive

		data_found = False

		lbr_path = os.path.join(path, "LBR")
		if os.path.exists(lbr_path) and os.path.isdir(lbr_path):
			path = lbr_path
			data_found = True
			print("Detected LabRadar directory '%s'. Using that directory instead." % lbr_path)

			msg = QMessageBox()
			msg.setIcon(QMessageBox.Information)
			msg.setText("Detected LabRadar directory, using '%s'" % lbr_path)
			msg.setWindowTitle("Information")
			msg.exec_()

		# Regex for LabRadar series directory
		pattern = re.compile("^SR\d\d\d\d")

		for fname in os.listdir(path):
			print(fname)

			fpath = os.path.join(path, fname)

			if fname == "LOG.CSV":
				csv_path = fpath
				print("Detected MagnetoSpeed file '%s'" % csv_path)

				msg = QMessageBox()
				msg.setIcon(QMessageBox.Information)
				msg.setText("Detected MagnetoSpeed directory, using '%s'" % path)
				msg.setWindowTitle("Information")
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
				print("Detected LabRadar directory, using '%s'" % path)

				if not data_found:
					msg = QMessageBox()
					msg.setIcon(QMessageBox.Information)
					msg.setText("Detected LabRadar directory, using '%s'" % path)
					msg.setWindowTitle("Information")
					msg.exec_()

					data_found = True

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

		# Sort the list
		self.series = sorted(self.series, key=lambda x: x[0])

		# Clear the placeholder text from the series data area
		self.stacked_widget.setCurrentIndex(1)

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

		self.scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
		self.scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
		self.scroll_area.setWidgetResizable(True)

	def showGraph(self, save_without_showing=False):
		print("showGraph clicked!")

		for val in self.series:
			series_num = val[0]
			series_name = val[1]
			csv_data = val[2]
			charge = val[3].value()
			checkbox = val[4]
			if checkbox.isChecked() and charge == 0:
				msg = QMessageBox()
				msg.setIcon(QMessageBox.Critical)
				msg.setText("Series %s is missing charge weight!" % series_name)
				msg.setWindowTitle("Error")
				msg.exec_()
				return

		plt.style.use("seaborn-whitegrid")
		matplotlib.rcParams['font.family'] = "DejaVu Sans"
		figure, ax = plt.subplots(1, figsize=(20,8))

		xticks = []
		averages = []

		last_average = None

		for i, val in enumerate(self.series):
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

		for i, val in enumerate(self.series):
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
					delta = average - last_average
					if delta < 0:
						sign = "-"
					else:
						sign = "+"

					if abs(delta) < 25:
						facecolor = "#00e810"
						alpha = (1 - (delta / 25.0)) * 0.6
					else:
						facecolor = "white"
						alpha = 1.0

					print("Placing velocity delta annotation")
					plt.annotate("%s%d" % (sign, delta), xy=(charge, bottom_label), ha="center", va="top", bbox=dict(boxstyle="square", facecolor=facecolor, alpha=alpha, linewidth=0))

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
		plt.suptitle(", ".join(filter(None, [rifle, propellant, projectile, brass, primer])), fontsize=14, color="#4d4d4d", y=0.88)
		plt.xlabel("Powder Charge (%s)" % weight_units, fontsize=14, labelpad=15, color="#4d4d4d")
		plt.ylabel("Velocity (%s)" % v_units, fontsize=14, labelpad=15, color="#4d4d4d")

		plt.xticks(xticks)

		if save_without_showing:
			path = QFileDialog.getSaveFileName(self, "Save graph as image")
			print(path)
			figure.savefig(path[0])

			msg = QMessageBox()
			msg.setIcon(QMessageBox.Information)
			msg.setText("Graph saved to %s" % path[0])
			msg.setWindowTitle("Success")
			msg.exec_()
		else:
			plt.show()

	def saveGraph(self):
		print("saveGraph clicked!")
		self.showGraph(save_without_showing=True)


app = QApplication([])
chpl = ChronoPlotter()
sys.exit(app.exec_())
