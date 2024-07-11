#include <sstream>

#include "untar.h"
#include "miniz.h"
#include "ChronoPlotter.h"
#include "PowderTest.h"

#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"

using namespace Powder;

PowderTest::PowderTest ( QWidget *parent )
	: QWidget(parent)
{
	qDebug() << "Powder test";

	graphPreview = NULL;
	prevLabRadarDir = QDir::homePath();
	prevMagnetoSpeedDir = QDir::homePath();
	prevProChronoDir = QDir::homePath();
	prevGarminDir = QDir::homePath();
	prevShotMarkerDir = QDir::homePath();
	prevSaveDir = QDir::homePath();

	/* Left panel */

	QVBoxLayout *leftLayout = new QVBoxLayout();

	QLabel *selectLabel = new QLabel("Select chronograph type\nto populate series data\n");
	selectLabel->setAlignment(Qt::AlignCenter);

	QPushButton *lrDirButton = new QPushButton("Select LabRadar directory");
	connect(lrDirButton, SIGNAL(clicked(bool)), this, SLOT(selectLabRadarDirectory(bool)));
	lrDirButton->setMinimumWidth(300);
	lrDirButton->setMaximumWidth(300);
	lrDirButton->setMinimumHeight(50);
	lrDirButton->setMaximumHeight(50);

	QPushButton *msFileButton = new QPushButton("Select MagnetoSpeed file");
	connect(msFileButton, SIGNAL(clicked(bool)), this, SLOT(selectMagnetoSpeedFile(bool)));
	msFileButton->setMinimumWidth(300);
	msFileButton->setMaximumWidth(300);
	msFileButton->setMinimumHeight(50);
	msFileButton->setMaximumHeight(50);

	QPushButton *pcFileButton = new QPushButton("Select ProChrono file");
	connect(pcFileButton, SIGNAL(clicked(bool)), this, SLOT(selectProChronoFile(bool)));
	pcFileButton->setMinimumWidth(300);
	pcFileButton->setMaximumWidth(300);
	pcFileButton->setMinimumHeight(50);
	pcFileButton->setMaximumHeight(50);

	QPushButton *gFileButton = new QPushButton("Select Garmin CSV/XLSX file");
	connect(gFileButton, SIGNAL(clicked(bool)), this, SLOT(selectGarminFile(bool)));
	gFileButton->setMinimumWidth(300);
	gFileButton->setMaximumWidth(300);
	gFileButton->setMinimumHeight(50);
	gFileButton->setMaximumHeight(50);

	QPushButton *smFileButton = new QPushButton("Select ShotMarker file");
	connect(smFileButton, SIGNAL(clicked(bool)), this, SLOT(selectShotMarkerFile(bool)));
	smFileButton->setMinimumWidth(300);
	smFileButton->setMaximumWidth(300);
	smFileButton->setMinimumHeight(50);
	smFileButton->setMaximumHeight(50);

	QPushButton *manualEntryButton = new QPushButton("Manual data entry");
	connect(manualEntryButton, SIGNAL(clicked(bool)), this, SLOT(manualDataEntry(bool)));
	manualEntryButton->setMinimumWidth(300);
	manualEntryButton->setMaximumWidth(300);
	manualEntryButton->setMinimumHeight(50);
	manualEntryButton->setMaximumHeight(50);

	QVBoxLayout *placeholderLayout = new QVBoxLayout();
	placeholderLayout->addStretch(0);
	placeholderLayout->addWidget(selectLabel);
	placeholderLayout->setAlignment(selectLabel, Qt::AlignCenter);
	placeholderLayout->addWidget(lrDirButton);
	placeholderLayout->setAlignment(lrDirButton, Qt::AlignCenter);
	placeholderLayout->addWidget(msFileButton);
	placeholderLayout->setAlignment(msFileButton, Qt::AlignCenter);
	placeholderLayout->addWidget(pcFileButton);
	placeholderLayout->setAlignment(pcFileButton, Qt::AlignCenter);
	placeholderLayout->addWidget(gFileButton);
	placeholderLayout->setAlignment(gFileButton, Qt::AlignCenter);
	placeholderLayout->addWidget(smFileButton);
	placeholderLayout->setAlignment(smFileButton, Qt::AlignCenter);
	placeholderLayout->addWidget(manualEntryButton);
	placeholderLayout->setAlignment(manualEntryButton, Qt::AlignCenter);
	placeholderLayout->addStretch(0);

	QWidget *placeholderWidget = new QWidget();
	placeholderWidget->setLayout(placeholderLayout);

	stackedWidget = new QStackedWidget();
	stackedWidget->addWidget(placeholderWidget);
	stackedWidget->setCurrentIndex(0);

	QVBoxLayout *stackedLayout = new QVBoxLayout();
	stackedLayout->addWidget(stackedWidget);
	QGroupBox *chronoGroupBox = new QGroupBox("Chronograph data:");
	chronoGroupBox->setLayout(stackedLayout);

	leftLayout->addWidget(chronoGroupBox);

	/* Right panel */

	QVBoxLayout *rightLayout = new QVBoxLayout();

	QFormLayout *detailsFormLayout = new QFormLayout();

	graphTitle = new QLineEdit();
	rifle = new QLineEdit();
	projectile = new QLineEdit();
	propellant = new QLineEdit();
	brass = new QLineEdit();
	primer = new QLineEdit();
	weather = new QLineEdit();
	detailsFormLayout->addRow(new QLabel("Graph title:"), graphTitle);
	detailsFormLayout->addRow(new QLabel("Rifle:"), rifle);
	detailsFormLayout->addRow(new QLabel("Projectile:"), projectile);
	detailsFormLayout->addRow(new QLabel("Propellant:"), propellant);
	detailsFormLayout->addRow(new QLabel("Brass:"), brass);
	detailsFormLayout->addRow(new QLabel("Primer:"), primer);
	detailsFormLayout->addRow(new QLabel("Weather:"), weather);

	QGroupBox *detailsGroupBox = new QGroupBox("Graph details:");
	detailsGroupBox->setLayout(detailsFormLayout);

	/* Graph options */

	QVBoxLayout *optionsLayout = new QVBoxLayout();

	QFormLayout *optionsFormLayout = new QFormLayout();

	graphType = new QComboBox();
	graphType->addItem("Scatter plot");
	graphType->addItem("Line chart + SD bars");
	optionsFormLayout->addRow(new QLabel("Graph type:"), graphType);

	weightUnits = new QComboBox();
	weightUnits->addItem("grain (gr)");
	weightUnits->addItem("gram (g)");
	optionsFormLayout->addRow(new QLabel("Weight units:"), weightUnits);

	velocityUnits = new QComboBox();
	velocityUnits->addItem("feet per second (ft/s)");
	velocityUnits->addItem("meters per second (m/s)");
	optionsFormLayout->addRow(new QLabel("Velocity units:"), velocityUnits);

	xAxisSpacing = new QComboBox();
	xAxisSpacing->addItem("Proportional");
	xAxisSpacing->addItem("Constant");
	connect(xAxisSpacing, SIGNAL(currentIndexChanged(int)), this, SLOT(xAxisSpacingChanged(int)));
	optionsFormLayout->addRow(new QLabel("X-axis spacing:"), xAxisSpacing);

	optionsLayout->addLayout(optionsFormLayout);
	optionsLayout->addWidget(new QHLine());

	QHBoxLayout *esLayout = new QHBoxLayout();
	esCheckBox = new QCheckBox();
	esCheckBox->setChecked(true);
	connect(esCheckBox, SIGNAL(clicked(bool)), this, SLOT(esCheckBoxChanged(bool)));
	esLayout->addWidget(esCheckBox, 0);
	esLabel = new QLabel("Show ES");
	esLayout->addWidget(esLabel, 1);
	esLocation = new QComboBox();
	esLocation->addItem("above shot strings");
	esLocation->addItem("below shot strings");
	esLayout->addWidget(esLocation);
	optionsLayout->addLayout(esLayout);

	QHBoxLayout *sdLayout = new QHBoxLayout();
	sdCheckBox = new QCheckBox();
	sdCheckBox->setChecked(true);
	connect(sdCheckBox, SIGNAL(clicked(bool)), this, SLOT(sdCheckBoxChanged(bool)));
	sdLayout->addWidget(sdCheckBox, 0);
	sdLabel = new QLabel("Show SD");
	sdLayout->addWidget(sdLabel, 1);
	sdLocation = new QComboBox();
	sdLocation->addItem("above shot strings");
	sdLocation->addItem("below shot strings");
	sdLayout->addWidget(sdLocation);
	optionsLayout->addLayout(sdLayout);

	QHBoxLayout *avgLayout = new QHBoxLayout();
	avgCheckBox = new QCheckBox();
	avgCheckBox->setChecked(false);
	connect(avgCheckBox, SIGNAL(clicked(bool)), this, SLOT(avgCheckBoxChanged(bool)));
	avgLayout->addWidget(avgCheckBox, 0);
	avgLabel = new QLabel("Show avg. velocity");
	avgLayout->addWidget(avgLabel, 1);
	avgLocation = new QComboBox();
	avgLocation->addItem("above shot strings");
	avgLocation->addItem("below shot strings");
	avgLocation->setEnabled(false);
	avgLayout->addWidget(avgLocation);
	optionsLayout->addLayout(avgLayout);

	QHBoxLayout *vdLayout = new QHBoxLayout();
	vdCheckBox = new QCheckBox();
	vdCheckBox->setChecked(true);
	connect(vdCheckBox, SIGNAL(clicked(bool)), this, SLOT(vdCheckBoxChanged(bool)));
	vdLayout->addWidget(vdCheckBox, 0);
	vdLabel = new QLabel("Show veloc. deltas");
	vdLayout->addWidget(vdLabel, 1);
	vdLocation = new QComboBox();
	vdLocation->addItem("above shot strings");
	vdLocation->addItem("below shot strings");
	vdLocation->setCurrentIndex(1);
	vdLayout->addWidget(vdLocation);
	optionsLayout->addLayout(vdLayout);

	QHBoxLayout *trendLayout = new QHBoxLayout();
	trendCheckBox = new QCheckBox();
	trendCheckBox->setChecked(false);
	connect(trendCheckBox, SIGNAL(clicked(bool)), this, SLOT(trendCheckBoxChanged(bool)));
	trendLayout->addWidget(trendCheckBox, 0);
	trendLabel = new QLabel("Show trend line");
	trendLayout->addWidget(trendLabel, 1);
	trendLineType = new QComboBox();
	trendLineType->addItem("solid line");
	trendLineType->addItem("dashed line");
	trendLineType->setCurrentIndex(1);
	trendLineType->setEnabled(false);
	trendLayout->addWidget(trendLineType);
	optionsLayout->addLayout(trendLayout);

	// Don't resize row heights if window height changes
	optionsLayout->addStretch(0);

	QGroupBox *optionsGroupBox = new QGroupBox("Graph options:");
	optionsGroupBox->setLayout(optionsLayout);

	QVBoxLayout *graphButtonsLayout = new QVBoxLayout();

	QPushButton *showGraphButton = new QPushButton("Show graph");
	connect(showGraphButton, SIGNAL(clicked(bool)), this, SLOT(showGraph(bool)));
	graphButtonsLayout->addWidget(showGraphButton);

	QPushButton *saveGraphButton = new QPushButton("Save graph as image");
	connect(saveGraphButton, SIGNAL(clicked(bool)), this, SLOT(saveGraph(bool)));
	graphButtonsLayout->addWidget(saveGraphButton);

	graphButtonsLayout->addStretch(0);

	/* Vertically position graph options and generate graph buttons */
	rightLayout->addWidget(detailsGroupBox);
	rightLayout->addWidget(optionsGroupBox);
	rightLayout->addLayout(graphButtonsLayout);

	/* Horizontally position series data and graph options */
	QHBoxLayout *pageLayout = new QHBoxLayout();
	pageLayout->addLayout(leftLayout, 2);
	pageLayout->addLayout(rightLayout, 0);

	this->setLayout(pageLayout);
}

static bool ChronoSeriesComparator ( ChronoSeries *one, ChronoSeries *two )
{
	return (one->seriesNum < two->seriesNum);
}

