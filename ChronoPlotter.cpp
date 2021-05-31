#include <sstream>
#include <cmath>
#include <Qt>
#include <QApplication>
#include <QDebug>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QTabBar>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QFrame>
#include <QCheckBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QFileDialog>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QIODevice>
#include <QMessageBox>
#include <QList>
#include <QDialogButtonBox>
#include <QDialog>
#include <QByteArray>
#include "qcustomplot/qcustomplot.h"

#include "ChronoPlotter.h"

int scaleFontSize ( int size )
{
	#if __APPLE__
	return size * 1.3;
	#else
	return size;
	#endif
}

QHLine::QHLine ( QFrame *parent )
	: QFrame(parent)
{
	setFrameShape(QFrame::HLine);
	setFrameShadow(QFrame::Sunken);
}

GraphPreview::GraphPreview ( QPixmap& image, QWidget *parent )
	: QWidget(parent)
{
	label = new QLabel(this);
	label->setPixmap(image);
	label->setScaledContents(true);
	setGeometry(300, 300, label->sizeHint().width() / 2, label->sizeHint().height() / 2);
	setWindowTitle("Graph preview");
	show();
}

void GraphPreview::resizeEvent ( QResizeEvent *event )
{
	qDebug() << "resizeEvent" << event;
	label->setFixedSize(event->size());
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

	QList<QList<int> > seriesVelocs;
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
	startingCharge->setMinimumWidth(100);
	startingCharge->setMaximumWidth(100);

	QHBoxLayout *startingChargeLayout = new QHBoxLayout();
	startingChargeLayout->addWidget(startingCharge);
	startingChargeLayout->addWidget(weightUnitsLabel);

	formLayout->addRow(new QLabel("Starting value:"), startingChargeLayout);

	interval = new QDoubleSpinBox();
	interval->setDecimals(2);
	interval->setSingleStep(0.1);
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

AutofillSeatingDialog::AutofillSeatingDialog ( SeatingDepthTest *main, QDialog *parent )
	: QDialog(parent)
{
	qDebug() << "Autofill dialog";

	setWindowTitle("Auto-fill cartridge lengths");

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &AutofillSeatingDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &AutofillSeatingDialog::reject);

	QLabel *lengthUnitsLabel;
	if ( main->cartridgeUnits->currentIndex() == INCH )
	{
		lengthUnitsLabel = new QLabel("in");
	}
	else
	{
		lengthUnitsLabel = new QLabel("cm");
	}

	QFormLayout *formLayout = new QFormLayout();

	startingLength = new QDoubleSpinBox();
	startingLength->setDecimals(3);
	startingLength->setSingleStep(0.001);
	startingLength->setMinimumWidth(100);
	startingLength->setMaximumWidth(100);

	QHBoxLayout *startingLengthLayout = new QHBoxLayout();
	startingLengthLayout->addWidget(startingLength);
	startingLengthLayout->addWidget(lengthUnitsLabel);

	formLayout->addRow(new QLabel("Starting value:"), startingLengthLayout);

	interval = new QDoubleSpinBox();
	interval->setDecimals(3);
	interval->setSingleStep(0.001);
	interval->setMinimumWidth(100);
	interval->setMaximumWidth(100);

	QHBoxLayout *intervalLayout = new QHBoxLayout();
	intervalLayout->addWidget(interval);
	intervalLayout->addWidget(new QLabel(lengthUnitsLabel));

	formLayout->addRow(new QLabel("Interval:"), intervalLayout);

	direction = new QComboBox();
	direction->addItem("Values increasing", QVariant(true));
	direction->addItem("Values decreasing", QVariant(false));
	direction->resize(direction->sizeHint());
	direction->setFixedWidth(direction->sizeHint().width());

	formLayout->addRow(new QLabel("Direction:"), direction);

	QLabel *label = new QLabel("Automatically fill in cartridge lengths for all enabled series, starting from the top of the list.\n");
	label->setWordWrap(true);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(label);
	layout->addLayout(formLayout);
	layout->addWidget(direction);
	layout->addWidget(buttonBox);

	setLayout(layout);
	setFixedSize(sizeHint());
}

AutofillSeatingValues *AutofillSeatingDialog::getValues ( void )
{
	AutofillSeatingValues *values = new AutofillSeatingValues();
	values->startingLength = startingLength->value();
	values->interval = interval->value();
	values->increasing = direction->currentData().value<bool>();

	return values;
}

PowderTest::PowderTest ( QWidget *parent )
	: QWidget(parent)
{
	qDebug() << "Powder test";

	graphPreview = NULL;
	prevLabRadarDir = QDir::homePath();
	prevMagnetoSpeedDir = QDir::homePath();
	prevSaveDir = QDir::homePath();
	utilitiesDisplayed = false;

	/* Left panel */

	QVBoxLayout *leftLayout = new QVBoxLayout();

	QLabel *selectLabel = new QLabel("Select LabRadar or MagnetoSpeed\nto populate series data\n");
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

	QVBoxLayout *placeholderLayout = new QVBoxLayout();
	placeholderLayout->addStretch(0);
	placeholderLayout->addWidget(selectLabel);
	placeholderLayout->setAlignment(selectLabel, Qt::AlignCenter);
	placeholderLayout->addWidget(lrDirButton);
	placeholderLayout->setAlignment(lrDirButton, Qt::AlignCenter);
	placeholderLayout->addWidget(msFileButton);
	placeholderLayout->setAlignment(msFileButton, Qt::AlignCenter);
	placeholderLayout->addStretch(0);

	QWidget *placeholderWidget = new QWidget();
	placeholderWidget->setLayout(placeholderLayout);

	stackedWidget = new QStackedWidget();
	stackedWidget->addWidget(placeholderWidget);
	stackedWidget->setCurrentIndex(0);

	scrollLayout = new QVBoxLayout();
	scrollLayout->addWidget(stackedWidget);
	QGroupBox *chronoGroupBox = new QGroupBox("Chronograph data:");
	chronoGroupBox->setLayout(scrollLayout);

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
	velocityUnits->addItem("feet per second (fps)");
	velocityUnits->addItem("meters per second (m/s)");
	optionsFormLayout->addRow(new QLabel("Velocity units:"), velocityUnits);

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
	avgLabel->setStyleSheet("color: #878787");
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
	trendLabel->setStyleSheet("color: #878787");
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

	// If we already have series data displayed, clear it out first. This call is a no-op if scrollArea is not already added to stackedWidget.
	stackedWidget->removeWidget(scrollArea);

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

	stackedWidget->addWidget(scrollArea);
	stackedWidget->setCurrentWidget(scrollArea);

	// Now that we've hidden the placeholder text + buttons, reveal the utilities buttons under the scroll area
	if ( ! utilitiesDisplayed )
	{
		// Create hidden buttons under the scroll area. They'll be revealed once the left panel is replaced with chrono series data.

		QPushButton *lrDirButton2 = new QPushButton("Select LabRadar dir");
		connect(lrDirButton2, SIGNAL(clicked(bool)), this, SLOT(selectLabRadarDirectory(bool)));
		lrDirButton2->setMinimumWidth(225);
		lrDirButton2->setMaximumWidth(225);

		QPushButton *msDirButton2 = new QPushButton("Select MagnetoSpeed file");
		connect(msDirButton2, SIGNAL(clicked(bool)), this, SLOT(selectMagnetoSpeedFile(bool)));
		msDirButton2->setMinimumWidth(225);
		msDirButton2->setMaximumWidth(225);

		QPushButton *rrButton = new QPushButton("Convert from round-robin");
		connect(rrButton, SIGNAL(clicked(bool)), this, SLOT(rrClicked(bool)));
		rrButton->setMinimumWidth(225);
		rrButton->setMaximumWidth(225);

		QPushButton *autofillButton = new QPushButton("Auto-fill charge weights");
		connect(autofillButton, SIGNAL(clicked(bool)), this, SLOT(autofillClicked(bool)));
		autofillButton->setMinimumWidth(225);
		autofillButton->setMaximumWidth(225);

		QHBoxLayout *utilitiesLayout = new QHBoxLayout();
		utilitiesLayout->addWidget(lrDirButton2);
		utilitiesLayout->addWidget(msDirButton2);
		utilitiesLayout->addWidget(rrButton);
		utilitiesLayout->addWidget(autofillButton);

		scrollLayout->addLayout(utilitiesLayout);
		utilitiesDisplayed = true;
	}

	QCheckBox *headerCheckBox = new QCheckBox();
	headerCheckBox->setChecked(true);
	connect(headerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(headerCheckBoxChanged(int)));
	seriesGrid->addWidget(headerCheckBox, 0, 0);

	/* Headers for series data */

	QLabel *headerName = new QLabel("Series Name");
	seriesGrid->addWidget(headerName, 0, 1, Qt::AlignVCenter);
	QLabel *headerChargeWeight = new QLabel("Charge Weight");
	seriesGrid->addWidget(headerChargeWeight, 0, 2, Qt::AlignVCenter);
	QLabel *headerResult = new QLabel("Series Result");
	seriesGrid->addWidget(headerResult, 0, 3, Qt::AlignVCenter);
	QLabel *headerDate = new QLabel("Series Date");
	seriesGrid->addWidget(headerDate, 0, 4, Qt::AlignVCenter);

	for ( int i = 0; i < seriesData.size(); i++ )
	{
		ChronoSeries *series = seriesData.at(i);

		connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesCheckBoxChanged(int)));

		seriesGrid->addWidget(series->enabled, i + 1, 0);
		seriesGrid->addWidget(new QLabel(series->name), i + 1, 1, Qt::AlignVCenter);

		QHBoxLayout *chargeWeightLayout = new QHBoxLayout();
		chargeWeightLayout->addWidget(series->chargeWeight);
		chargeWeightLayout->addStretch(0);
		seriesGrid->addLayout(chargeWeightLayout, i + 1, 2);

		int totalShots = series->muzzleVelocities.size();
		int velocityMin = *std::min_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
		int velocityMax = *std::max_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
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

void PowderTest::showGraph ( bool state )
{
	qDebug() << "showGraph state =" << state;

	renderGraph(true);
}

double sampleStdev ( QList<int> muzzleVelocities )
{
	int totalShots = muzzleVelocities.size();
	double mean = std::accumulate(muzzleVelocities.begin(), muzzleVelocities.end(), 0LL) / static_cast<double>(totalShots);

	double temp = 0;
	for ( int i = 0; i < totalShots; i++ )
	{
		double mV = static_cast<double>(muzzleVelocities.at(i));
		temp += pow(mV - mean, 2);
	}

	// subtract 1 in denominator for sample variance
	return std::sqrt(temp / (totalShots - 1));
}

// We need to re-implement this because QStringList.join() includes empty strings
QString StringListJoin ( QStringList stringList, const char *separator )
{
	std::ostringstream result;
	bool atLeastOne = false;

	foreach ( QString str, stringList )
	{
		// skip empty strings
		if ( str.isEmpty() )
		{
			continue;
		}

		// skip the separator for the first element
		if ( atLeastOne )
		{
			result << separator;
		}

		atLeastOne = true;

		result << str.toStdString();
	}

	return QString::fromStdString(result.str());
}

std::vector<double> GetLinearFit( QVector<double>& xPoints, QVector<double>& yPoints )
{
    double xSum = 0, ySum = 0, xxSum = 0, xySum = 0, slope, intercept;
    std::vector<double> xData;

    //for ( int i = 0; i < data.size(); i++)
    //{
    //    xData.push_back(static_cast<T>(i));
    //}

    for ( int i = 0; i < yPoints.size(); i++ )
    {
        xSum += xPoints.at(i);
        ySum += yPoints.at(i);
        xxSum += xPoints.at(i) * xPoints.at(i);
        xySum += xPoints.at(i) * yPoints.at(i);
    }

    slope = (yPoints.size() * xySum - xSum * ySum) / (yPoints.size() * xxSum - xSum * xSum);
    intercept = (ySum - slope * xSum) / yPoints.size();
    std::vector<double> res;
    res.push_back(slope);
    res.push_back(intercept);
    return res;
}

void PowderTest::saveGraph ( bool state )
{
	qDebug() << "showGraph state =" << state;

	renderGraph(false);
}

struct SplineSet{
    double a;
    double b;
    double c;
    double d;
    double x;
};

std::vector<SplineSet> spline(QVector<double> &x, QVector<double> &y)
{
    int n = x.size()-1;
    std::vector<double> a;
    a.insert(a.begin(), y.begin(), y.end());
    std::vector<double> b(n);
    std::vector<double> d(n);
    std::vector<double> h;

    for(int i = 0; i < n; ++i)
        h.push_back(x.at(i+1)-x.at(i));

    std::vector<double> alpha;
    alpha.push_back(0);
    for(int i = 1; i < n; ++i)
        alpha.push_back( 3*(a[i+1]-a[i])/h[i] - 3*(a[i]-a[i-1])/h[i-1]  );

    std::vector<double> c(n+1);
    std::vector<double> l(n+1);
    std::vector<double> mu(n+1);
    std::vector<double> z(n+1);
    l[0] = 1;
    mu[0] = 0;
    z[0] = 0;

    for(int i = 1; i < n; ++i)
    {
        l[i] = 2 *(x.at(i+1)-x.at(i-1))-h[i-1]*mu[i-1];
        mu[i] = h[i]/l[i];
        z[i] = (alpha[i]-h[i-1]*z[i-1])/l[i];
    }

    l[n] = 1;
    z[n] = 0;
    c[n] = 0;

    for(int j = n-1; j >= 0; --j)
    {
        c[j] = z [j] - mu[j] * c[j+1];
        b[j] = (a[j+1]-a[j])/h[j]-h[j]*(c[j+1]+2*c[j])/3;
        d[j] = (c[j+1]-c[j])/3/h[j];
    }

    std::vector<SplineSet> output_set(n);
    for(int i = 0; i < n; ++i)
    {
        output_set[i].a = a[i];
        output_set[i].b = b[i];
        output_set[i].c = c[i];
        output_set[i].d = d[i];
        output_set[i].x = x.at(i);
    }
    return output_set;
}