void PowderTest::DisplaySeriesData ( void )
{
	// Sort the list by series number
	std::sort(seriesData.begin(), seriesData.end(), ChronoSeriesComparator);

	// If we already have series data displayed, clear it out first. This call is a no-op if scrollWidget is not already added to stackedWidget.
	stackedWidget->removeWidget(scrollWidget);

	QVBoxLayout *scrollLayout = new QVBoxLayout();

	// Wrap grid in a widget to make the grid scrollable
	QWidget *scrollAreaWidget = new QWidget();

	seriesGrid = new QGridLayout(scrollAreaWidget);
	seriesGrid->setColumnStretch(0, 0);
	seriesGrid->setColumnStretch(1, 1);
	seriesGrid->setColumnStretch(2, 2);
	seriesGrid->setColumnStretch(3, 3);
	seriesGrid->setColumnStretch(4, 3);
	seriesGrid->setHorizontalSpacing(25);

	scrollArea = new QScrollArea();
	scrollArea->setWidget(scrollAreaWidget);
	scrollArea->setWidgetResizable(true);

	scrollLayout->addWidget(scrollArea);

	/* Create utilities toolbar under scroll area */

	QPushButton *loadNewButton = new QPushButton("Load new chronograph file");
	connect(loadNewButton, SIGNAL(clicked(bool)), this, SLOT(loadNewChronographData(bool)));
	loadNewButton->setMinimumWidth(225);
	loadNewButton->setMaximumWidth(225);

	QPushButton *rrButton = new QPushButton("Convert from round-robin");
	connect(rrButton, SIGNAL(clicked(bool)), this, SLOT(rrClicked(bool)));
	rrButton->setMinimumWidth(225);
	rrButton->setMaximumWidth(225);

	QPushButton *autofillButton = new QPushButton("Auto-fill charge weights");
	connect(autofillButton, SIGNAL(clicked(bool)), this, SLOT(autofillClicked(bool)));
	autofillButton->setMinimumWidth(225);
	autofillButton->setMaximumWidth(225);

	QHBoxLayout *utilitiesLayout = new QHBoxLayout();
	utilitiesLayout->addWidget(loadNewButton);
	utilitiesLayout->addWidget(rrButton);
	utilitiesLayout->addWidget(autofillButton);

	scrollLayout->addLayout(utilitiesLayout);

	scrollWidget = new QWidget();
	scrollWidget->setLayout(scrollLayout);

	stackedWidget->addWidget(scrollWidget);
	stackedWidget->setCurrentWidget(scrollWidget);

	QCheckBox *headerCheckBox = new QCheckBox();
	headerCheckBox->setChecked(true);
	connect(headerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(headerCheckBoxChanged(int)));
	seriesGrid->addWidget(headerCheckBox, 0, 0);

	/* Headers for series data */

	QLabel *headerName = new QLabel("Series Name");
	seriesGrid->addWidget(headerName, 0, 1, Qt::AlignVCenter);
	QLabel *headerChargeWeight = new QLabel("Charge Weight");
	seriesGrid->addWidget(headerChargeWeight, 0, 2, Qt::AlignVCenter);
	headerResult = new QLabel("Series Result");
	seriesGrid->addWidget(headerResult, 0, 3, Qt::AlignVCenter);
	QLabel *headerDate = new QLabel("Series Date");
	seriesGrid->addWidget(headerDate, 0, 4, Qt::AlignVCenter);

	for ( int i = 0; i < seriesData.size(); i++ )
	{
		ChronoSeries *series = seriesData.at(i);

		connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesCheckBoxChanged(int)));

		seriesGrid->addWidget(series->enabled, i + 1, 0);
		seriesGrid->addWidget(series->name, i + 1, 1, Qt::AlignVCenter);

		QHBoxLayout *chargeWeightLayout = new QHBoxLayout();
		chargeWeightLayout->addWidget(series->chargeWeight);
		chargeWeightLayout->addStretch(0);
		seriesGrid->addLayout(chargeWeightLayout, i + 1, 2);

		int totalShots = series->muzzleVelocities.size();
		double velocityMin = *std::min_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
		double velocityMax = *std::max_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
		QLabel *resultLabel = new QLabel(QString("%1 shot%2, %3-%4 %5").arg(totalShots).arg(totalShots > 1 ? "s" : "").arg(velocityMin).arg(velocityMax).arg(series->velocityUnits));
		seriesGrid->addWidget(resultLabel, i + 1, 3, Qt::AlignVCenter);

		QLabel *datetimeLabel = new QLabel(QString("%1 %2").arg(series->firstDate).arg(series->firstTime));
		seriesGrid->addWidget(datetimeLabel, i + 1, 4, Qt::AlignVCenter);

		seriesGrid->setRowMinimumHeight(0, series->chargeWeight->sizeHint().height());
	}

	// Add an empty stretch row at the end for proper spacing
	seriesGrid->setRowStretch(seriesData.size() + 1, 1);

	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);
}

void PowderTest::addNewClicked ( bool state )
{
	qDebug() << "addNewClicked state =" << state;

	// un-bold the button after the first click
	addNewButton->setStyleSheet("");

	int numRows = seriesGrid->rowCount();

	// Remove the stretch from the last row, we'll be using the row for our new series
	seriesGrid->setRowStretch(numRows - 1, 0);

	qDebug() << "numRows =" << numRows;

	ChronoSeries *series = new ChronoSeries();

	series->deleted = false;

	series->enabled = new QCheckBox();
	series->enabled->setChecked(true);

	connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesManualCheckBoxChanged(int)));
	seriesGrid->addWidget(series->enabled, numRows - 1, 0);

	int newSeriesNum = 1;
	for ( int i = seriesData.size() - 1; i >= 0; i-- )
	{
		ChronoSeries *series = seriesData.at(i);
		if ( ! series->deleted )
		{
			newSeriesNum = series->seriesNum + 1;
			qDebug() << "Found last un-deleted series" << series->seriesNum << "(" << series->name->text() << ") at index" << i;
			break;
		}
	}

	series->seriesNum = newSeriesNum;

	series->name = new QLabel(QString("Series %1").arg(newSeriesNum));
	seriesGrid->addWidget(series->name, numRows - 1, 1);

	series->chargeWeight = new QDoubleSpinBox();
	series->chargeWeight->setDecimals(2);
	series->chargeWeight->setSingleStep(0.1);
	series->chargeWeight->setMaximum(1000000);
	series->chargeWeight->setMinimumWidth(100);
	series->chargeWeight->setMaximumWidth(100);

	QHBoxLayout *chargeWeightLayout = new QHBoxLayout();
	chargeWeightLayout->addWidget(series->chargeWeight);
	chargeWeightLayout->addStretch(0);
	seriesGrid->addLayout(chargeWeightLayout, numRows - 1, 2);

	const char *velocityUnits2;
	if ( velocityUnits->currentIndex() == FPS )
	{
		velocityUnits2 = "ft/s";
	}
	else
	{
		velocityUnits2 = "m/s";
	}

	series->result = new QLabel(QString("0 shots, 0-0 %1").arg(velocityUnits2));
	seriesGrid->addWidget(series->result, numRows - 1, 3, Qt::AlignVCenter);

	series->enterDataButton = new QPushButton("Enter velocity data");
	connect(series->enterDataButton, SIGNAL(clicked(bool)), this, SLOT(enterDataClicked(bool)));
	series->enterDataButton->setFixedSize(series->enterDataButton->minimumSizeHint());
	seriesGrid->addWidget(series->enterDataButton, numRows - 1, 4, Qt::AlignLeft);

	series->deleteButton = new QPushButton();
	connect(series->deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteClicked(bool)));
	series->deleteButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
	series->deleteButton->setFixedSize(series->deleteButton->minimumSizeHint());
	seriesGrid->addWidget(series->deleteButton, numRows - 1, 5, Qt::AlignLeft);

	seriesData.append(series);

	// Add an empty stretch row at the end for proper spacing
	seriesGrid->setRowStretch(numRows, 1);
}

EnterVelocitiesDialog::EnterVelocitiesDialog ( ChronoSeries *series, QDialog *parent )
	: QDialog(parent)
{
	qDebug() << "Enter velocities dialog";

	setWindowTitle("Enter velocities");

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &EnterVelocitiesDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &EnterVelocitiesDialog::reject);

	QVBoxLayout *layout = new QVBoxLayout();

	velocitiesEntered = new QLabel(QString("Velocities entered: %1").arg(series->muzzleVelocities.size()));
	layout->addWidget(velocitiesEntered);

	textEdit = new QTextEdit();
	textEdit->setPlaceholderText("Enter velocity numbers here, each one on a new line.\n\nFor example:\n2785\n2782\n2798");

	if ( series->muzzleVelocities.size() > 0 )
	{
		QString prevVelocs;
		for ( int i = 0; i < series->muzzleVelocities.size(); i++ )
		{
			prevVelocs += QString::number(series->muzzleVelocities.at(i));
			if ( i < (series->muzzleVelocities.size() - 1) )
			{
				prevVelocs += "\n";
			}
		}

		textEdit->setPlainText(prevVelocs);
	}

	connect(textEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
	layout->addWidget(textEdit);

	layout->addWidget(buttonBox);
	setLayout(layout);

	setFixedSize(sizeHint());
}

void EnterVelocitiesDialog::textChanged ( )
{
	//qDebug() << "textChanged";

	QStringList list = textEdit->toPlainText().split("\n");
	//qDebug() << list;

	int numVelocities = 0;
	for ( int i = 0; i < list.length(); i++ )
	{
		// validate inputted number
		bool ok;
		list.at(i).toInt(&ok);
		if ( ok )
		{
			numVelocities++;
		}
		else
		{
			qDebug() << "Skipping invalid number:" << list.at(i);
		}
	}

	velocitiesEntered->setText(QString("Velocities entered: %1").arg(numVelocities));
}

QList<double> EnterVelocitiesDialog::getValues ( void )
{
	QList<double> values;

	QStringList list = textEdit->toPlainText().split("\n");
	qDebug() << list;

	for ( int i = 0; i < list.length(); i++ )
	{
		bool ok;
		int velocity = list.at(i).toInt(&ok);
		if ( ok )
		{
			values.append(velocity);
		}
		else
		{
			qDebug() << "Skipping invalid number:" << list.at(i);
		}
	}

	return values;
}

void PowderTest::enterDataClicked ( bool state )
{
	QPushButton *enterDataButton = qobject_cast<QPushButton *>(sender());

	qDebug() << "enterDataClicked state =" << state;

	// We have the button object, now locate its ChronoSeries object
	ChronoSeries *series = NULL;
	for ( int i = 0; i < seriesData.size(); i++ )
	{
		ChronoSeries *curSeries = seriesData.at(i);
		if ( enterDataButton == curSeries->enterDataButton )
		{
			series = curSeries;
			break;
		}
	}

	EnterVelocitiesDialog *dialog = new EnterVelocitiesDialog(series);
	int result = dialog->exec();

	qDebug() << "dialog result:" << result;

	if ( result )
	{
		qDebug() << "User OK'd dialog";

		QList<double> values = dialog->getValues();

		if ( values.size() == 0 )
		{
			qDebug() << "No valid velocities were provided, bailing...";
			return;
		}

		// We have the button object, now locate its row in the grid
		for ( int i = 0; i < seriesData.size(); i++ )
		{
			ChronoSeries *series = seriesData.at(i);
			if ( enterDataButton == series->enterDataButton )
			{
				qDebug() << "Setting velocities for Series" << series->seriesNum;

				series->muzzleVelocities = values;

				const char *velocityUnits2;
				if ( velocityUnits->currentIndex() == FPS )
				{
					velocityUnits2 = "ft/s";
				}
				else
				{
					velocityUnits2 = "m/s";
				}

				// Update the series result
				int totalShots = series->muzzleVelocities.size();
				double velocityMin = *std::min_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
				double velocityMax = *std::max_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
				series->result->setText(QString("%1 shot%2, %3-%4 %5").arg(totalShots).arg(totalShots > 1 ? "s" : "").arg(velocityMin).arg(velocityMax).arg(velocityUnits2));

				break;
			}
		}
	}
	else
	{
		qDebug() << "User cancelled dialog";
	}
}

void PowderTest::deleteClicked ( bool state )
{
	QPushButton *deleteButton = qobject_cast<QPushButton *>(sender());

	qDebug() << "deleteClicked state =" << state;

	int newSeriesNum = -1;

	// We have the checkbox object, now locate its row in the grid
	for ( int i = 0; i < seriesData.size(); i++ )
	{
		ChronoSeries *series = seriesData.at(i);
		if ( deleteButton == series->deleteButton )
		{
			qDebug() << "Series" << series->seriesNum << "(" << series->name->text() << ") was deleted";

			series->deleted = true;

			series->enabled->hide();
			series->name->hide();
			series->chargeWeight->hide();
			series->result->hide();
			series->enterDataButton->hide();
			series->deleteButton->hide();

			newSeriesNum = series->seriesNum;
		}
		else if ( (! series->deleted) && newSeriesNum > 0 )
		{
			qDebug() << "Updating Series" << series->seriesNum << "to Series" << newSeriesNum;

			series->seriesNum = newSeriesNum;
			series->name->setText(QString("Series %1").arg(newSeriesNum));

			newSeriesNum++;
		}
	}
}