class SmoothCurveGenerator
{
protected:
    static QPainterPath generateSmoothCurveImp(const QVector<QPointF> &points) {
        QPainterPath path;
        int len = points.size();

        if (len < 2) {
            return path;
        }

        QVector<QPointF> firstControlPoints;
        QVector<QPointF> secondControlPoints;
        calculateControlPoints(points, &firstControlPoints, &secondControlPoints);

        path.moveTo(points[0].x(), points[0].y());

        // Using bezier curve to generate a smooth curve.
        for (int i = 0; i < len - 1; ++i) {
            path.cubicTo(firstControlPoints[i], secondControlPoints[i], points[i+1]);
        }

        return path;
    }
public:
    static QPainterPath generateSmoothCurve(const QVector<QPointF> &points) {
        QPainterPath result;

        int segmentStart = 0;
        int i = 0;
        int pointSize = points.size();
        while (i < pointSize) {
            if (qIsNaN(points.at(i).y()) || qIsNaN(points.at(i).x()) || qIsInf(points.at(i).y())) {
                //QVector<QPointF> lineData(QVector<QPointF>(points.constBegin() + segmentStart, points.constBegin() + i - segmentStart));
                QVector<QPointF> lineData(QVector<QPointF>(points.mid(segmentStart, i - segmentStart)));
                result.addPath(generateSmoothCurveImp(lineData));
                segmentStart = i + 1;
            }
            ++i;
        }
        //QVector<QPointF> lineData(QVector<QPointF>(points.constBegin() + segmentStart, points.constEnd()));
        QVector<QPointF> lineData(QVector<QPointF>(points.mid(segmentStart)));
        result.addPath(generateSmoothCurveImp(lineData));
        return result;
    }

    static QPainterPath generateSmoothCurve(const QPainterPath &basePath, const QVector<QPointF> &points) {
        if (points.isEmpty()) return basePath;

        QPainterPath path = basePath;
        int len = points.size();
        if (len == 1) {
            path.lineTo(points.at(0));
            return path;
        }

        QVector<QPointF> firstControlPoints;
        QVector<QPointF> secondControlPoints;
        calculateControlPoints(points, &firstControlPoints, &secondControlPoints);

        path.lineTo(points.at(0));
        for (int i = 0; i < len - 1; ++i)
            path.cubicTo(firstControlPoints[i], secondControlPoints[i], points[i+1]);

        return path;
    }

    static void calculateFirstControlPoints(double *&result, const double *rhs, int n) {
        result = new double[n];
        double *tmp = new double[n];
        double b = 2.0;
        result[0] = rhs[0] / b;

        // Decomposition and forward substitution.
        for (int i = 1; i < n; i++) {
            tmp[i] = 1 / b;
            b = (i < n - 1 ? 4.0 : 3.5) - tmp[i];
            result[i] = (rhs[i] - result[i - 1]) / b;
        }

        for (int i = 1; i < n; i++) {
            result[n - i - 1] -= tmp[n - i] * result[n - i]; // Backsubstitution.
        }

        delete[] tmp;
    }

    static void calculateControlPoints(const QVector<QPointF> &knots,
                                                       QVector<QPointF> *firstControlPoints,
                                                       QVector<QPointF> *secondControlPoints) {
        int n = knots.size() - 1;

        firstControlPoints->reserve(n);
        secondControlPoints->reserve(n);

        for (int i = 0; i < n; ++i) {
            firstControlPoints->append(QPointF());
            secondControlPoints->append(QPointF());
        }

        if (n == 1) {
            // Special case: Bezier curve should be a straight line.
            // P1 = (2P0 + P3) / 3
            (*firstControlPoints)[0].rx() = (2 * knots[0].x() + knots[1].x()) / 3;
            (*firstControlPoints)[0].ry() = (2 * knots[0].y() + knots[1].y()) / 3;

            // P2 = 2P1 – P0
            (*secondControlPoints)[0].rx() = 2 * (*firstControlPoints)[0].x() - knots[0].x();
            (*secondControlPoints)[0].ry() = 2 * (*firstControlPoints)[0].y() - knots[0].y();

            return;
        }

        // Calculate first Bezier control points
        double *xs = NULL;
        double *ys = NULL;
        double *rhsx = new double[n]; // Right hand side vector
        double *rhsy = new double[n]; // Right hand side vector

        // Set right hand side values
        for (int i = 1; i < n - 1; ++i) {
            rhsx[i] = 4 * knots[i].x() + 2 * knots[i + 1].x();
            rhsy[i] = 4 * knots[i].y() + 2 * knots[i + 1].y();
        }
        rhsx[0] = knots[0].x() + 2 * knots[1].x();
        rhsx[n - 1] = (8 * knots[n - 1].x() + knots[n].x()) / 2.0;
        rhsy[0] = knots[0].y() + 2 * knots[1].y();
        rhsy[n - 1] = (8 * knots[n - 1].y() + knots[n].y()) / 2.0;

        // Calculate first control points coordinates
        calculateFirstControlPoints(xs, rhsx, n);
        calculateFirstControlPoints(ys, rhsy, n);

        // Fill output control points.
        for (int i = 0; i < n; ++i) {
            (*firstControlPoints)[i].rx() = xs[i];
            (*firstControlPoints)[i].ry() = ys[i];

            if (i < n - 1) {
                (*secondControlPoints)[i].rx() = 2 * knots[i + 1].x() - xs[i + 1];
                (*secondControlPoints)[i].ry() = 2 * knots[i + 1].y() - ys[i + 1];
            } else {
                (*secondControlPoints)[i].rx() = (knots[n].x() + xs[n - 1]) / 2;
                (*secondControlPoints)[i].ry() = (knots[n].y() + ys[n - 1]) / 2;
            }
        }

        delete xs;
        delete ys;
        delete[] rhsx;
        delete[] rhsy;
    }
};

void QCPSmoothGraph::drawLinePlot(QCPPainter *painter, const QVector<QPointF> &lines) const
{
  qDebug() << "QCPSmoothGraph::drawLinePlot lines =" << lines;
  if (painter->pen().style() != Qt::NoPen && painter->pen().color().alpha() != 0)
  {
    applyDefaultAntialiasingHint(painter);
	if ( mSmooth && mLineStyle == lsLine )
	{
		painter->drawPath(SmoothCurveGenerator::generateSmoothCurve(lines));
	}
	else
	{
    	drawPolyline(painter, lines);
	}
  }
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
		if ( series->enabled->isChecked() )
		{
			numEnabled += 1;
			if ( series->chargeWeight->value() == 0 )
			{
				qDebug() << series->name << "is missing charge weight, bailing";

				QMessageBox *msg = new QMessageBox();
				msg->setIcon(QMessageBox::Critical);
				msg->setText(QString("'%1' is missing charge weight!").arg(series->name));
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

		if ( series->enabled->isChecked() )
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

		int totalShots = series->muzzleVelocities.size();
		double mean = std::accumulate(series->muzzleVelocities.begin(), series->muzzleVelocities.end(), 0LL) / static_cast<double>(totalShots);
		double stdev = sampleStdev(series->muzzleVelocities);

		qDebug() << "Total shots:" << totalShots;
		qDebug() << "Mean:" << mean;
		qDebug() << "Stdev:" << stdev;
		qDebug() << "";

		// We use the index as our x-value here to ensure equal spacing between series regardless of their charge weight. We override the xAxis ticker to then use our custom tick labels.
		xAvgPoints.push_back(i);
		yAvgPoints.push_back(mean);

		if ( graphType->currentIndex() == SCATTER )
		{
			for ( int j = 0; j < totalShots; j++ )
			{
				xPoints.push_back(i);
				yPoints.push_back(series->muzzleVelocities.at(j));
				allXPoints.push_back(i);
				allYPoints.push_back(series->muzzleVelocities.at(j));
			}
		}
		else
		{
			for ( int j = 0; j < totalShots; j++ )
			{
				allXPoints.push_back(i);
				allYPoints.push_back(series->muzzleVelocities.at(j));
			}

			xPoints.push_back(i);
			yPoints.push_back(mean);
			yError.push_back(stdev);
		}

		textTicker->addTick(i, QString::number(series->chargeWeight->value()));
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
		xTrendPoints.push_back(allXPoints.at(0));
		yTrendPoints.push_back(res[1]);
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
		velocityUnits2 = "fps";
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

		int totalShots = series->muzzleVelocities.size();
		int velocityMin = *std::min_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
		int velocityMax = *std::max_element(series->muzzleVelocities.begin(), series->muzzleVelocities.end());
		double mean = std::accumulate(series->muzzleVelocities.begin(), series->muzzleVelocities.end(), 0LL) / static_cast<double>(totalShots);
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
			rect->topLeft->setCoords(customPlot->xAxis->coordToPixel(i) - 7, customPlot->yAxis->coordToPixel(velocityMax) - 7);
			rect->bottomRight->setType(QCPItemPosition::ptAbsolute);
			rect->bottomRight->setCoords(customPlot->xAxis->coordToPixel(i) + 7, customPlot->yAxis->coordToPixel(velocityMin) + 7);
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
				annotation = QString::number(mean, 'f', 1);
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
		belowAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(i), customPlot->yAxis->coordToPixel(yCoordBelow) + 10);
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
		aboveAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(i), customPlot->yAxis->coordToPixel(yCoordAbove) - 10);
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

	qDebug() << "seriesCheckBoxChanged state =" << state;

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

				seriesName->setStyleSheet("");
				chargeWeight->setEnabled(true);
				seriesResult->setStyleSheet("");
				seriesDate->setStyleSheet("");
			}
			else
			{
				qDebug() << "Grid row" << i << "was unchecked";

				seriesName->setStyleSheet("color: #878787");
				chargeWeight->setEnabled(false);
				seriesResult->setStyleSheet("color: #878787");
				seriesDate->setStyleSheet("color: #878787");
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
		qDebug() << "Header checkbox was checked";

		for ( int i = 0; i < seriesGrid->rowCount() - 1; i++ )
		{
			QCheckBox *checkBox = qobject_cast<QCheckBox *>(seriesGrid->itemAtPosition(i, 0)->widget());
			checkBox->setChecked(false);
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

void PowderTest::optionCheckBoxChanged ( QCheckBox *checkBox, QLabel *label, QComboBox *comboBox )
{
	if ( checkBox->isChecked() )
	{
		qDebug() << "checkbox was checked";
		label->setStyleSheet("");
		comboBox->setEnabled(true);
	}
	else
	{
		qDebug() << "checkbox was unchecked";
		label->setStyleSheet("color: #878787");
		comboBox->setEnabled(false);
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

			series->name = fileName;

			series->chargeWeight = new QDoubleSpinBox();
			series->chargeWeight->setDecimals(2);
			series->chargeWeight->setSingleStep(0.1);
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
	QList<ChronoSeries *> allSeries;
	ChronoSeries *curSeries = new ChronoSeries();
	curSeries->isValid = false;
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
				if ( (curSeries->seriesNum == -1) || curSeries->velocityUnits.isNull() )
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
				curSeries->seriesNum = -1;
			}
			else if ( (rows.at(0).compare("Series") == 0) && (rows.at(2) == "Shots:") )
			{
				curSeries->seriesNum = rows.at(1).toInt();
				curSeries->name = QString("Series %1").arg(rows.at(1).toInt());
				qDebug() << "seriesNum =" << curSeries->seriesNum;
			}
			else
			{
				bool ok = false;

				// If cell is a valid integer, it's a velocity value
				rows.at(0).toInt(&ok);
				if ( ok )
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

		i++;
	}

	return allSeries;
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

		QList<QList<int> > enabledSeriesVelocs;
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
			QList<int> newVelocs;
			for ( int j = 0; j < enabledSeriesVelocs.size(); j++ )
			{
				newVelocs.append(enabledSeriesVelocs.at(j).at(i));
			}

			ChronoSeries *newSeries = new ChronoSeries();

			newSeries->seriesNum = i;

			newSeries->name = QString("Series %1").arg(i + 1);

			newSeries->muzzleVelocities = newVelocs;

			newSeries->enabled = new QCheckBox();
			newSeries->enabled->setChecked(true);

			newSeries->chargeWeight = new QDoubleSpinBox();
			newSeries->chargeWeight->setDecimals(2);
			newSeries->chargeWeight->setSingleStep(0.1);
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
			if ( series->enabled->isChecked() )
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

void SeatingDepthTest::autofillClicked ( bool state )
{
	qDebug() << "autofillClicked state =" << state;

	AutofillSeatingDialog *dialog = new AutofillSeatingDialog(this);
	int result = dialog->exec();

	qDebug() << "dialog result:" << result;

	if ( result )
	{
		qDebug() << "User OK'd dialog";

		AutofillSeatingValues *values = dialog->getValues();

		double currentLength = values->startingLength;

		for ( int i = 0; i < seatingSeriesData.size(); i++ )
		{
			SeatingSeries *series = seatingSeriesData.at(i);
			if ( (! series->deleted) && series->enabled->isChecked() )
			{
				qDebug() << "Setting series" << i << "to" << currentLength;
				series->cartridgeLength->setValue(currentLength);
				if ( values->increasing )
				{
					currentLength += values->interval;
				}
				else
				{
					currentLength -= values->interval;
				}
			}
		}
	}
	else
	{
		qDebug() << "User cancelled dialog";
	}
}

void SeatingDepthTest::addNewClicked ( bool state )
{
	qDebug() << "addNewClicked state =" << state;

	// un-bold the button after the first click
	addNewButton->setStyleSheet("");

	int numRows = seatingSeriesGrid->rowCount();

	// Remove the stretch from the last row, we'll be using the row for our new series
	seatingSeriesGrid->setRowStretch(numRows - 1, 0);

	qDebug() << "numRows =" << numRows;

	SeatingSeries *series = new SeatingSeries();

	series->deleted = false;

	series->enabled = new QCheckBox();
	series->enabled->setChecked(true);

	connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesCheckBoxChanged(int)));
	seatingSeriesGrid->addWidget(series->enabled, numRows - 1, 0);

	int newSeriesNum = 1;
	for ( int i = seatingSeriesData.size() - 1; i >= 0; i-- )
	{
		SeatingSeries *series = seatingSeriesData.at(i);
		if ( ! series->deleted )
		{
			newSeriesNum = series->seriesNum + 1;
			qDebug() << "Found last un-deleted series" << series->seriesNum << "(" << series->name->text() << ") at index" << i;
			break;
		}
	}

	series->seriesNum = newSeriesNum;

	series->name = new QLabel(QString("Series %1").arg(newSeriesNum));
	seatingSeriesGrid->addWidget(series->name, numRows - 1, 1);

	series->cartridgeLength = new QDoubleSpinBox();
	series->cartridgeLength->setDecimals(3);
	series->cartridgeLength->setSingleStep(0.001);
	series->cartridgeLength->setMinimumWidth(100);
	series->cartridgeLength->setMaximumWidth(100);
	seatingSeriesGrid->addWidget(series->cartridgeLength, numRows - 1, 2);

	series->groupSize = new QDoubleSpinBox();
	series->groupSize->setDecimals(3);
	series->groupSize->setSingleStep(0.001);
	series->groupSize->setMinimumWidth(100);
	series->groupSize->setMaximumWidth(100);
	seatingSeriesGrid->addWidget(series->groupSize, numRows - 1, 3);

	series->deleteButton = new QPushButton();
	connect(series->deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteClicked(bool)));
	series->deleteButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
	series->deleteButton->setFixedSize(series->deleteButton->minimumSizeHint());
	seatingSeriesGrid->addWidget(series->deleteButton, numRows - 1, 4, Qt::AlignLeft);

	seatingSeriesData.append(series);

	// Add an empty stretch row at the end for proper spacing
	seatingSeriesGrid->setRowStretch(numRows, 1);
}