void PowderTest::manualDataEntry ( bool state )
{
	qDebug() << "manualDataEntry state =" << state;

	// If we already have series data displayed, clear it out first. This call is a no-op if scrollWidget is not already added to stackedWidget.
	stackedWidget->removeWidget(scrollWidget);

	QVBoxLayout *scrollLayout = new QVBoxLayout();

	// Wrap grid in a widget to make the grid scrollable
	QWidget *scrollAreaWidget = new QWidget();

	seriesGrid = new QGridLayout(scrollAreaWidget);
	seriesGrid->setColumnStretch(0, 0);
	seriesGrid->setColumnStretch(1, 1);
	seriesGrid->setColumnStretch(2, 2);
	seriesGrid->setColumnStretch(3, 3);
	seriesGrid->setColumnStretch(4, 3);
	seriesGrid->setColumnStretch(5, 3);
	seriesGrid->setHorizontalSpacing(25);

	scrollArea = new QScrollArea();
	scrollArea->setWidget(scrollAreaWidget);
	scrollArea->setWidgetResizable(true);

	scrollLayout->addWidget(scrollArea);

	/* Create utilities toolbar under scroll area */

	addNewButton = new QPushButton("Add new series");
	addNewButton->setStyleSheet("font-weight: bold");
	connect(addNewButton, SIGNAL(clicked(bool)), this, SLOT(addNewClicked(bool)));
	addNewButton->setMinimumWidth(225);
	addNewButton->setMaximumWidth(225);

	QPushButton *loadNewButton = new QPushButton("Load new chronograph file");
	connect(loadNewButton, SIGNAL(clicked(bool)), this, SLOT(loadNewChronographData(bool)));
	loadNewButton->setMinimumWidth(225);
	loadNewButton->setMaximumWidth(225);

	QPushButton *autofillButton = new QPushButton("Auto-fill charge weights");
	connect(autofillButton, SIGNAL(clicked(bool)), this, SLOT(autofillClicked(bool)));
	autofillButton->setMinimumWidth(225);
	autofillButton->setMaximumWidth(225);

	QHBoxLayout *utilitiesLayout = new QHBoxLayout();
	utilitiesLayout->addWidget(addNewButton);
	utilitiesLayout->addWidget(loadNewButton);
	utilitiesLayout->addWidget(autofillButton);

	scrollLayout->addLayout(utilitiesLayout);

	scrollWidget = new QWidget();
	scrollWidget->setLayout(scrollLayout);

	stackedWidget->addWidget(scrollWidget);
	stackedWidget->setCurrentWidget(scrollWidget);

	QCheckBox *headerCheckBox = new QCheckBox();
	headerCheckBox->setChecked(true);
	connect(headerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(headerCheckBoxChanged(int)));
	seriesGrid->addWidget(headerCheckBox, 0, 0);

	QLabel *headerName = new QLabel("Series Name");
	seriesGrid->addWidget(headerName, 0, 1, Qt::AlignVCenter);
	QLabel *headerChargeWeight = new QLabel("Charge Weight");
	seriesGrid->addWidget(headerChargeWeight, 0, 2, Qt::AlignVCenter);
	headerResult = new QLabel("Series Result");
	seriesGrid->addWidget(headerResult, 0, 3, Qt::AlignVCenter);
	QLabel *headerEnterData = new QLabel("");
	seriesGrid->addWidget(headerEnterData, 0, 4, Qt::AlignVCenter);
	QLabel *headerDelete = new QLabel("");
	seriesGrid->addWidget(headerDelete, 0, 5, Qt::AlignVCenter);

	// Only connect this signal for manual data entry
	connect(velocityUnits, SIGNAL(activated(int)), this, SLOT(velocityUnitsChanged(int)));

	/* Create initial row */

	ChronoSeries *series = new ChronoSeries();

	series->deleted = false;

	series->enabled = new QCheckBox();
	series->enabled->setChecked(true);

	connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesManualCheckBoxChanged(int)));
	seriesGrid->addWidget(series->enabled, 1, 0);

	series->seriesNum = 1;

	series->name = new QLabel("Series 1");
	seriesGrid->addWidget(series->name, 1, 1);

	series->chargeWeight = new QDoubleSpinBox();
	series->chargeWeight->setDecimals(2);
	series->chargeWeight->setSingleStep(0.1);
	series->chargeWeight->setMaximum(1000000);
	series->chargeWeight->setMinimumWidth(100);
	series->chargeWeight->setMaximumWidth(100);

	QHBoxLayout *chargeWeightLayout = new QHBoxLayout();
	chargeWeightLayout->addWidget(series->chargeWeight);
	chargeWeightLayout->addStretch(0);
	seriesGrid->addLayout(chargeWeightLayout, 1, 2);

	const char *velocityUnits2;
	if ( velocityUnits->currentIndex() == FPS )
	{
		velocityUnits2 = "ft/s";
	}
	else
	{
		velocityUnits2 = "m/s";
	}

	series->result = new QLabel(QString("0 shots, 0-0 %1").arg(velocityUnits2));
	seriesGrid->addWidget(series->result, 1, 3, Qt::AlignVCenter);

	series->enterDataButton = new QPushButton("Enter velocity data");
	connect(series->enterDataButton, SIGNAL(clicked(bool)), this, SLOT(enterDataClicked(bool)));
	series->enterDataButton->setFixedSize(series->enterDataButton->minimumSizeHint());
	seriesGrid->addWidget(series->enterDataButton, 1, 4, Qt::AlignLeft);

	series->deleteButton = new QPushButton();
	connect(series->deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteClicked(bool)));
	series->deleteButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
	series->deleteButton->setFixedSize(series->deleteButton->minimumSizeHint());
	seriesGrid->addWidget(series->deleteButton, 1, 5, Qt::AlignLeft);

	seriesGrid->setRowMinimumHeight(0, series->chargeWeight->sizeHint().height());

	seriesData.append(series);

	// Add an empty stretch row at the end for proper spacing
	seriesGrid->setRowStretch(seriesGrid->rowCount(), 1);
}

void PowderTest::showGraph ( bool state )
{
	qDebug() << "showGraph state =" << state;

	renderGraph(true);
}

void PowderTest::saveGraph ( bool state )
{
	qDebug() << "showGraph state =" << state;

	renderGraph(false);
}

static bool ChargeWeightComparator ( ChronoSeries *one, ChronoSeries *two )
{
	return (one->chargeWeight->value() < two->chargeWeight->value());
}

void PowderTest::renderGraph ( bool displayGraphPreview )
{
	qDebug() << "renderGraph displayGraphPreview =" << displayGraphPreview;

	/* Validate series before continuing */

	int numEnabled = 0;
	for ( int i = 0; i < seriesData.size(); i++ )
	{
		ChronoSeries *series = seriesData.at(i);
		if ( (! series->deleted) && series->enabled->isChecked() )
		{
			numEnabled += 1;
			if ( series->chargeWeight->value() == 0 )
			{
				qDebug() << series->name->text() << "is missing charge weight, bailing";

				QMessageBox *msg = new QMessageBox();
				msg->setIcon(QMessageBox::Critical);
				msg->setText(QString("'%1' is missing charge weight!").arg(series->name->text()));
				msg->setWindowTitle("Error");
				msg->exec();
				return;
			}
			else if ( series->muzzleVelocities.size() == 0 )
			{
				qDebug() << series->name->text() << "is missing velocities, bailing";

				QMessageBox *msg = new QMessageBox();
				msg->setIcon(QMessageBox::Critical);
				msg->setText(QString("'%1' is missing velocities!").arg(series->name->text()));
				msg->setWindowTitle("Error");
				msg->exec();
				return;
			}
		}
	}

	if ( numEnabled < 2 )
	{
		qDebug() << "Only" << numEnabled << "series enabled, bailing";

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Critical);
		msg->setText("At least two series are required to graph!");
		msg->setWindowTitle("Error");
		msg->exec();
		return;
	}

	QCustomPlot *customPlot = new QCustomPlot();
	// TODO: dynamically calculate width based on graph contents
	customPlot->setGeometry(40, 40, 1440, 625);
	customPlot->setAntialiasedElements(QCP::aeAll);

	/* Make a copy of the subset of data actually being graphed */

	QList<ChronoSeries *> seriesToGraph;
	for ( int i = 0; i < seriesData.size(); i++ )
	{
		ChronoSeries *series = seriesData.at(i);

		if ( (! series->deleted) && series->enabled->isChecked() )
		{
			seriesToGraph.append(series);
		}
		else
		{
			qDebug() << "Series" << series->seriesNum << "is unchecked, skipping...";
		}
	}

	/* Sort the data by charge weight (in case the user inputted charge weights out of order */

	std::sort(seriesToGraph.begin(), seriesToGraph.end(), ChargeWeightComparator);

	/* Check if any cartridge lengths are duplicated. We can rely on series being sorted and not zero. */

	if ( xAxisSpacing->currentIndex() == PROPORTIONAL )
	{
		double lastChargeWeight = 0;
		for ( int i = 0; i < seriesToGraph.size(); i++ )
		{
			ChronoSeries *series = seriesToGraph.at(i);

			double chargeWeight = series->chargeWeight->value();

			if ( chargeWeight == lastChargeWeight )
			{
				qDebug() << "Duplicate charge weight detected" << chargeWeight << ", prompting user to switch to constant x-axis spacing";

				QMessageBox::StandardButton reply;
				reply = QMessageBox::question(this, "Duplicate charge weights", "Duplicate charge weights detected. Switching graph to constant spacing mode.", QMessageBox::Ok | QMessageBox::Cancel);

				if ( reply == QMessageBox::Ok )
				{
					qDebug() << "Set x-axis spacing to constant";

					xAxisSpacing->setCurrentIndex(CONSTANT);
					break;
				}
				else
				{
					qDebug() << "User cancel, bailing out";
					return;
				}
			}

			lastChargeWeight = chargeWeight;
		}
	}
	else
	{
		qDebug() << "Constant x-axis spacing selected, skipping duplicate check";
	}

	/* Collect the data to graph */

	QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);

	QVector<double> xPoints;
	QVector<double> yPoints;
	QVector<double> xAvgPoints;
	QVector<double> yAvgPoints;
	QVector<double> yError;
	QVector<double> allXPoints;
	QVector<double> allYPoints;

	for ( int i = 0; i < seriesToGraph.size(); i++ )
	{
		ChronoSeries *series = seriesToGraph.at(i);

		double chargeWeight = series->chargeWeight->value();

		qDebug() << QString("Series %1 (%2 gr)").arg(series->seriesNum).arg(chargeWeight);
		qDebug() << series->muzzleVelocities;

		int totalShots = series->muzzleVelocities.size();
		double mean = std::accumulate(series->muzzleVelocities.begin(), series->muzzleVelocities.end(), 0.0) / static_cast<double>(totalShots);
		double stdev = sampleStdev(series->muzzleVelocities);

		qDebug() << "Total shots:" << totalShots;
		qDebug() << "Mean:" << mean;
		qDebug() << "Stdev:" << stdev;
		qDebug() << "";

		/*
		 * The user can select either default x-axis spacing (x-ticks are spaced proportionally and irregular values will create "holes" in the
		 * graph) or force equal spacing regardless of value. The default case plots the x-values as usual, while the latter case plots a regularly
		 * incrementing index and overrides the xAxis ticker to display custom tick labels.
		 */

		if ( xAxisSpacing->currentIndex() == CONSTANT )
		{
			xAvgPoints.push_back(i);
		}
		else
		{
			xAvgPoints.push_back(chargeWeight);
		}
		yAvgPoints.push_back(mean);

		if ( graphType->currentIndex() == SCATTER )
		{
			for ( int j = 0; j < totalShots; j++ )
			{
				if ( xAxisSpacing->currentIndex() == CONSTANT )
				{
					xPoints.push_back(i);
					allXPoints.push_back(i);
				}
				else
				{
					xPoints.push_back(chargeWeight);
					allXPoints.push_back(chargeWeight);
				}
				yPoints.push_back(series->muzzleVelocities.at(j));
				allYPoints.push_back(series->muzzleVelocities.at(j));
			}
		}
		else
		{
			for ( int j = 0; j < totalShots; j++ )
			{
				if ( xAxisSpacing->currentIndex() == CONSTANT )
				{
					allXPoints.push_back(i);
				}
				else
				{
					allXPoints.push_back(chargeWeight);
				}
				allYPoints.push_back(series->muzzleVelocities.at(j));
			}

			if ( xAxisSpacing->currentIndex() == CONSTANT )
			{
				xPoints.push_back(i);
			}
			else
			{
				xPoints.push_back(chargeWeight);
			}
			yPoints.push_back(mean);
			yError.push_back(stdev);
		}

		if ( xAxisSpacing->currentIndex() == CONSTANT )
		{
			textTicker->addTick(i, QString::number(series->chargeWeight->value()));
		}
		else
		{
			textTicker->addTick(chargeWeight, QString::number(series->chargeWeight->value()));
		}
	}

	/* Create average line */

	QPen avgLinePen(Qt::SolidLine);
	QColor avgLineColor("#1c57eb");
	avgLineColor.setAlphaF(0.65);
	avgLinePen.setColor(avgLineColor);
	avgLinePen.setWidthF(1.5);

	QCPGraph *averageLine = customPlot->addGraph();
	averageLine->setData(xAvgPoints, yAvgPoints);
	averageLine->setScatterStyle(QCPScatterStyle::ssNone);
	averageLine->setPen(avgLinePen);

	/* Create scatter plot */

	QCPGraph *scatterPlot = customPlot->addGraph();
	scatterPlot->setData(xPoints, yPoints);
	scatterPlot->rescaleAxes();
	scatterPlot->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QColor("#0536b0"), 6.0));
	scatterPlot->setLineStyle(QCPGraph::lsNone);

	/* Draw SD error bars if necessary */

	if ( graphType->currentIndex() == LINE_SD )
	{
		QCPErrorBars *errorBars = new QCPErrorBars(customPlot->xAxis, customPlot->yAxis);
		errorBars->setData(yError);
		errorBars->setDataPlottable(averageLine);
		errorBars->rescaleAxes();

		// Also disable the x grid lines for readability
		customPlot->xAxis->grid()->setVisible(false);
	}

	/* Draw trend line if necessary */

	if ( trendCheckBox->isChecked() )
	{
		std::vector<double> res = GetLinearFit(allXPoints, allYPoints);
		qDebug() << "linear fit:" << res[0] << res[1];

		QVector<double> xTrendPoints;
		QVector<double> yTrendPoints;
		xTrendPoints.push_back(allXPoints.first());
		yTrendPoints.push_back(res[1] + (allXPoints.first() * res[0]));
		xTrendPoints.push_back(allXPoints.last());
		yTrendPoints.push_back(res[1] + (allXPoints.last() * res[0]));

		qDebug() << "xTrendPoints:" << xTrendPoints;
		qDebug() << "yTrendPoints:" << yTrendPoints;

		Qt::PenStyle lineType;
		if ( trendLineType->currentIndex() == SOLID_LINE )
		{
			lineType = Qt::SolidLine;
		}
		else
		{
			lineType = Qt::DashLine;
		}

		QPen trendLinePen(lineType);
		QColor trendLineColor(Qt::red);
		trendLineColor.setAlphaF(0.65);
		trendLinePen.setColor(trendLineColor);
		trendLinePen.setWidthF(1.5);

		QCPGraph *trendLine = customPlot->addGraph();
		trendLine->setData(xTrendPoints, yTrendPoints);
		trendLine->setScatterStyle(QCPScatterStyle::ssNone);
		trendLine->setPen(trendLinePen);
	}

	/* Configure rest of the graph */

	const char *weightUnits2;
	if ( weightUnits->currentIndex() == GRAINS )
	{
		weightUnits2 = "gr";
	}
	else
	{
		weightUnits2 = "g";
	}

	const char *velocityUnits2;
	if ( velocityUnits->currentIndex() == FPS )
	{
		velocityUnits2 = "ft/s";
	}
	else
	{
		velocityUnits2 = "m/s";
	}

	QCPTextElement *title = new QCPTextElement(customPlot);
	title->setText(QString("\n%1").arg(graphTitle->text()));
	title->setFont(QFont("DejaVu Sans", scaleFontSize(24)));
	title->setTextColor(QColor("#4d4d4d"));
	customPlot->plotLayout()->insertRow(0);
	customPlot->plotLayout()->addElement(0, 0, title);

	QCPTextElement *subtitle = new QCPTextElement(customPlot);
	QStringList subtitleText;
	subtitleText << rifle->text() << propellant->text() << projectile->text() << brass->text() << primer->text() << weather->text();
	subtitle->setText(StringListJoin(subtitleText, ", ") + "\n");
	subtitle->setFont(QFont("DejaVu Sans", scaleFontSize(12)));
	subtitle->setTextColor(QColor("#4d4d4d"));
	customPlot->plotLayout()->insertRow(1);
	customPlot->plotLayout()->addElement(1, 0, subtitle);

	QPen gridPen(Qt::SolidLine);
	gridPen.setColor("#d9d9d9");

	QPen axisBasePen(Qt::SolidLine);
	axisBasePen.setColor("#d9d9d9");
	axisBasePen.setWidth(2);

	customPlot->xAxis->setLabel(QString("Powder charge (%1)").arg(weightUnits2));
	customPlot->xAxis->scaleRange(1.1);
	customPlot->xAxis->setTicker(textTicker);
	customPlot->xAxis->setTickLabelFont(QFont("DejaVu Sans", scaleFontSize(9)));
	customPlot->xAxis->setTickLabelColor(QColor("#4d4d4d"));
	customPlot->xAxis->setLabelFont(QFont("DejaVu Sans", scaleFontSize(12)));
	customPlot->xAxis->setLabelColor(QColor("#4d4d4d"));
	customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
	customPlot->xAxis->grid()->setPen(gridPen);
	customPlot->xAxis->setBasePen(axisBasePen);
	customPlot->xAxis->setTickPen(Qt::NoPen);
	customPlot->xAxis->setSubTickPen(Qt::NoPen);
	customPlot->xAxis->setLabelPadding(13);
	customPlot->xAxis->setPadding(20);

	customPlot->xAxis2->setBasePen(axisBasePen);
	customPlot->xAxis2->setTickPen(Qt::NoPen);
	customPlot->xAxis2->setSubTickPen(Qt::NoPen);
	customPlot->xAxis2->setPadding(20);

	customPlot->yAxis->setLabel(QString("Velocity (%1)").arg(velocityUnits2));
	customPlot->yAxis->scaleRange(1.3);
	customPlot->yAxis->setTickLabelFont(QFont("DejaVu Sans", scaleFontSize(9)));
	customPlot->yAxis->setTickLabelColor(QColor("#4d4d4d"));
	customPlot->yAxis->setLabelFont(QFont("DejaVu Sans", scaleFontSize(12)));
	customPlot->yAxis->setLabelColor(QColor("#4d4d4d"));
	customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
	customPlot->yAxis->grid()->setPen(gridPen);
	customPlot->yAxis->setBasePen(axisBasePen);
	customPlot->yAxis->setTickPen(Qt::NoPen);
	customPlot->yAxis->setSubTickPen(Qt::NoPen);
	customPlot->yAxis->setLabelPadding(20);
	customPlot->yAxis->setPadding(20);

	customPlot->yAxis2->setBasePen(axisBasePen);
	customPlot->yAxis2->setTickPen(Qt::NoPen);
	customPlot->yAxis2->setSubTickPen(Qt::NoPen);
	customPlot->yAxis2->setPadding(20);

	// seems to strike a good balance of tick frequency and readability
	customPlot->yAxis->ticker()->setTickCount(6);

	customPlot->axisRect()->setupFullAxesBox();

	// Render the graph off-screen
	QPixmap picture(QSize(1440, 625));
	QCPPainter painter(&picture);
	customPlot->toPainter(&painter, 1440, 625);

	/*
	 * Generate bounding boxes and text annotations. We need to do this after rendering the graph so that coordToPixel() works.
	 */

	bool prevMeanSet = false;
	double prevMean = 0;

	for ( int i = 0; i < seriesToGraph.size(); i++ )
	{
		ChronoSeries *series = seriesToGraph.at(i);

		// Collect annotation contents. Only display ES and SD if there are 2+ shots in the series.

		double chargeWeight = series->chargeWeight->value();

		int totalShots = series->muzzleVelocities.size();
		double velocityMin = *std::min_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
		double velocityMax = *std::max_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
		double mean = std::accumulate(series->muzzleVelocities.begin(), series->muzzleVelocities.end(), 0.0) / static_cast<double>(totalShots);
		int es = velocityMax - velocityMin;
		double stdev = sampleStdev(series->muzzleVelocities);
		QStringList aboveAnnotationText;
		QStringList belowAnnotationText;

		if ( graphType->currentIndex() == SCATTER )
		{
			QCPItemRect *rect = new QCPItemRect(customPlot);
			QPen rectPen("#0536b0");
			rectPen.setWidthF(1.3);
			rect->setPen(rectPen);
			rect->topLeft->setType(QCPItemPosition::ptAbsolute);
			rect->bottomRight->setType(QCPItemPosition::ptAbsolute);
			if ( xAxisSpacing->currentIndex() == CONSTANT )
			{
				rect->topLeft->setCoords(customPlot->xAxis->coordToPixel(i) - 7, customPlot->yAxis->coordToPixel(velocityMax) - 7);
				rect->bottomRight->setCoords(customPlot->xAxis->coordToPixel(i) + 7, customPlot->yAxis->coordToPixel(velocityMin) + 7);
			}
			else
			{
				rect->topLeft->setCoords(customPlot->xAxis->coordToPixel(chargeWeight) - 7, customPlot->yAxis->coordToPixel(velocityMax) - 7);
				rect->bottomRight->setCoords(customPlot->xAxis->coordToPixel(chargeWeight) + 7, customPlot->yAxis->coordToPixel(velocityMin) + 7);
			}
		}

		if ( esCheckBox->isChecked() && (series->muzzleVelocities.size() > 1) )
		{
			QString annotation = QString("ES: %1").arg(es);
			if ( esLocation->currentIndex() == ABOVE_STRING )
			{
				aboveAnnotationText.append(annotation);
			}
			else
			{
				belowAnnotationText.append(annotation);
			}
		}

		if ( sdCheckBox->isChecked() && (series->muzzleVelocities.size() > 1) )
		{
			QString annotation = QString("SD: %1").arg(stdev, 0, 'f', 1);
			if ( sdLocation->currentIndex() == ABOVE_STRING )
			{
				aboveAnnotationText.append(annotation);
			}
			else
			{
				belowAnnotationText.append(annotation);
			}
		}

		if ( avgCheckBox->isChecked() )
		{
			QString annotation;
			if ( series->muzzleVelocities.size() > 1 )
			{
				annotation = QString("x\u0305: %1").arg(mean, 0, 'f', 1);
			}
			else
			{
				annotation = QString::number(mean);
			}

			if ( avgLocation->currentIndex() == ABOVE_STRING )
			{
				aboveAnnotationText.append(annotation);
			}
			else
			{
				belowAnnotationText.append(annotation);
			}
		}

		if ( vdCheckBox->isChecked() )
		{
			if ( prevMeanSet )
			{
				QString sign;
				int delta = round(mean - prevMean);
				if ( delta < 0 )
				{
					sign = QString("-");
				}
				else
				{
					sign = QString("+");
				}

				QString annotation = QString("%1%2").arg(sign).arg(abs(delta));
				if ( vdLocation->currentIndex() == ABOVE_STRING )
				{
					aboveAnnotationText.append(annotation);
				}
				else
				{
					belowAnnotationText.append(annotation);
				}
			}
		}

		// Obtain pixel coordinates for points. We need the min/max points for scatter plots and stdev for line + SD bar charts.

		double yCoordBelow;
		double yCoordAbove;

		if ( graphType->currentIndex() == SCATTER )
		{
			yCoordBelow = velocityMin;
			yCoordAbove = velocityMax;
		}
		else
		{
			if ( qIsNaN(stdev) )
			{
				yCoordBelow = mean;
				yCoordAbove = mean;
			}
			else
			{
				yCoordBelow = mean - stdev;
				yCoordAbove = mean + stdev;
			}
		}

		QCPItemText *belowAnnotation = new QCPItemText(customPlot);
		belowAnnotation->setText(belowAnnotationText.join('\n'));
		belowAnnotation->setFont(QFont("DejaVu Sans", scaleFontSize(9)));
		belowAnnotation->setColor(QColor("#4d4d4d"));
		belowAnnotation->position->setType(QCPItemPosition::ptAbsolute);
		if ( xAxisSpacing->currentIndex() == CONSTANT )
		{
			belowAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(i), customPlot->yAxis->coordToPixel(yCoordBelow) + 10);
		}
		else
		{
			belowAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(chargeWeight), customPlot->yAxis->coordToPixel(yCoordBelow) + 10);
		}
		belowAnnotation->setPositionAlignment(Qt::AlignHCenter | Qt::AlignTop);
		belowAnnotation->setTextAlignment(Qt::AlignCenter);
		belowAnnotation->setBrush(QBrush(Qt::white));
		belowAnnotation->setClipToAxisRect(false);
		belowAnnotation->setLayer(customPlot->layer(5));
		qDebug() << "min pixel coords:" << belowAnnotation->position->pixelPosition() << "layer:" << belowAnnotation->layer()->name() << "rect:" << customPlot->axisRect()->layer()->name();

		QCPItemText *aboveAnnotation = new QCPItemText(customPlot);
		aboveAnnotation->setText(aboveAnnotationText.join('\n'));
		aboveAnnotation->setFont(QFont("DejaVu Sans", scaleFontSize(9)));
		aboveAnnotation->setColor(QColor("#4d4d4d"));
		aboveAnnotation->position->setType(QCPItemPosition::ptAbsolute);
		if ( xAxisSpacing->currentIndex() == CONSTANT )
		{
			aboveAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(i), customPlot->yAxis->coordToPixel(yCoordAbove) - 10);
		}
		else
		{
			aboveAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(chargeWeight), customPlot->yAxis->coordToPixel(yCoordAbove) - 10);
		}
		aboveAnnotation->setPositionAlignment(Qt::AlignHCenter | Qt::AlignBottom);
		aboveAnnotation->setTextAlignment(Qt::AlignCenter);
		aboveAnnotation->setBrush(QBrush(Qt::white));
		aboveAnnotation->setClipToAxisRect(false);
		aboveAnnotation->setLayer(customPlot->layer(5));
		qDebug() << "max pixel coords:" << aboveAnnotation->position->pixelPosition() << "layer:" << aboveAnnotation->layer()->name() << "rect:" << customPlot->axisRect()->layer()->name();

		prevMean = mean;
		prevMeanSet = true;
	}

	if ( displayGraphPreview )
	{
		qDebug() << "Showing graph preview";

		//customPlot->show();
		QPixmap preview = customPlot->toPixmap(1440, 625, 2.0);

		if ( graphPreview )
		{
			graphPreview->deleteLater();
		}

		qDebug() << "xPoints:" << xPoints;
		qDebug() << "yPoints:" << yPoints;
		qDebug() << "allXPoints:" << allXPoints;
		qDebug() << "allYPoints:" << allYPoints;

		graphPreview = new GraphPreview(preview);
	}
	else
	{
		QString fileName;
		if ( graphTitle->text().isEmpty() )
		{
			fileName = QString("graph.png");
		}
		else
		{
			fileName = QString(graphTitle->text()).append(".png");
		}

		QString savePath = QDir(prevSaveDir).filePath(fileName);
		qDebug() << "graphTitle:" << graphTitle->text();
		qDebug() << "fileName:" << fileName;
		qDebug() << "savePath:" << savePath;

		QString path = QFileDialog::getSaveFileName(this, "Save graph as image", savePath, "PNG image (*.png);;JPG image (*.jpg);;PDF file (*.pdf)");
		qDebug() << "User selected save path:" << path;

		if ( path.isEmpty() )
		{
			qDebug() << "No path selected, bailing";
			return;
		}

		QFileInfo pathInfo(path);
		prevSaveDir = pathInfo.absolutePath();

		QStringList allowedExts;
		allowedExts << "png" << "jpg" << "pdf";

		QString pathExt = pathInfo.suffix().toLower();

		if ( pathExt.isEmpty() || (! allowedExts.contains(pathExt)) )
		{
			path.append(".png");
			pathExt = "png";;
		}

		qDebug() << "Using save path:" << path;

		bool res;
		if ( pathExt == "png")
		{
			res = customPlot->savePng(path, 1440, 625, 2.0);
		}
		else if ( pathExt == "jpg" )
		{
			res = customPlot->saveJpg(path, 1440, 625, 2.0);
		}
		else if ( pathExt == "pdf" )
		{
			res = customPlot->savePdf(path, 1440, 625);
		}
		else
		{
			qDebug() << "error, shouldn't be reached";
			res = false;
		}

		qDebug() << "save file res =" << res;

		if ( res )
		{
			QMessageBox::information(this, "Save file", QString("Saved file to '%1'").arg(path), QMessageBox::Ok, QMessageBox::Ok);
		}
		if ( res == false )
		{
			QMessageBox::warning(this, "Save file", QString("Unable to save file to '%1'\n\nPlease choose a different path").arg(path), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
}

void PowderTest::seriesCheckBoxChanged ( int state )
{
	QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());

	qDebug() << "seriesCheckBoxChanged state =" << state << " checkBox =" << checkBox;

	// We have the checkbox object, now locate its row in the grid
	for ( int i = 0; i < seriesGrid->rowCount() - 1; i++ )
	{
		QWidget *rowCheckBox = seriesGrid->itemAtPosition(i, 0)->widget();

		if ( checkBox == rowCheckBox )
		{
			QWidget *seriesName = seriesGrid->itemAtPosition(i, 1)->widget();
			QWidget *chargeWeight = seriesGrid->itemAtPosition(i, 2)->layout()->itemAt(0)->widget();
			QWidget *seriesResult = seriesGrid->itemAtPosition(i, 3)->widget();
			QWidget *seriesDate = seriesGrid->itemAtPosition(i, 4)->widget();

			if ( checkBox->isChecked() )
			{
				qDebug() << "Grid row" << i << "was checked";

				seriesName->setEnabled(true);
				chargeWeight->setEnabled(true);
				seriesResult->setEnabled(true);
				seriesDate->setEnabled(true);
			}
			else
			{
				qDebug() << "Grid row" << i << "was unchecked";

				seriesName->setEnabled(false);
				chargeWeight->setEnabled(false);
				seriesResult->setEnabled(false);
				seriesDate->setEnabled(false);
			}

			return;
		}
	}
}

void PowderTest::seriesManualCheckBoxChanged ( int state )
{
	QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());

	qDebug() << "seriesManualCheckBoxChanged state =" << state << " checkBox =" << checkBox;

	// We have the checkbox object, now locate its row in the grid
	for ( int i = 0; i < seriesGrid->rowCount() - 1; i++ )
	{
		QWidget *rowCheckBox = seriesGrid->itemAtPosition(i, 0)->widget();

		if ( checkBox == rowCheckBox )
		{
			QWidget *seriesName = seriesGrid->itemAtPosition(i, 1)->widget();
			QWidget *chargeWeight = seriesGrid->itemAtPosition(i, 2)->layout()->itemAt(0)->widget();
			QWidget *seriesResult = seriesGrid->itemAtPosition(i, 3)->widget();
			QWidget *seriesEnterData = seriesGrid->itemAtPosition(i, 4)->widget();
			QWidget *seriesDelete = seriesGrid->itemAtPosition(i, 5)->widget();

			if ( checkBox->isChecked() )
			{
				qDebug() << "Grid row" << i << "was checked";

				seriesName->setEnabled(true);
				chargeWeight->setEnabled(true);
				seriesResult->setEnabled(true);
				seriesEnterData->setEnabled(true);
				seriesDelete->setEnabled(true);
			}
			else
			{
				qDebug() << "Grid row" << i << "was unchecked";

				seriesName->setEnabled(false);
				chargeWeight->setEnabled(false);
				seriesResult->setEnabled(false);
				seriesEnterData->setEnabled(false);
				seriesDelete->setEnabled(false);
			}

			return;
		}
	}
}