void SeatingDepthTest::deleteClicked ( bool state )
{
	QPushButton *deleteButton = qobject_cast<QPushButton *>(sender());

	qDebug() << "deleteClicked state =" << state;

	int newSeriesNum = -1;

	// We have the checkbox object, now locate its row in the grid
	for ( int i = 0; i < seatingSeriesData.size(); i++ )
	{
		SeatingSeries *series = seatingSeriesData.at(i);
		if ( deleteButton == series->deleteButton )
		{
			qDebug() << "Series" << series->seriesNum << "(" << series->name->text() << ") was deleted";

			series->deleted = true;

			series->enabled->hide();
			series->name->hide();
			series->cartridgeLength->hide();
			series->groupSize->hide();
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

SeatingDepthTest::SeatingDepthTest ( QWidget *parent )
	: QWidget(parent)
{
	qDebug() << "Seating depth test";

	/* Left panel */

	graphPreview = NULL;
	prevSaveDir = QDir::homePath();

	QVBoxLayout *leftLayout = new QVBoxLayout();

	// Wrap grid in a widget to make the grid scrollable
	QWidget *scrollAreaWidget = new QWidget();

	seatingSeriesGrid = new QGridLayout(scrollAreaWidget);
	seatingSeriesGrid->setColumnStretch(0, 0);
	seatingSeriesGrid->setColumnStretch(1, 1);
	seatingSeriesGrid->setColumnStretch(2, 2);
	seatingSeriesGrid->setColumnStretch(3, 2);
	seatingSeriesGrid->setColumnStretch(4, 3);
	seatingSeriesGrid->setHorizontalSpacing(25);

	QCheckBox *headerCheckBox = new QCheckBox();
	headerCheckBox->setChecked(true);
	connect(headerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(headerCheckBoxChanged(int)));
	seatingSeriesGrid->addWidget(headerCheckBox, 0, 0);

	/* Headers for series data */

	QLabel *headerNumber = new QLabel("Series Name");
	seatingSeriesGrid->addWidget(headerNumber, 0, 1, Qt::AlignVCenter);
	headerLengthType = new QLabel("CBTO");
	seatingSeriesGrid->addWidget(headerLengthType, 0, 2, Qt::AlignVCenter);
	headerGroupType = new QLabel("Group Size (ES)");
	seatingSeriesGrid->addWidget(headerGroupType, 0, 3, Qt::AlignVCenter);
	QLabel *headerDelete = new QLabel("");
	seatingSeriesGrid->addWidget(headerDelete, 0, 4, Qt::AlignVCenter);

	scrollArea = new QScrollArea();
	scrollArea->setWidget(scrollAreaWidget);
	scrollArea->setWidgetResizable(true);

	scrollLayout = new QVBoxLayout();
	scrollLayout->addWidget(scrollArea);

	addNewButton = new QPushButton("Add new group");
	addNewButton->setStyleSheet("font-weight: bold");
	connect(addNewButton, SIGNAL(clicked(bool)), this, SLOT(addNewClicked(bool)));
	addNewButton->setMinimumWidth(225);
	addNewButton->setMaximumWidth(225);

	QPushButton *autofillButton = new QPushButton("Auto-fill cartridge lengths");
	connect(autofillButton, SIGNAL(clicked(bool)), this, SLOT(autofillClicked(bool)));
	autofillButton->setMinimumWidth(225);
	autofillButton->setMaximumWidth(225);

	QHBoxLayout *utilitiesLayout = new QHBoxLayout();
	utilitiesLayout->addWidget(addNewButton);
	utilitiesLayout->addWidget(autofillButton);

	scrollLayout->addLayout(utilitiesLayout);

	QGroupBox *seatingGroupBox = new QGroupBox("Seating depth data:");
	seatingGroupBox->setLayout(scrollLayout);

	leftLayout->addWidget(seatingGroupBox);

	/* Create initial row */

	SeatingSeries *series = new SeatingSeries();

	series->deleted = false;

	series->enabled = new QCheckBox();
	series->enabled->setChecked(true);

	connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesCheckBoxChanged(int)));
	seatingSeriesGrid->addWidget(series->enabled, 1, 0);

	series->seriesNum = 1;

	series->name = new QLabel("Series 1");
	seatingSeriesGrid->addWidget(series->name, 1, 1);

	series->cartridgeLength = new QDoubleSpinBox();
	series->cartridgeLength->setDecimals(3);
	series->cartridgeLength->setSingleStep(0.001);
	series->cartridgeLength->setMinimumWidth(100);
	series->cartridgeLength->setMaximumWidth(100);
	seatingSeriesGrid->addWidget(series->cartridgeLength, 1, 2);

	series->groupSize = new QDoubleSpinBox();
	series->groupSize->setDecimals(3);
	series->groupSize->setSingleStep(0.001);
	series->groupSize->setMinimumWidth(100);
	series->groupSize->setMaximumWidth(100);
	seatingSeriesGrid->addWidget(series->groupSize, 1, 3);

	series->deleteButton = new QPushButton();
	connect(series->deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteClicked(bool)));
	series->deleteButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
	series->deleteButton->setFixedSize(series->deleteButton->minimumSizeHint());
	seatingSeriesGrid->addWidget(series->deleteButton, 1, 4, Qt::AlignLeft);

	seatingSeriesGrid->setRowMinimumHeight(0, series->cartridgeLength->sizeHint().height());

	seatingSeriesData.append(series);

	// Add an empty stretch row at the end for proper spacing
	seatingSeriesGrid->setRowStretch(seatingSeriesGrid->rowCount(), 1);

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
	distance = new QLineEdit();
	detailsFormLayout->addRow(new QLabel("Graph title:"), graphTitle);
	detailsFormLayout->addRow(new QLabel("Rifle:"), rifle);
	detailsFormLayout->addRow(new QLabel("Projectile:"), projectile);
	detailsFormLayout->addRow(new QLabel("Propellant:"), propellant);
	detailsFormLayout->addRow(new QLabel("Brass:"), brass);
	detailsFormLayout->addRow(new QLabel("Primer:"), primer);
	detailsFormLayout->addRow(new QLabel("Weather:"), weather);
	detailsFormLayout->addRow(new QLabel("Distance:"), distance);

	QGroupBox *detailsGroupBox = new QGroupBox("Graph details:");
	detailsGroupBox->setLayout(detailsFormLayout);

	/* Graph options */

	QVBoxLayout *optionsLayout = new QVBoxLayout();

	QFormLayout *optionsFormLayout = new QFormLayout();

	cartridgeMeasurementType = new QComboBox();
	cartridgeMeasurementType->addItem("CBTO");
	cartridgeMeasurementType->addItem("COAL");
	connect(cartridgeMeasurementType, SIGNAL(activated(int)), this, SLOT(cartridgeMeasurementTypeChanged(int)));
	optionsFormLayout->addRow(new QLabel("Cartridge measurement:"), cartridgeMeasurementType);

	cartridgeUnits = new QComboBox();
	cartridgeUnits->addItem("inches (in)");
	cartridgeUnits->addItem("centimeters (cm)");
	optionsFormLayout->addRow(new QLabel("Cartridge units:"), cartridgeUnits);

	groupMeasurementType = new QComboBox();
	groupMeasurementType->addItem("Extreme Spread (ES)");
	groupMeasurementType->addItem("Radial Stdev (RSD)");
	groupMeasurementType->addItem("Mean Radius (MR)");
	connect(groupMeasurementType, SIGNAL(activated(int)), this, SLOT(groupMeasurementTypeChanged(int)));
	optionsFormLayout->addRow(new QLabel("Group size measurement:"), groupMeasurementType);

	groupUnits = new QComboBox();
	groupUnits->addItem("inches (in)");
	groupUnits->addItem("minutes (MOA)");
	groupUnits->addItem("centimeters (cm)");
	groupUnits->addItem("milliradians (mil)");
	optionsFormLayout->addRow(new QLabel("Group size units:"), groupUnits);

	optionsLayout->addLayout(optionsFormLayout);
	optionsLayout->addWidget(new QHLine());

	QHBoxLayout *groupSizeLayout = new QHBoxLayout();
	groupSizeCheckBox = new QCheckBox();
	groupSizeCheckBox->setChecked(true);
	connect(groupSizeCheckBox, SIGNAL(clicked(bool)), this, SLOT(groupSizeCheckBoxChanged(bool)));
	groupSizeLayout->addWidget(groupSizeCheckBox, 0);
	groupSizeLabel = new QLabel("Show group size");
	groupSizeLayout->addWidget(groupSizeLabel, 1);
	groupSizeLocation = new QComboBox();
	groupSizeLocation->addItem("above shot strings");
	groupSizeLocation->addItem("below shot strings");
	groupSizeLayout->addWidget(groupSizeLocation);
	optionsLayout->addLayout(groupSizeLayout);

	QHBoxLayout *gsdLayout = new QHBoxLayout();
	gsdCheckBox = new QCheckBox();
	gsdCheckBox->setChecked(false);
	connect(gsdCheckBox, SIGNAL(clicked(bool)), this, SLOT(gsdCheckBoxChanged(bool)));
	gsdLayout->addWidget(gsdCheckBox, 0);
	gsdLabel = new QLabel("Show group size deltas");
	gsdLabel->setStyleSheet("color: #878787");
	gsdLayout->addWidget(gsdLabel, 1);
	gsdLocation = new QComboBox();
	gsdLocation->addItem("above shot strings");
	gsdLocation->addItem("below shot strings");
	gsdLocation->setCurrentIndex(1);
	gsdLocation->setEnabled(false);
	gsdLayout->addWidget(gsdLocation);
	optionsLayout->addLayout(gsdLayout);

	QHBoxLayout *trendLayout = new QHBoxLayout();
	trendCheckBox = new QCheckBox();
	trendCheckBox->setChecked(false);
	connect(trendCheckBox, SIGNAL(clicked(bool)), this, SLOT(trendCheckBoxChanged(bool)));
	trendLayout->addWidget(trendCheckBox, 0);
	trendLabel = new QLabel("Show trend line");
	trendLabel->setStyleSheet("color: #878787");
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

void SeatingDepthTest::seriesCheckBoxChanged ( int state )
{
	QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());

	qDebug() << "seriesCheckBoxChanged state =" << state;

	// We have the checkbox object, now locate its row in the grid
	for ( int i = 0; i < seatingSeriesGrid->rowCount() - 1; i++ )
	{
		QWidget *rowCheckBox = seatingSeriesGrid->itemAtPosition(i, 0)->widget();
		if ( checkBox == rowCheckBox )
		{
			QWidget *seriesName = seatingSeriesGrid->itemAtPosition(i, 1)->widget();
			QWidget *cartridgeLength = seatingSeriesGrid->itemAtPosition(i, 2)->widget();
			QWidget *groupSize = seatingSeriesGrid->itemAtPosition(i, 3)->widget();
			QWidget *deleteButton = seatingSeriesGrid->itemAtPosition(i, 4)->widget();

			if ( checkBox->isChecked() )
			{
				qDebug() << "Grid row" << i << "was checked";

				seriesName->setStyleSheet("");
				cartridgeLength->setEnabled(true);
				groupSize->setEnabled(true);
				deleteButton->setEnabled(true);;
			}
			else
			{
				qDebug() << "Grid row" << i << "was unchecked";

				seriesName->setStyleSheet("color: #878787");
				cartridgeLength->setEnabled(false);
				groupSize->setEnabled(false);
				deleteButton->setEnabled(false);
			}

			return;
		}
	}
}