void PowderTest::headerCheckBoxChanged ( int state )
{
	qDebug() << "headerCheckBoxChanged state =" << state;

	if ( state == Qt::Checked )
	{
		qDebug() << "Header checkbox was checked";

		for ( int i = 0; i < seriesGrid->rowCount() - 1; i++ )
		{
			QCheckBox *checkBox = qobject_cast<QCheckBox *>(seriesGrid->itemAtPosition(i, 0)->widget());
			checkBox->setChecked(true);
		}
	}
	else
	{
		qDebug() << "Header checkbox was unchecked";

		for ( int i = 0; i < seriesGrid->rowCount() - 1; i++ )
		{
			QCheckBox *checkBox = qobject_cast<QCheckBox *>(seriesGrid->itemAtPosition(i, 0)->widget());
			checkBox->setChecked(false);
		}
	}
}

void PowderTest::velocityUnitsChanged ( int index )
{
	qDebug() << "velocityUnitsChanged index =" << index;

	/*
	 * This signal handler is only connected in manual data entry mode. When the velocity unit is changed, we
	 * need to iterate through and update every Series Result row to display the new unit.
	 */

	const char *velocityUnit;

	if ( index == FPS )
	{
		velocityUnit = "ft/s";
	}
	else
	{
		velocityUnit = "m/s";
	}

	for ( int i = 0; i < seriesData.size(); i++ )
	{
		ChronoSeries *series = seriesData.at(i);
		if ( ! series->deleted )
		{
			qDebug() << "Setting series" << i << "velocity unit to" << velocityUnit;

			if ( series->muzzleVelocities.size() == 0 )
			{
				series->result->setText(QString("0 shots, 0-0 %1").arg(velocityUnit));
			}
			else
			{
				int totalShots = series->muzzleVelocities.size();
				double velocityMin = *std::min_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
				double velocityMax = *std::max_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
				series->result->setText(QString("%1 shot%2, %3-%4 %5").arg(totalShots).arg(totalShots > 1 ? "s" : "").arg(velocityMin).arg(velocityMax).arg(velocityUnit));
			}
		}
	}
}

void PowderTest::esCheckBoxChanged ( bool state )
{
	qDebug() << "esCheckBoxChanged state =" << state;

	optionCheckBoxChanged(esCheckBox, esLabel, esLocation);
}

void PowderTest::sdCheckBoxChanged ( bool state )
{
	qDebug() << "sdsCheckBoxChanged state =" << state;

	optionCheckBoxChanged(sdCheckBox, sdLabel, sdLocation);
}

void PowderTest::avgCheckBoxChanged ( bool state )
{
	qDebug() << "avgCheckBoxChanged state =" << state;

	optionCheckBoxChanged(avgCheckBox, avgLabel, avgLocation);
}

void PowderTest::vdCheckBoxChanged ( bool state )
{
	qDebug() << "vdCheckBoxChanged state =" << state;

	optionCheckBoxChanged(vdCheckBox, vdLabel, vdLocation);
}

void PowderTest::trendCheckBoxChanged ( bool state )
{
	qDebug() << "trendCheckBoxChanged state =" << state;

	optionCheckBoxChanged(trendCheckBox, trendLabel, trendLineType);
}

void PowderTest::xAxisSpacingChanged ( int index )
{
	qDebug() << "xAxisSpacingChanged index =" << index;

	if ( index == CONSTANT )
	{
		trendCheckBox->setChecked(false);
		trendCheckBox->setEnabled(false);
		trendLabel->setStyleSheet("color: #878787");
		trendLineType->setEnabled(false);
	}
	else
	{
		trendCheckBox->setEnabled(true);
		trendLabel->setStyleSheet("");
		trendLineType->setEnabled(false); // we always re-enable the checkbox unchecked, so the combobox stays disabled
	}
}

void PowderTest::optionCheckBoxChanged ( QCheckBox *checkBox, QLabel *label, QComboBox *comboBox )
{
	if ( checkBox->isChecked() )
	{
		qDebug() << "checkbox was checked";
		comboBox->setEnabled(true);
	}
	else
	{
		qDebug() << "checkbox was unchecked";
		comboBox->setEnabled(false);
	}
}

void PowderTest::loadNewChronographData ( bool state )
{
	qDebug() << "loadNewChronographData state =" << state;

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Load new data", "Are you sure you want to load new chronograph data?\n\nThis will clear your current work.", QMessageBox::Yes | QMessageBox::Cancel);

	if ( reply == QMessageBox::Yes )
	{
		qDebug() << "User said yes";

		// Hide the chronograph data screen. This returns to the initial screen to choose a new chronograph file.
		stackedWidget->removeWidget(scrollWidget);

		// Delete the loaded chronograph data
		seriesData.clear();

		// Disconnect the velocity units header signal (used in manual data entry), if necessary
		disconnect(velocityUnits, SIGNAL(activated(int)), this, SLOT(velocityUnitsChanged(int)));
	}
	else
	{
		qDebug() << "User said cancel";
	}
}

void PowderTest::selectLabRadarDirectory ( bool state )
{
	qDebug() << "selectLabRadarDirectory state =" << state;

	qDebug() << "Previous directory:" << prevLabRadarDir;

	QString path = QFileDialog::getExistingDirectory(this, "Select directory", prevLabRadarDir);
	prevLabRadarDir = path;

	qDebug() << "Selected directory:" << path;

	if ( path.isEmpty() )
	{
		qDebug() << "User didn't select a directory, bail";
		return;
	}

	seriesData.clear();

	/*
	 * Look for LabRadar data. LabRadar has a LBR/ directory in the root of its drive filled with SR####/ directories.
	 * We handle the case where the user selected the root of the SD card (and the series directories are actually in LBR/),
	 * or if we're inside one of the actual series directories and we need to be one directory up to enumerate all of them.
	 */

	QDir lbrPath(path);
	lbrPath.setPath(lbrPath.filePath("LBR"));
	if ( lbrPath.exists() )
	{
		path = lbrPath.path();
		qDebug() << "Detected LabRadar directory" << path << ". Using that directory instead.";
	}

	QDir trkPath(path);
	trkPath.setPath(trkPath.filePath("TRK"));
	if ( trkPath.exists() )
	{
		trkPath.setPath(trkPath.filePath("../.."));
		path = trkPath.canonicalPath();
		qDebug() << "Detected LabRadar directory" << path << ". Using one directory level up instead.";
	}

	qDebug() << "path:" << path;

	/* Enumerate the LabRadar directory */

	QRegularExpression re;
	re.setPattern("^SR\\d\\d\\d\\d.*");

	QDir dir(path);
	QStringList items = dir.entryList(QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);

	foreach ( QString fileName, items )
	{
		qDebug() << "Entry:" << fileName;
		if ( re.match(fileName).hasMatch() )
		{
			qDebug() << "Detected LabRadar series directory" << fileName;

			QString seriesPath(dir.filePath(fileName));
			QDir seriesDir(seriesPath);
			QStringList csvItems = seriesDir.entryList(QStringList() << "* Report.csv", QDir::Files | QDir::NoDotAndDotDot);
			QString csvFileName = csvItems.at(0);

			qDebug() << "CSV file:" << csvFileName;

			QFile csvFile(seriesDir.filePath(csvFileName));
			csvFile.open(QIODevice::ReadOnly);
			QTextStream csv(&csvFile);

			ChronoSeries *series = ExtractLabRadarSeries(csv);

			if ( ! series->isValid )
			{
				qDebug() << "Invalid series, skipping...";
				continue;
			}

			series->enabled = new QCheckBox();
			series->enabled->setChecked(true);

			series->name = new QLabel(fileName);

			series->chargeWeight = new QDoubleSpinBox();
			series->chargeWeight->setDecimals(2);
			series->chargeWeight->setSingleStep(0.1);
			series->chargeWeight->setMaximum(1000000);
			series->chargeWeight->setMinimumWidth(100);
			series->chargeWeight->setMaximumWidth(100);

			seriesData.append(series);

			csvFile.close();
		}
	}

	/* We're finished enumerating the directory */

	if ( seriesData.empty() )
	{
		qDebug() << "Didn't find any chrono data in this directory, bail";

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Critical);
		msg->setText(QString("Unable to find LabRadar data in '%1'").arg(path));
		msg->setWindowTitle("Error");
		msg->exec();
	}
	else
	{
		qDebug() << "Detected LabRadar directory" << path;

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Information);
		msg->setText(QString("Detected LabRadar data\n\nUsing '%1'").arg(path));
		msg->setWindowTitle("Success");
		msg->exec();

		// Proceed to display the data
		DisplaySeriesData();
	}
}

ChronoSeries *PowderTest::ExtractLabRadarSeries ( QTextStream &csv )
{
	ChronoSeries *series = new ChronoSeries();
	series->isValid = false;
	series->deleted = false;
	series->seriesNum = -1;

	while ( ! csv.atEnd() )
	{
		QString line = csv.readLine().replace(QString(1, QChar('\0')), "");

		// LabRadar uses semicolon (;) as delimeter
		QStringList rows(line.split(";"));
		//qDebug() << "Line:" << rows;

		// Only parse rows with enough columns to index
		if ( rows.size() < 2 )
		{
			qDebug() << "Less than 2, skipping row";
			continue;
		}

		if ( (rows.size() >= 17) && (rows.at(0).compare("Shot ID") != 0) )
		{
			// Parsing a velocity record
			if ( series->firstDate.isNull() )
			{
				series->firstDate = rows.at(15);
				qDebug() << "firstDate =" << series->firstDate;
			}

			if ( series->firstTime.isNull() )
			{
				series->firstTime = rows.at(16);
				qDebug() << "firstTime =" << series->firstTime;
			}

			series->muzzleVelocities.append(rows.at(1).toInt());
			qDebug() << "muzzleVelocities +=" << rows.at(1).toInt();
		}
		else if ( rows.at(0).compare("Series No") == 0 )
		{
			series->seriesNum = rows.at(1).toInt();
			qDebug() << "seriesNum =" << series->seriesNum;
		}
		else if ( rows.at(0).compare("Units velocity") == 0 )
		{
			series->velocityUnits = rows.at(1);
			series->velocityUnits.replace("fps", "ft/s");
			qDebug() << "velocityUnits =" << series->velocityUnits;
		}
	}

	// Ensure we have a valid LabRadar series
	if ( (series->seriesNum == -1) || series->velocityUnits.isNull() || series->firstDate.isNull() || series->firstTime.isNull() )
	{
		qDebug() << "Series does not have all expected fields set, returning invalid.";
		return series;
	}

	if ( series->muzzleVelocities.empty() )
	{
		qDebug() << "Series has no velocities. Likely deleted series, returning invalid.";
		return series;
	}

	// We have a valid series CSV
	series->isValid = true;

	return series;
}

void PowderTest::selectMagnetoSpeedFile ( bool state )
{
	qDebug() << "selectMagnetoSpeedFile state =" << state;

	qDebug() << "Previous directory:" << prevMagnetoSpeedDir;

	QString path = QFileDialog::getOpenFileName(this, "Select file", prevMagnetoSpeedDir, "CSV files (*.csv)");
	prevMagnetoSpeedDir = path;

	qDebug() << "Selected file:" << path;

	if ( path.isEmpty() )
	{
		qDebug() << "User didn't select a file, bail";
		return;
	}

	seriesData.clear();

	/*
	 * MagnetoSpeed records all of its series data in a single LOG.CSV file
	 */

	QFile csvFile(path);
	csvFile.open(QIODevice::ReadOnly);
	QTextStream csv(&csvFile);

	QList<ChronoSeries *> allSeries = ExtractMagnetoSpeedSeries(csv);

	qDebug() << "Got allSeries from ExtractMagnetoSpeedSeries with size" << allSeries.size();

	if ( ! allSeries.empty() )
	{
		qDebug() << "Detected MagnetoSpeed file";

		for ( int i = 0; i < allSeries.size(); i++ )
		{
			ChronoSeries *series = allSeries.at(i);

			series->enabled = new QCheckBox();
			series->enabled->setChecked(true);

			series->chargeWeight = new QDoubleSpinBox();
			series->chargeWeight->setDecimals(2);
			series->chargeWeight->setSingleStep(0.1);
			series->chargeWeight->setMaximum(1000000);
			series->chargeWeight->setMinimumWidth(100);
			series->chargeWeight->setMaximumWidth(100);

			seriesData.append(series);
		}
	}

	csvFile.close();

	/* We're finished parsing the file */

	if ( seriesData.empty() )
	{
		qDebug() << "Didn't find any chrono data in this file, bail";

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Critical);
		msg->setText(QString("Unable to find MagnetoSpeed data in '%1'").arg(path));
		msg->setWindowTitle("Error");
		msg->exec();
	}
	else
	{
		qDebug() << "Detected MagnetoSpeed file" << path;

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Information);
		msg->setText(QString("Detected MagnetoSpeed data\n\nUsing '%1'").arg(path));
		msg->setWindowTitle("Success");
		msg->exec();

		// Proceed to display the data
		DisplaySeriesData();
	}
}

QList<ChronoSeries *> PowderTest::ExtractMagnetoSpeedSeries ( QTextStream &csv )
{
	// MagnetoSpeed XFR app exports .CSV files in a slightly different format
	bool xfr_export = false;

	QList<ChronoSeries *> allSeries;
	ChronoSeries *curSeries = new ChronoSeries();
	curSeries->isValid = false;
	curSeries->deleted = false;
	curSeries->seriesNum = -1;

	int i = 0;
	while ( ! csv.atEnd() )
	{
		QString line = csv.readLine();

		// MagnetoSpeed uses colon (,) as delimeter
		QStringList rows(line.split(","));

		// Trim whitespace from cells
		QMutableStringListIterator it(rows);
		while ( it.hasNext() )
		{
			it.next();
			it.setValue(it.value().trimmed());
		}

		qDebug() << "Line" << i << ":" << rows;

		if ( rows.size() > 0)
		{
			if ( rows.at(0).compare("----") == 0 )
			{
				bool useSeries = true;

				// Ensure we have a valid MagnetoSpeed series
				if ( ((! xfr_export) && (curSeries->seriesNum == -1)) || curSeries->velocityUnits.isNull() )
				{
					qDebug() << "Series does not have all expected fields set, skipping series.";
					useSeries = false;
				}

				if ( curSeries->muzzleVelocities.empty() )
				{
					qDebug() << "Series has no velocities. Likely deleted or empty, skipping series..";
					useSeries = false;
				}

				if ( useSeries )
				{
					// We have a valid series CSV
					curSeries->isValid = true;

					qDebug() << "Adding curSeries to allSeries";

					allSeries.append(curSeries);
				}

				curSeries = new ChronoSeries();
				curSeries->isValid = false;
				curSeries->deleted = false;
				curSeries->seriesNum = -1;
			}
			else if ( rows.at(0).compare("Synced on:") == 0 )
			{
				// .CSV file is exported from the MagnetoSpeed XFR app
				xfr_export = true;

				QStringList dateTime = rows.at(1).split(" ");
				if ( dateTime.size() == 2 )
				{
					curSeries->firstDate = dateTime.at(0);
					curSeries->firstTime = dateTime.at(1);
					qDebug() << "firstDate =" << curSeries->firstDate;
					qDebug() << "firstTime =" << curSeries->firstTime;
				}
				else
				{
					qDebug() << "Failed to split datetime cell:" << rows.at(1);
				}
			}
			else if ( (rows.at(0).compare("Series") == 0) && (rows.at(2) == "Shots:") )
			{
				bool ok;
				int seriesNum = rows.at(1).toInt(&ok);
				if ( ok )
				{
					// MagnetoSpeed V3 files contain an integer in the 'Series' field. Use it as the series name.
					curSeries->seriesNum = seriesNum;
					curSeries->name = new QLabel(QString("Series %1").arg(seriesNum));
					qDebug() << "seriesNum =" << curSeries->seriesNum;
				}
				else
				{
					// XFR export files contain a date in the 'Series' field. Ignore it since we're expecting to be replaced by the name in the 'Notes' field.
					qDebug() << "XFR file detected, skipping Series row";
				}
			}
			else if ( rows.at(0).compare("Notes") == 0 )
			{
				// Use the series name if the user entered one
				if ( rows.at(1).compare("") == 0 )
				{
					curSeries->name = new QLabel("Unnamed");
				}
				else
				{
					curSeries->name = new QLabel(rows.at(1));
				}

				qDebug() << "Setting name to '" << curSeries->name << "' via Notes field";
			}
			else
			{
				bool ok = false;

				// If the first cell is a valid integer, it's a velocity entry
				rows.at(0).toInt(&ok);
				if ( ok )
				{
					if ( xfr_export )
					{
						curSeries->muzzleVelocities.append(rows.at(1).toInt());
						qDebug() << "muzzleVelocities +=" << rows.at(1).toInt();

						if ( curSeries->muzzleVelocities.size() == 1 )
						{
							curSeries->velocityUnits = rows.at(2);
							qDebug() << "velocityUnits =" << curSeries->velocityUnits;
						}
					}
					else
					{
						curSeries->muzzleVelocities.append(rows.at(2).toInt());
						qDebug() << "muzzleVelocities +=" << rows.at(2).toInt();

						if ( curSeries->muzzleVelocities.size() == 1 )
						{
							curSeries->velocityUnits = rows.at(3);
							qDebug() << "velocityUnits =" << curSeries->velocityUnits;
						}
					}
				}
			}
		}

		i++;
	}

	// XFR export files do not include series numbers, so iterate through and set the seriesNum's
	for ( i = 0; i < allSeries.size(); i++ )
	{
		ChronoSeries *series = allSeries.at(i);
		series->seriesNum = i + 1;
		qDebug() << "Setting" << series->name << "to" << series->seriesNum;
	}

	return allSeries;
}

void PowderTest::selectProChronoFile ( bool state )
{
	qDebug() << "selectProChronoFile state =" << state;

	qDebug() << "Previous directory:" << prevProChronoDir;

	QString path = QFileDialog::getOpenFileName(this, "Select file", prevProChronoDir, "CSV files (*.csv)");
	prevProChronoDir = path;

	qDebug() << "Selected file:" << path;

	if ( path.isEmpty() )
	{
		qDebug() << "User didn't select a file, bail";
		return;
	}

	seriesData.clear();

	/*
	 * ProChrono records all of its series data in a single .CSV file
	 */

	QFile csvFile(path);
	csvFile.open(QIODevice::ReadOnly);
	QTextStream csv(&csvFile);

	// Test which format this ProChrono file is
	QString line = csv.readLine();
	csv.seek(0);

	QList<ChronoSeries *> allSeries;

	if ( line.startsWith("Shot 1") )
	{
		qDebug() << "Detected ProChrono format 2";
		allSeries = ExtractProChronoSeries_format2(csv);
	}
	else
	{
		qDebug() << "Detected ProChrono format 1";
		allSeries = ExtractProChronoSeries(csv);
	}

	qDebug() << "Got allSeries from ExtractProChronoSeries with size" << allSeries.size();

	if ( ! allSeries.empty() )
	{
		qDebug() << "Detected ProChrono file";

		for ( int i = 0; i < allSeries.size(); i++ )
		{
			ChronoSeries *series = allSeries.at(i);

			series->enabled = new QCheckBox();
			series->enabled->setChecked(true);

			series->chargeWeight = new QDoubleSpinBox();
			series->chargeWeight->setDecimals(2);
			series->chargeWeight->setSingleStep(0.1);
			series->chargeWeight->setMaximum(1000000);
			series->chargeWeight->setMinimumWidth(100);
			series->chargeWeight->setMaximumWidth(100);

			seriesData.append(series);
		}
	}

	csvFile.close();

	/* We're finished parsing the file */

	if ( seriesData.empty() )
	{
		qDebug() << "Didn't find any chrono data in this file, bail";

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Critical);
		msg->setText(QString("Unable to find ProChrono data in '%1'").arg(path));
		msg->setWindowTitle("Error");
		msg->exec();
	}
	else
	{
		qDebug() << "Detected ProChrono file" << path;

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Information);
		msg->setText(QString("Detected ProChrono data\n\nUsing '%1'").arg(path));
		msg->setWindowTitle("Success");
		msg->exec();

		// Proceed to display the data
		DisplaySeriesData();
	}
}

QList<ChronoSeries *> PowderTest::ExtractProChronoSeries ( QTextStream &csv )
{
	QList<ChronoSeries *> allSeries;
	ChronoSeries *curSeries = new ChronoSeries();
	curSeries->isValid = true;
	curSeries->deleted = false;
	curSeries->seriesNum = -1;
	curSeries->velocityUnits = "ft/s";

	/*
	 * ProChrono has two CSV formats, their legacy Digital USB format and a newer one exported by their Digital Link app:
	 *  - A Digital USB .CSV file contains only column headers followed by shot entries for one or more series.
	 *  - A Digital Link .CSV file contains multiple series and precedes each series with a header containing name/stats.
	 * We parse both in one below, for simplicity's sake for the user.
	 */

	int i = 0;
	while ( ! csv.atEnd() )
	{
		QString line = csv.readLine();

		// ProChrono uses comma (,) as delimeter
		QStringList rows(line.split(","));

		// Trim whitespace from cells
		QMutableStringListIterator it(rows);
		while ( it.hasNext() )
		{
			it.next();
			it.setValue(it.value().trimmed());
		}

		qDebug() << "Line" << i << ":" << rows;

		/*
		 * We can actually largely ignore the series name/stats headers in the Digital Link CSV. The only field we'd care
		 * about in the header is the series name, but the name is repeated in each shot entry row.
		 *
		 * To parse:
		 *  - If rows.size() >= 9 and row is not column headers, parse it as a shot entry.
		 *  - If cell 1 of the shot entry row contains (index) 1, create a new series with the name in cell 0.
		 *
		 * And that's it. It should get both file formats.
		 */

		if ( rows.size() >= 9 )
		{
			if ( rows.at(0) == "Shot List" )
			{
				// skip column headers
				qDebug() << "Skipping column headers";
			}
			else
			{
				bool ok = false;
				int index = rows.at(1).toInt(&ok);

				// If cell is a valid integer, the row is a shot entry
				if ( ok )
				{
					if ( index == 1 )
					{
						// First shot in the series. End the previous series (if necessary) and start a new one.

						if ( curSeries->muzzleVelocities.size() > 0 )
						{
							qDebug() << "Adding curSeries to allSeries";

							allSeries.append(curSeries);
						}

						qDebug() << "Beginning new series";

						curSeries = new ChronoSeries();
						curSeries->isValid = true;
						curSeries->deleted = false;
						curSeries->seriesNum = -1;
						curSeries->name = new QLabel(rows.at(0));
						curSeries->velocityUnits = "ft/s";
					}

					if ( curSeries->firstDate.isNull() )
					{
						QStringList dateTime = rows.at(8).split(" ");
						if ( dateTime.size() == 2 )
						{
							curSeries->firstDate = dateTime.at(0);
							curSeries->firstTime = dateTime.at(1);
							qDebug() << "firstDate =" << curSeries->firstDate;
							qDebug() << "firstTime =" << curSeries->firstTime;
						}
						else
						{
							qDebug() << "Failed to split datetime cell:" << rows.at(8);
						}
					}

					curSeries->muzzleVelocities.append(rows.at(2).toInt());
					qDebug() << "muzzleVelocities +=" << rows.at(2).toInt();
				}
			}
		}

		i++;
	}

	// End of the file. Finish parsing the current series.
	qDebug() << "End of file";

	if ( curSeries->muzzleVelocities.size() > 0 )
	{
		qDebug() << "Adding curSeries to allSeries";

		allSeries.append(curSeries);
	}

	// ProChrono files list series in reverse order from newest to oldest. Iterate through and
	// set the seriesNum's accordingly.
	int seriesNum = 1;
	for ( i = allSeries.size() - 1; i >= 0; i-- )
	{
		ChronoSeries *series = allSeries.at(i);
		series->seriesNum = seriesNum;
		seriesNum++;
	}

	return allSeries;
}

QList<ChronoSeries *> PowderTest::ExtractProChronoSeries_format2 ( QTextStream &csv )
{
	QList<ChronoSeries *> allSeries;
	ChronoSeries *curSeries = new ChronoSeries();
	curSeries->isValid = true;
	curSeries->deleted = false;
	curSeries->seriesNum = -1;
	curSeries->velocityUnits = "ft/s";

	/*
	 * Users have reported an alternate CSV format exported by Digital Link where
	 * shots are recorded as columns instead of rows
	 *
	 * To parse:
	 *  - If cell 0 is a number, the row is a velocity record. Create a new series, count the number of columns, and import each shot.
	 *  - Immediately on the next line, parse cell 0 as the first datetime.
	 */

	int i = 0;
	while ( ! csv.atEnd() )
	{
		QString line = csv.readLine();

		// ProChrono uses comma (,) as delimeter
		QStringList rows(line.split(","));

		// Trim whitespace from cells
		QMutableStringListIterator it(rows);
		while ( it.hasNext() )
		{
			it.next();
			it.setValue(it.value().trimmed());
		}

		qDebug() << "Line" << i << ":" << rows;

		if ( rows.at(0).contains("Shot") )
		{
			// skip column headers
			qDebug() << "Skipping column headers";
		}
		else
		{
			// If cell is a valid integer, parse the row as velocity data

			bool ok = false;
			rows.at(0).toInt(&ok);
			if ( ok )
			{
				// End the previous series (if necessary) and start a new one

				if ( curSeries->muzzleVelocities.size() > 0 )
				{
					qDebug() << "Adding curSeries to allSeries";

					allSeries.append(curSeries);
				}

				qDebug() << "Beginning new series";

				curSeries = new ChronoSeries();
				curSeries->isValid = true;
				curSeries->deleted = false;
				curSeries->seriesNum = -1;
				curSeries->velocityUnits = "ft/s";

				// Series in the file are recorded newest first. We'll iterate through and name them at the end.

				for ( int j = 0; j < rows.size(); j++ )
				{
					bool ok = false;
					int veloc = rows.at(j).toInt(&ok);

					if ( ok )
					{
						curSeries->muzzleVelocities.append(rows.at(j).toInt());
						qDebug() << "muzzleVelocities +=" << rows.at(j).toInt();
					}
					else
					{
						qDebug() << "Skipping velocity entry:" << rows.at(j);
					}
				}
			}
			else
			{
				QDateTime seriesDateTime;
				seriesDateTime = QDateTime::fromString(rows.at(0), "M/d/yyyy hh:mm:ss");
				if ( seriesDateTime.isValid() )
				{
					QStringList dateTime = rows.at(0).split(" ");
					if ( dateTime.size() == 2 )
					{
						curSeries->firstDate = dateTime.at(0);
						curSeries->firstTime = dateTime.at(1);
						qDebug() << "firstDate =" << curSeries->firstDate;
						qDebug() << "firstTime =" << curSeries->firstTime;
					}
				}
			}
		}

		i++;
	}

	// End of the file. Finish parsing the current series.
	qDebug() << "End of file";

	if ( curSeries->muzzleVelocities.size() > 0 )
	{
		qDebug() << "Adding curSeries to allSeries";

		allSeries.append(curSeries);
	}

	// ProChrono files list series in reverse order from newest to oldest. Iterate through and
	// set the seriesNum's and names accordingly.
	int seriesNum = 1;
	for ( i = allSeries.size() - 1; i >= 0; i-- )
	{
		ChronoSeries *series = allSeries.at(i);
		series->seriesNum = seriesNum;
		series->name = new QLabel(QString("Series %1").arg(seriesNum));
		seriesNum++;
	}

	return allSeries;
}