void SeatingDepthTest::headerCheckBoxChanged ( int state )
{
	qDebug() << "headerCheckBoxChanged state =" << state;

	if ( state == Qt::Checked )
	{
		qDebug() << "Header checkbox was checked";

		for ( int i = 0; i < seatingSeriesGrid->rowCount() - 1; i++ )
		{
			QCheckBox *checkBox = qobject_cast<QCheckBox *>(seatingSeriesGrid->itemAtPosition(i, 0)->widget());
			checkBox->setChecked(true);
		}
	}
	else
	{
		qDebug() << "Header checkbox was checked";

		for ( int i = 0; i < seatingSeriesGrid->rowCount() - 1; i++ )
		{
			QCheckBox *checkBox = qobject_cast<QCheckBox *>(seatingSeriesGrid->itemAtPosition(i, 0)->widget());
			checkBox->setChecked(false);
		}
	}
}

void SeatingDepthTest::groupSizeCheckBoxChanged ( bool state )
{
	qDebug() << "groupSizeCheckBoxChanged state =" << state;

	optionCheckBoxChanged(groupSizeCheckBox, groupSizeLabel, groupSizeLocation);
}

void SeatingDepthTest::gsdCheckBoxChanged ( bool state )
{
	qDebug() << "gsdCheckBoxChanged state =" << state;

	optionCheckBoxChanged(gsdCheckBox, gsdLabel, gsdLocation);
}

void SeatingDepthTest::trendCheckBoxChanged ( bool state )
{
	qDebug() << "trendCheckBoxChanged state =" << state;

	optionCheckBoxChanged(trendCheckBox, trendLabel, trendLineType);
}

void SeatingDepthTest::optionCheckBoxChanged ( QCheckBox *checkBox, QLabel *label, QComboBox *comboBox )
{
	if ( checkBox->isChecked() )
	{
		qDebug() << "checkbox was checked";
		label->setStyleSheet("");
		comboBox->setEnabled(true);
	}
	else
	{
		qDebug() << "checkbox was unchecked";
		label->setStyleSheet("color: #878787");
		comboBox->setEnabled(false);
	}
}

void SeatingDepthTest::cartridgeMeasurementTypeChanged ( int index )
{
	qDebug() << "cartridgeMeasurementTypeChanged index =" << index;

	headerLengthType->setText(cartridgeMeasurementType->currentText());
}

void SeatingDepthTest::groupMeasurementTypeChanged ( int index )
{
	qDebug() << "groupMeasurementTypeChanged index =" << index;

	if ( index == ES )
	{
		headerGroupType->setText("Group Size (ES)");
	}
	else if ( index == RSD )
	{
		headerGroupType->setText("Group Size (RSD)");
	}
	else
	{
		headerGroupType->setText("Group Size (MR)");
	}
}

void SeatingDepthTest::showGraph ( bool state )
{
	qDebug() << "showGraph state =" << state;

	renderGraph(true);
}


void SeatingDepthTest::saveGraph ( bool state )
{
	qDebug() << "saveGraph state =" << state;

	renderGraph(false);
}

static bool CartridgeLengthComparator ( SeatingSeries *one, SeatingSeries *two )
{
	return (one->cartridgeLength->value() < two->cartridgeLength->value());
}