void PowderTest::selectShotMarkerFile ( bool state )
{
	qDebug() << "selectShotMarkerFile state =" << state;

	qDebug() << "Previous directory:" << prevShotMarkerDir;

	QString path = QFileDialog::getOpenFileName(this, "Select file", prevShotMarkerDir, "ShotMarker files (*.tar)");
	prevShotMarkerDir = path;

	qDebug() << "Selected file:" << path;

	if ( path.isEmpty() )
	{
		qDebug() << "User didn't select a file, bail";
		return;
	}

	seriesData.clear();

	/*
	 * ShotMarker only records velocity data in .tar export files
	 */

	QList<ChronoSeries *> allSeries;

	if ( path.endsWith(".tar") )
	{
		qDebug() << "ShotMarker .tar bundle";
	
		allSeries = ExtractShotMarkerSeriesTar(path);
	}
	else
	{
		qDebug() << "ShotMarker .csv export, bailing";

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Critical);
		msg->setText(QString("Only ShotMarker .tar files are supported for velocity data.\n\nSelected: '%1'").arg(path));
		msg->setWindowTitle("Error");
		msg->exec();

		return;
	}

	qDebug() << "Got allSeries with size" << allSeries.size();

	if ( ! allSeries.empty() )
	{
		qDebug() << "Detected ShotMarker file";

		for ( int i = 0; i < allSeries.size(); i++ )
		{
			ChronoSeries *series = allSeries.at(i);

			series->enabled = new QCheckBox();
			series->enabled->setChecked(true);

			series->chargeWeight = new QDoubleSpinBox();
			series->chargeWeight->setDecimals(2);
			series->chargeWeight->setSingleStep(0.1);
			series->chargeWeight->setMaximum(1000000);
			series->chargeWeight->setMinimumWidth(100);
			series->chargeWeight->setMaximumWidth(100);

			seriesData.append(series);
		}
	}

	/* We're finished parsing the file */

	if ( seriesData.empty() )
	{
		qDebug() << "Didn't find any shot data in this file, bail";

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Critical);
		msg->setText(QString("Unable to find ShotMarker data in '%1'").arg(path));
		msg->setWindowTitle("Error");
		msg->exec();
	}
	else
	{
		qDebug() << "Detected ShotMarker file" << path;

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Information);
		msg->setText(QString("Detected ShotMarker data\n\nUsing '%1'").arg(path));
		msg->setWindowTitle("Success");
		msg->exec();

		// Proceed to display the data
		DisplaySeriesData();
	}
}

QList<ChronoSeries *> PowderTest::ExtractShotMarkerSeriesTar ( QString path )
{
	QList<ChronoSeries *> allSeries;
	ChronoSeries *curSeries = new ChronoSeries();
	QTemporaryDir tempDir;
	int ret;

	if ( ! tempDir.isValid() )
	{
		qDebug() << "Temp directory is NOT valid";
	}

	qDebug() << "Temporary directory:" << tempDir.path();

	/*
	 * ShotMarker .tar files contain one .z file for each string being exported.
	 * Each .z file is a zlib-compressed JSON file containing shot data for that string.
	 */

	QFile rf(path);
	if ( ! rf.open(QIODevice::ReadOnly) )
	{
		qDebug() << "Failed to open ShotMarker .tar file:" << path;
		return allSeries;
	}

	ret = untar(rf, tempDir.path());
	if ( ret )
	{
		qDebug() << "Error while extracting ShotMarker .tar file:" << path;
		return allSeries;
	}

	/*
	 * We have a temp directory with our extracted files in it. Now iterate over each .z
	 * file and decompress it.
	 */

	QDir dir(tempDir.path());
	QStringList stringFiles = dir.entryList(QStringList() << "*.z", QDir::Files);

	qDebug() << "iterating over files:";
	int seriesNum = 1;
	foreach ( QString filename, stringFiles )
	{
		QString path(tempDir.path());
		path.append("/");
		path.append(filename);
		qDebug() << path;


		QFile file(path);
		file.open(QIODevice::ReadOnly);
		QByteArray buf = file.readAll();
		qDebug() << "file:" << path << ", size:" << buf.size();

		// 1mb ought to be enough for anybody!
		unsigned char *destBuf = (unsigned char *)malloc(1024 * 1024);
		qDebug() << "malloc returned" << (void *)destBuf;
		if ( destBuf == NULL )
		{
			qDebug() << "malloc returned NULL! skipping... but we should really throw an exception here.";
			continue;
		}

		mz_ulong uncomp_len = 1024 * 1024;
		qDebug() << "calling uncompress 1 with destBuf=" << (void *)destBuf << ", uncomp_len=" << uncomp_len << ", buf.data()=" << buf.data() << ", buf.size()=" << buf.size();
		ret = uncompress(destBuf, &uncomp_len, (const unsigned char *)buf.data(), buf.size());

		qDebug() << "output size:" << uncomp_len;
		if ( ret != MZ_OK )
		{
			qDebug() << "Failed to uncompress, skipping..." << path;
			free(destBuf);
			continue;
		}

		//qDebug() << "calling uncompress 2 with destBuf=" << (void *)destBuf << ", uncomp_len=" << uncomp_len << ", buf.data()=" << buf.data() << ", buf.size()=" << buf.size();
		//uncompress(destBuf, &uncomp_len, (const unsigned char *)buf.data(), buf.size());
		QByteArray ba = QByteArray::fromRawData((const char *)destBuf, uncomp_len);
		//qDebug() << "decompressed" << uncomp_len << ":" << ba;

		QJsonParseError parseError;
		QJsonDocument jsonDoc;
		jsonDoc = QJsonDocument::fromJson(ba, &parseError);

		if ( parseError.error != QJsonParseError::NoError )
		{
			qDebug() << "JSON parse error, skipping... at" << parseError.offset << ":" << parseError.errorString();
			free(destBuf);
			continue;
		}

		QJsonObject jsonObj = jsonDoc.object();

		/* We have a JSON file containing a single series */

		qDebug() << "Beginning new series";

		curSeries = new ChronoSeries();
		curSeries->isValid = false;
		curSeries->seriesNum = seriesNum;
		qDebug() << "name =" << jsonObj["name"].toString();
		curSeries->name = new QLabel(jsonObj["name"].toString());
		curSeries->velocityUnits = "ft/s";
		curSeries->deleted = false;
		QDateTime dateTime;
		dateTime.setMSecsSinceEpoch(jsonObj["ts"].toVariant().toULongLong());
		curSeries->firstDate = dateTime.date().toString(Qt::TextDate);
		curSeries->firstTime = dateTime.time().toString(Qt::TextDate);

		qDebug() << "setting date =" << curSeries->firstDate << " time =" << curSeries->firstTime << "from ts" << jsonObj["ts"].toVariant().toULongLong();

		foreach ( const QJsonValue& shot, jsonObj["shots"].toArray() )
		{
			// convert from m/s to ft/s
			int velocity = shot["v"].toDouble() * 1.0936133 * 3; // the result is cast to an int

			if ( shot["hidden"].toBool() )
			{
				// hidden shot

				qDebug() << "ignoring hidden shot" << shot["display_text"];
			}
			else if ( shot["sighter"].toBool() )
			{
				// sighter shot

				qDebug() << "ignoring sighter shot" << shot["display_text"];
			}
			else
			{
				// shot for record

				qDebug() << "adding velocity" << velocity << "from m/s:" << shot["v"].toDouble();
				curSeries->muzzleVelocities.append(velocity);
			}

		}

		// Finish parsing the current series.
		qDebug() << "End of JSON";

		if ( curSeries->muzzleVelocities.size() > 0 )
		{
			qDebug() << "Adding curSeries to allSeries";

			allSeries.append(curSeries);
		}

		free(destBuf);

		seriesNum++;
	}

	return allSeries;
}

void PowderTest::selectGarminFile ( bool state )
{
	qDebug() << "selectGarminFile state =" << state;

	qDebug() << "Previous directory:" << prevGarminDir;

	QString path = QFileDialog::getOpenFileName(this, "Select file", prevGarminDir, "Garmin files (*.xlsx *.csv)");
	prevGarminDir = path;

	qDebug() << "Selected file:" << path;

	if ( path.isEmpty() )
	{
		qDebug() << "User didn't select a file, bail";
		return;
	}

	seriesData.clear();

	/*
	 * Garmin Xero C1 records its series data as CSV, XLSX, or FIT files. Garmin, seriously why is this such a mess.
	 * CSV files contain a single series.
	 * XLSX files contain one series per worksheet.
	 * We don't currently support FIT files.
	 */

	QList<ChronoSeries *> allSeries;

	if ( path.endsWith(".xlsx", Qt::CaseInsensitive) )
	{
		qDebug() << "Garmin XLSX file";
	
		QXlsx::Document xlsx(path);
		xlsx.load();
		
		qDebug() << "Loaded xlsx doc. sheets: " << xlsx.sheetNames();
		
		allSeries = ExtractGarminSeries_xlsx(xlsx);
	}
	else if ( path.endsWith(".csv", Qt::CaseInsensitive) )
	{
		qDebug() << "Garmin CSV file";
	
		QFile csvFile(path);
		csvFile.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream csv(&csvFile);
		
		allSeries = ExtractGarminSeries_csv(csv);
	}
	else
	{
		qDebug() << "Garmin unsupported file, bailing...";

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Critical);
		msg->setText(QString("Only Garmin .XLSX and .CSV files are supported.\n\nSelected: '%1'").arg(path));
		msg->setWindowTitle("Error");
		msg->exec();

		return;
	}

	qDebug() << "Got allSeries with size" << allSeries.size();

	if ( ! allSeries.empty() )
	{
		qDebug() << "Detected Garmin file";

		for ( int i = 0; i < allSeries.size(); i++ )
		{
			ChronoSeries *series = allSeries.at(i);

			series->enabled = new QCheckBox();
			series->enabled->setChecked(true);

			series->chargeWeight = new QDoubleSpinBox();
			series->chargeWeight->setDecimals(2);
			series->chargeWeight->setSingleStep(0.1);
			series->chargeWeight->setMaximum(1000000);
			series->chargeWeight->setMinimumWidth(100);
			series->chargeWeight->setMaximumWidth(100);

			seriesData.append(series);
		}
	}

	/* We're finished parsing the file */

	if ( seriesData.empty() )
	{
		qDebug() << "Didn't find any chrono data in this file, bail";

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Critical);
		msg->setText(QString("Unable to find Garmin data in '%1'").arg(path));
		msg->setWindowTitle("Error");
		msg->exec();
	}
	else
	{
		qDebug() << "Detected Garmin file" << path;

		QMessageBox *msg = new QMessageBox();
		msg->setIcon(QMessageBox::Information);
		msg->setText(QString("Detected Garmin data\n\nUsing '%1'").arg(path));
		msg->setWindowTitle("Success");
		msg->exec();

		// Proceed to display the data
		DisplaySeriesData();
	}

}

QList<ChronoSeries *> PowderTest::ExtractGarminSeries_xlsx ( QXlsx::Document &xlsx )
{
	QList<ChronoSeries *> allSeries;
	
	/*
	 * for worksheet in XLSX file:
	 *  set active sheet
	 *  create curSeries object
	 *  for row in sheet:
	 *    add velocity to curSeries
	 */
	 
	int i = 0;
	foreach ( QString sheetName, xlsx.sheetNames() )
	{
		qDebug() << "Sheet: " << sheetName;
		QXlsx::AbstractSheet *curSheet = xlsx.sheet(sheetName);
		if ( curSheet == NULL )
		{
			qDebug() << "Failed to get sheet, skipping...";
			continue;
		}
		
		curSheet->workbook()->setActiveSheet(i);
		
		QXlsx::Worksheet *worksheet = (QXlsx::Worksheet *)curSheet->workbook()->activeSheet();
		if ( worksheet == NULL )
		{
			qDebug() << "Failed to set active sheet, skipping...";
			continue;
		}
		
		ChronoSeries *curSeries = new ChronoSeries();
		curSeries->isValid = false;
		curSeries->deleted = false;
		curSeries->seriesNum = i + 1;
		curSeries->firstDate = QString("-");
		curSeries->firstTime = QString("");
		
		qDebug() << "Series name:" << worksheet->read(1,1).toString();
		curSeries->name = new QLabel(worksheet->read(1, 1).toString());
		
		// Unit of measure
		if ( worksheet->read(2, 2).toString().contains("FPS") )
		{
			curSeries->velocityUnits = "ft/s";
		}
		else
		{
			curSeries->velocityUnits = "m/s";
		}
		
		int maxRow = 0, maxCol = 0;
		worksheet->getFullCells(&maxRow, &maxCol);
		
		// Iterate through each row of the worksheet, looking for velocities
		int j;
		for ( j = 3; j <= maxRow; j++ )
		{
			bool ok_shot_id = false;
			int shot_id = worksheet->read(j, 1).toInt(&ok_shot_id);
			if ( ok_shot_id )
			{
				// We found a row with an integer (shot ID) in the first column
				
				bool ok_veloc = false;
				double veloc = worksheet->read(j, 2).toFloat(&ok_veloc);
				if ( ok_veloc )
				{
					curSeries->muzzleVelocities.append(veloc);
					qDebug() << "muzzleVelocities +=" << veloc;
				}
				else
				{
					qDebug() << "Skipping velocity entry:" << worksheet->read(j, 2);
				}
			}
			else
			{
				if ( worksheet->read(j, 1).toString().compare("DATE") == 0 )
				{
					// Date time
					QStringList dateTime = worksheet->read(j, 2).toString().split(" at ");
					if ( dateTime.size() == 2 )
					{
						curSeries->firstDate = dateTime.at(0);
						curSeries->firstTime = dateTime.at(1);
						qDebug() << "firstDate =" << curSeries->firstDate;
						qDebug() << "firstTime =" << curSeries->firstTime;
					}
					else
					{
						qDebug() << "Failed to split datetime cell:" << worksheet->read(j, 2);
					}
				}
			}
		}
		
		// We have a valid series CSV
		curSeries->isValid = true;

		qDebug() << "Adding curSeries to allSeries";
		qDebug() << "";

		allSeries.append(curSeries);
		
		i += 1;
	}

	return allSeries;
}