void SeatingDepthTest::renderGraph ( bool displayGraphPreview )
{
	qDebug() << "renderGraph displayGraphPreview =" << displayGraphPreview;

	/* Validate series before continuing */

	int numEnabled = 0;
	for ( int i = 0; i < seatingSeriesData.size(); i++ )
	{
		SeatingSeries *series = seatingSeriesData.at(i);
		if ( (! series->deleted) && series->enabled->isChecked() )
		{
			numEnabled += 1;

			if ( series->cartridgeLength->value() == 0 )
			{
				qDebug() << series->name->text() << "is missing cartridge length, bailing";

				QMessageBox *msg = new QMessageBox();
				msg->setIcon(QMessageBox::Critical);
				msg->setText(QString("%1 is missing cartridge length!").arg(series->name->text()));
				msg->setWindowTitle("Error");
				msg->exec();
				return;
			}

			if ( series->groupSize->value() == 0 )
			{
				qDebug() << series->name->text() << "is missing group size, bailing";

				QMessageBox *msg = new QMessageBox();
				msg->setIcon(QMessageBox::Critical);
				msg->setText(QString("%1 is missing group size!").arg(series->name->text()));
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

	QList<SeatingSeries *> seriesToGraph;
	for ( int i = 0; i < seatingSeriesData.size(); i++ )
	{
		SeatingSeries *series = seatingSeriesData.at(i);

		if ( series->deleted )
		{
			qDebug() << series->name->text() << "is deleted, skipping...";
		}
		else if ( ! series->enabled->isChecked() )
		{
			qDebug() << series->name->text() << "is unchecked, skipping...";
		}
		else
		{
			seriesToGraph.append(series);
		}
	}

	/* Sort the data by cartridge length (not that length matters...) in case the user inputted lengths out of order */

	std::sort(seriesToGraph.begin(), seriesToGraph.end(), CartridgeLengthComparator);

	/* Collect the data to graph */

	QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);

	QVector<double> xPoints;
	QVector<double> yPoints;
	QVector<double> yError;

	for ( int i = 0; i < seriesToGraph.size(); i++ )
	{
		SeatingSeries *series = seriesToGraph.at(i);

		double cartridgeLength = series->cartridgeLength->value();
		double groupSize = series->groupSize->value();

		qDebug() << QString("%1 - %2, %3").arg(series->name->text()).arg(cartridgeLength).arg(groupSize);
		qDebug() << "";

		// We use the index as our x-value here to ensure equal spacing between series regardless of their cartridge length. We override the xAxis ticker to then use our custom tick labels.
		xPoints.push_back(i);
		yPoints.push_back(groupSize);

		textTicker->addTick(i, QString::number(series->cartridgeLength->value()));
	}

	/* Create scatter plot */

	QPen seatingLinePen(Qt::SolidLine);
	QColor seatingLineColor("#1c57eb");
	seatingLineColor.setAlphaF(0.65);
	seatingLinePen.setColor(seatingLineColor);
	seatingLinePen.setWidthF(1.5);

	QCPSmoothGraph *scatterPlot = new QCPSmoothGraph(customPlot->xAxis, customPlot->yAxis);
	scatterPlot->setName(QLatin1String("Graph ")+QString::number(customPlot->graphCount()));
	scatterPlot->setSmooth(true);
	scatterPlot->setData(xPoints, yPoints);
	scatterPlot->rescaleAxes();
	scatterPlot->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QColor("#0536b0"), 6.0));
	scatterPlot->setPen(seatingLinePen);

	/* Draw trend line if necessary */

	if ( trendCheckBox->isChecked() )
	{
		std::vector<double> res = GetLinearFit(xPoints, yPoints);
		qDebug() << "linear fit:" << res[0] << res[1];

		std::vector<SplineSet> res2a = spline(xPoints, yPoints);
		SplineSet res2;
		foreach ( res2, res2a )
		{
			qDebug() << "spline:" << res2.a << res2.b << res2.c << res2.d << res2.x;
		}

		QVector<double> xTrendPoints;
		QVector<double> yTrendPoints;
		xTrendPoints.push_back(xPoints.at(0));
		yTrendPoints.push_back(res[1]);
		xTrendPoints.push_back(xPoints.last());
		yTrendPoints.push_back(res[1] + (xPoints.last() * res[0]));

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

	const char *cartridgeMeasurementType2;
	if ( cartridgeMeasurementType->currentIndex() == CBTO )
	{
		cartridgeMeasurementType2 = "CBTO";
	}
	else
	{
		cartridgeMeasurementType2 = "COAL";
	}

	const char *cartridgeUnits2;
	if ( cartridgeUnits->currentIndex() == INCH )
	{
		cartridgeUnits2 = "in";
	}
	else
	{
		cartridgeUnits2 = "cm";
	}

	const char *groupMeasurementType2;
	if ( groupMeasurementType->currentIndex() == ES )
	{
		groupMeasurementType2 = "Extreme Spread";
	}
	else if ( groupMeasurementType->currentIndex() == RSD )
	{
		groupMeasurementType2 = "Radial Standard Deviation";
	}
	else
	{
		groupMeasurementType2 = "Mean Radius";
	}

	const char *groupUnits2;
	if ( groupUnits->currentIndex() == INCH )
	{
		groupUnits2 = "in";
	}
	else if ( groupUnits->currentIndex() == MOA )
	{
		groupUnits2 = "MOA";
	}
	else if ( groupUnits->currentIndex() == CENTIMETER )
	{
		groupUnits2 = "cm";
	}
	else
	{
		groupUnits2 = "mil";
	}

	QCPTextElement *title = new QCPTextElement(customPlot);
	title->setText(QString("\n%1").arg(graphTitle->text()));
	title->setFont(QFont("DejaVu Sans", scaleFontSize(24)));
	title->setTextColor(QColor("#4d4d4d"));
	customPlot->plotLayout()->insertRow(0);
	customPlot->plotLayout()->addElement(0, 0, title);

	QCPTextElement *subtitle = new QCPTextElement(customPlot);
	QStringList subtitleText;
	subtitleText << rifle->text() << propellant->text() << projectile->text() << brass->text() << primer->text() << weather->text() << distance->text();
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

	customPlot->xAxis->setLabel(QString("%1 (%2)").arg(cartridgeMeasurementType2).arg(cartridgeUnits2));
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

	customPlot->yAxis->setLabel(QString("%1 (%2)").arg(groupMeasurementType2).arg(groupUnits2));
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
	 * Generate text annotations. We need to do this after rendering the graph so that coordToPixel() works.
	 */

	bool prevSizeSet = false;
	double prevSize = 0;

	for ( int i = 0; i < seriesToGraph.size(); i++ )
	{
		SeatingSeries *series = seriesToGraph.at(i);

		// Collect annotation contents
		QStringList aboveAnnotationText;
		QStringList belowAnnotationText;

		if ( groupSizeCheckBox->isChecked() )
		{
			QString annotation = QString::number(series->groupSize->value());
			if ( groupSizeLocation->currentIndex() == ABOVE_STRING )
			{
				aboveAnnotationText.append(annotation);
			}
			else
			{
				belowAnnotationText.append(annotation);
			}
		}

		if ( gsdCheckBox->isChecked() )
		{
			if ( prevSizeSet )
			{
				QString sign;
				double delta = series->groupSize->value() - prevSize;
				if ( delta < 0 )
				{
					sign = QString("-");
				}
				else
				{
					sign = QString("+");
				}

				QString annotation = QString("%1%2").arg(sign).arg(fabs(delta), 0, 'f', 3);
				if ( gsdLocation->currentIndex() == ABOVE_STRING )
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

		double yCoord;

		yCoord = series->groupSize->value();

		QCPItemText *belowAnnotation = new QCPItemText(customPlot);
		belowAnnotation->setText(belowAnnotationText.join('\n'));
		belowAnnotation->setFont(QFont("DejaVu Sans", scaleFontSize(9)));
		belowAnnotation->setColor(QColor("#4d4d4d"));
		belowAnnotation->position->setType(QCPItemPosition::ptAbsolute);
		belowAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(i), customPlot->yAxis->coordToPixel(yCoord) + 10);
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
		aboveAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(i), customPlot->yAxis->coordToPixel(yCoord) - 10);
		aboveAnnotation->setPositionAlignment(Qt::AlignHCenter | Qt::AlignBottom);
		aboveAnnotation->setTextAlignment(Qt::AlignCenter);
		aboveAnnotation->setBrush(QBrush(Qt::white));
		aboveAnnotation->setClipToAxisRect(false);
		aboveAnnotation->setLayer(customPlot->layer(5));
		qDebug() << "max pixel coords:" << aboveAnnotation->position->pixelPosition() << "layer:" << aboveAnnotation->layer()->name() << "rect:" << customPlot->axisRect()->layer()->name();

		prevSize = yCoord;
		prevSizeSet = true;
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

About::About ( QWidget *parent )
	: QWidget(parent)
{
	qDebug() << "About this app";

	QPixmap logoPix(":/images/logo.png");

	QLabel *about1 = new QLabel();
	about1->setTextFormat(Qt::RichText);
	about1->setText(QString("<center><h1>Version %1</h1>By Michael Coppola<br><a href=\"https://github.com/mncoppola/ChronoPlotter\">github.com/mncoppola/ChronoPlotter</a>").arg(CHRONOPLOTTER_VERSION));
	about1->setOpenExternalLinks(true);

	QLabel *logo = new QLabel();
	logo->setPixmap(logoPix.scaledToWidth(600, Qt::SmoothTransformation));

	QLabel *about2 = new QLabel();
	about2->setTextFormat(Qt::RichText);
	about2->setText("<center>If you found this tool helpful, share some primers with a friend in need.<br><br>Or consider contributing to:<br><a href=\"https://www.doctorswithoutborders.org/\">Doctors Without Borders</a><br><a href=\"https://www.navysealfoundation.org/\">The Navy SEAL Foundation</a><br><a href=\"https://eji.org/\">Equal Justice Initiative</a><br><a href=\"https://www.mskcc.org/\">Memorial Sloan Kettering Cancer Center</a>");
	about2->setOpenExternalLinks(true);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addStretch(1);
	layout->addWidget(logo);
	layout->addWidget(about1);
	layout->addSpacing(10);
	layout->addWidget(about2);
	layout->setAlignment(logo, Qt::AlignHCenter);
	layout->addStretch(3);

	setLayout(layout);
}

void MainWindow::closeEvent ( QCloseEvent *event )
{
	qDebug() << "closeEvent called";

	event->ignore();

	QMessageBox *box = new QMessageBox(QMessageBox::Question, "ChronoPlotter", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::Cancel, this);

	QObject::connect(box->button(QMessageBox::Yes), &QAbstractButton::clicked, this, &QApplication::quit);
	QObject::connect(box->button(QMessageBox::Cancel), &QAbstractButton::clicked, box, &QObject::deleteLater);
	box->show();
}

int main ( int argc, char *argv[] )
{
	QApplication a(argc, argv);

	int id = QFontDatabase::addApplicationFont(":/DejaVuSans.ttf");
	QString family = QFontDatabase::applicationFontFamilies(id).at(0);
	qDebug() << "id:" << id << "font family:" << family;

	QWidget *powderTab = new PowderTest();

	QWidget *seatingTab = new SeatingDepthTest();

	QWidget *aboutTab = new About();

	QTabWidget *tabs = new QTabWidget();
	tabs->setDocumentMode(true);
	tabs->addTab(powderTab, "Powder charge");
	tabs->addTab(seatingTab, "Seating depth");
	tabs->addTab(aboutTab, "About this app");
	tabs->tabBar()->setExpanding(true);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addWidget(tabs);

	MainWindow *mainWindow = new MainWindow();
	QWidget *w = new QWidget();
	w->setLayout(mainLayout);
	mainWindow->setCentralWidget(w);
	mainWindow->setGeometry(300, 300, 1100, mainLayout->sizeHint().height());
	mainWindow->setWindowTitle("ChronoPlotter");
	mainWindow->show();

	return a.exec();
}