// https://stackoverflow.com/a/40229435
bool readCSVRow (QTextStream &in, QStringList *row) {

    static const int delta[][5] = {
        //  ,    "   \n    ?  eof
        {   1,   2,  -1,   0,  -1  }, // 0: parsing (store char)
        {   1,   2,  -1,   0,  -1  }, // 1: parsing (store column)
        {   3,   4,   3,   3,  -2  }, // 2: quote entered (no-op)
        {   3,   4,   3,   3,  -2  }, // 3: parsing inside quotes (store char)
        {   1,   3,  -1,   0,  -1  }, // 4: quote exited (no-op)
        // -1: end of row, store column, success
        // -2: eof inside quotes
    };

    row->clear();

    if (in.atEnd())
        return false;

    int state = 0, t;
    char ch;
    QString cell;

    while (state >= 0) {

        if (in.atEnd())
            t = 4;
        else {
            in >> ch;
            if (ch == ',') t = 0;
            else if (ch == '\"') t = 1;
            else if (ch == '\n') t = 2;
            else t = 3;
        }

        state = delta[state][t];

        if (state == 0 || state == 3) {
            cell += ch;
        } else if (state == -1 || state == 1) {
            row->append(cell);
            cell = "";
        }

    }

    if (state == -2)
	{
        qDebug() << "End-of-file found while inside quotes.";
		return false;
	}

    return true;

}

QList<ChronoSeries *> PowderTest::ExtractGarminSeries_csv ( QTextStream &csv )
{
	QList<ChronoSeries *> allSeries;
	ChronoSeries *curSeries = new ChronoSeries();
	curSeries->isValid = false;
	curSeries->deleted = false;
	curSeries->seriesNum = 1;
	curSeries->velocityUnits = "ft/s";
	curSeries->firstDate = QString("-");
	curSeries->firstTime = QString("");

	int i = 0;
	QStringList cols;
	while ( readCSVRow(csv, &cols) )
	{
		// Trim whitespace from cells
		QMutableStringListIterator it(cols);
		while ( it.hasNext() )
		{
			it.next();
			it.setValue(it.value().trimmed());
		}

		qDebug() << "Line" << i << ":" << cols;

		if ( cols.size() >= 1 )
		{
			// Series name in first row, first column
			if ( i == 0 )
			{
				qDebug() << "Series name:" << cols.at(0);
				curSeries->name = new QLabel(cols.at(0));
			}
			// Unit of measure in second row, second column
			else if ( i == 1 )
			{
				if ( cols.at(1).contains("FPS") )
				{
					qDebug() << "Velocity units: ft/s";
					curSeries->velocityUnits = "ft/s";
				}
				else
				{
					qDebug() << "Velocity units: m/s";
					curSeries->velocityUnits = "m/s";
				}
			}
			// Date time
			else if ( cols.at(0).compare("DATE") == 0 )
			{
				curSeries->firstDate = cols.at(1);
				curSeries->firstTime = QString("");
				qDebug() << "firstDate =" << curSeries->firstDate;
				qDebug() << "firstTime =" << curSeries->firstTime;
			}
			// Look for shot velocity row
			else
			{
				bool ok_shot_id = false;
				int shot_id = cols.at(0).toInt(&ok_shot_id);
				if ( ok_shot_id )
				{
					// We found a row with an integer (shot ID) in the first column
					
					bool ok_veloc = false;
					QString veloc_str = cols.at(1);
					veloc_str.replace(",", "."); // handle international-formatted numbers
					double veloc = veloc_str.toFloat(&ok_veloc);
					if ( ok_veloc )
					{
						curSeries->muzzleVelocities.append(veloc);
						qDebug() << "muzzleVelocities +=" << veloc;
					}
					else
					{
						qDebug() << "Skipping velocity entry:" << cols.at(1);
					}
				}
			}
		}

		i++;
	}

	// End of the file. Finish parsing the current series.
	qDebug() << "End of file";
	
	// Ensure we have a valid Garmin series
	if ( curSeries->muzzleVelocities.empty() )
	{
		qDebug() << "Series has no velocities, returning invalid.";
		return allSeries;
	}

	allSeries.append(curSeries);

	return allSeries;
}

RoundRobinDialog::RoundRobinDialog ( PowderTest *main, QDialog *parent )
	: QDialog(parent)
{
	qDebug() << "Round-robin dialog";

	setWindowTitle("Convert from round-robin");

	QLabel *label = new QLabel();
	label->setTextFormat(Qt::RichText);
	label->setText("<p>This feature handles chronograph data recorded using the \"round-robin\" method popular with <a href=\"http://www.ocwreloading.com/\">OCW testing</a>.<p>For example a shooter might record three chronograph series, where each series contains 10 shots with 10 different charge weights. Use this feature to \"convert\" the data back into 10 series of three-shot strings.<p>Data recorded using the <a href=\"http://www.65guys.com/10-round-load-development-ladder-test/\">Satterlee method</a> can be converted by using this feature with only a single series enabled (rather than multiple).<p>Note: This will <i>not</i> alter your CSV files, this only converts the data loaded in ChronoPlotter. If converting multiple series, it's assumed that charge weights are shot in the same order in each series.<br>");
	label->setOpenExternalLinks(true);
	label->setWordWrap(true);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(label);
	layout->addWidget(new QHLine());

	QList<QList<double> > seriesVelocs;
	for ( int i = 0; i < main->seriesData.size(); i++ )
	{
		ChronoSeries *series = main->seriesData.at(i);
		if ( series->enabled->isChecked() )
		{
			seriesVelocs.append(series->muzzleVelocities);
		}
	}

	qDebug() << "Number of enabled series:" << seriesVelocs.size();
	qDebug() << "seriesVelocs:" << seriesVelocs;

	QList<int> numVelocs;
	for ( int i = 0; i < seriesVelocs.size(); i++ )
	{
		numVelocs.append(seriesVelocs.at(i).size());
	}

	qDebug() << "numVelocs:" << numVelocs;

	bool equalLens = std::equal(numVelocs.begin() + 1, numVelocs.end(), numVelocs.begin());

	qDebug() << "equalLens:" << equalLens;

	QLabel *detected = new QLabel();
	QDialogButtonBox *buttonBox;

	if ( equalLens )
	{
		detected->setText(QString("<center><br>Detected <b>%1</b> enabled series of <b>%2</b> shots each.<p>Click <b>OK</b> to convert this data into <b>%3</b> series of <b>%4</b> shots each.<br>").arg(seriesVelocs.size()).arg(numVelocs.at(0)).arg(numVelocs.at(0)).arg(seriesVelocs.size()));

		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &RoundRobinDialog::accept);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &RoundRobinDialog::reject);

	}
	else
	{
		detected->setText(QString("<center><br>Detected <b>%1</b> enabled series of <b>different</b> lengths.<p>Series with the same number of shots are required to convert.<br>").arg(seriesVelocs.size()));

		buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &RoundRobinDialog::reject);
	}

	detected->setTextFormat(Qt::RichText);
	detected->setWordWrap(true);
	layout->addWidget(detected);
	layout->addWidget(buttonBox);
	setLayout(layout);

	setFixedSize(sizeHint());
}

void PowderTest::rrClicked ( bool state )
{
	qDebug() << "rrClicked state =" << state;

	RoundRobinDialog *dialog = new RoundRobinDialog(this);
	int result = dialog->exec();

	qDebug() << "dialog result:" << result;

	if ( result )
	{
		qDebug() << "Performing series conversion";

		QList<QList<double> > enabledSeriesVelocs;
		for ( int i = 0; i < seriesData.size(); i++ )
		{
			ChronoSeries *series = seriesData.at(i);
			if ( series->enabled->isChecked() )
			{
				enabledSeriesVelocs.append(series->muzzleVelocities);
			}
		}

		// Iterate the number of velocities in the enabled series
		QList<ChronoSeries *> newSeriesData;
		for ( int i = 0; i < enabledSeriesVelocs.at(0).size(); i++ )
		{
			// Grab the Xth velocity in each of the enabled series and create a new series with them
			QList<double> newVelocs;
			for ( int j = 0; j < enabledSeriesVelocs.size(); j++ )
			{
				newVelocs.append(enabledSeriesVelocs.at(j).at(i));
			}

			ChronoSeries *newSeries = new ChronoSeries();

			newSeries->seriesNum = i;

			newSeries->name = new QLabel(QString("Series %1").arg(i + 1));

			newSeries->muzzleVelocities = newVelocs;

			newSeries->enabled = new QCheckBox();
			newSeries->enabled->setChecked(true);

			newSeries->chargeWeight = new QDoubleSpinBox();
			newSeries->chargeWeight->setDecimals(2);
			newSeries->chargeWeight->setSingleStep(0.1);
			newSeries->chargeWeight->setMaximum(1000000);
			newSeries->chargeWeight->setMinimumWidth(100);
			newSeries->chargeWeight->setMaximumWidth(100);

			newSeriesData.append(newSeries);
		}

		// Replace the current series data with the new one
		seriesData = newSeriesData;

		// Proceed to display the data
		DisplaySeriesData();
	}
	else
	{
		qDebug() << "Not performing series conversion";
	}
}

AutofillDialog::AutofillDialog ( PowderTest *main, QDialog *parent )
	: QDialog(parent)
{
	qDebug() << "Autofill dialog";

	setWindowTitle("Auto-fill charge weights");

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &AutofillDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &AutofillDialog::reject);

	QLabel *weightUnitsLabel;
	if ( main->weightUnits->currentIndex() == GRAINS )
	{
		weightUnitsLabel = new QLabel("gr");
	}
	else
	{
		weightUnitsLabel = new QLabel("g");
	}

	QFormLayout *formLayout = new QFormLayout();

	startingCharge = new QDoubleSpinBox();
	startingCharge->setDecimals(2);
	startingCharge->setSingleStep(0.1);
	startingCharge->setMaximum(1000000);
	startingCharge->setMinimumWidth(100);
	startingCharge->setMaximumWidth(100);

	QHBoxLayout *startingChargeLayout = new QHBoxLayout();
	startingChargeLayout->addWidget(startingCharge);
	startingChargeLayout->addWidget(weightUnitsLabel);

	formLayout->addRow(new QLabel("Starting value:"), startingChargeLayout);

	interval = new QDoubleSpinBox();
	interval->setDecimals(2);
	interval->setSingleStep(0.1);
	interval->setMaximum(1000000);
	interval->setMinimumWidth(100);
	interval->setMaximumWidth(100);

	QHBoxLayout *intervalLayout = new QHBoxLayout();
	intervalLayout->addWidget(interval);
	intervalLayout->addWidget(new QLabel(weightUnitsLabel));

	formLayout->addRow(new QLabel("Interval:"), intervalLayout);

	direction = new QComboBox();
	direction->addItem("Values increasing", QVariant(true));
	direction->addItem("Values decreasing", QVariant(false));
	direction->resize(direction->sizeHint());
	direction->setFixedWidth(direction->sizeHint().width());

	formLayout->addRow(new QLabel("Direction:"), direction);

	QLabel *label = new QLabel("Automatically fill in charge weights for all enabled series, starting from the top of the list.\n");
	label->setWordWrap(true);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(label);
	layout->addLayout(formLayout);
	layout->addWidget(direction);
	layout->addWidget(buttonBox);

	setLayout(layout);
	setFixedSize(sizeHint());
}

AutofillValues *AutofillDialog::getValues ( void )
{
	AutofillValues *values = new AutofillValues();
	values->startingCharge = startingCharge->value();
	values->interval = interval->value();
	values->increasing = direction->currentData().value<bool>();

	return values;
}

void PowderTest::autofillClicked ( bool state )
{
	qDebug() << "autofillClicked state =" << state;

	AutofillDialog *dialog = new AutofillDialog(this);
	int result = dialog->exec();

	qDebug() << "dialog result:" << result;

	if ( result )
	{
		qDebug() << "User OK'd dialog";

		AutofillValues *values = dialog->getValues();

		double currentCharge = values->startingCharge;

		for ( int i = 0; i < seriesData.size(); i++ )
		{
			ChronoSeries *series = seriesData.at(i);
			if ( (! series->deleted) && series->enabled->isChecked() )
			{
				qDebug() << "Setting series" << i << "to" << currentCharge;
				series->chargeWeight->setValue(currentCharge);
				if ( values->increasing )
				{
					currentCharge += values->interval;
				}
				else
				{
					currentCharge -= values->interval;
				}
			}
		}
	}
	else
	{
		qDebug() << "User cancelled dialog";
	}
}
