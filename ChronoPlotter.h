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
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QDialog>
#include <QMainWindow>
#include "qcustomplot/qcustomplot.h"

#define CHRONOPLOTTER_VERSION "2.2.0"

#define SCATTER 0
#define LINE_SD 1

#define SOLID_LINE 0
#define DASHED_LINE 1

#define GRAINS 0
#define GRAMS 1

#define FPS 0
#define MPS 1

#define CBTO 0
#define COAL 1

#define ES 0
#define YSTDEV 1
#define XSTDEV 2
#define RSD 3
#define MR 4

#define INCH 0
#define MOA 1
#define CENTIMETER 2
#define MIL 3

#define ABOVE_STRING 0
#define BELOW_STRING 1

#define PROPORTIONAL 0
#define CONSTANT 1

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		MainWindow() : QMainWindow() {};
		~MainWindow() {};

	protected:
		void closeEvent(QCloseEvent *);
};

struct ChronoSeries
{
	bool isValid;
	int seriesNum;
	QLabel *name;
	QList<int> muzzleVelocities;
	QString velocityUnits;
	QLabel *result;
	QString firstDate;
	QString firstTime;
	QDoubleSpinBox *chargeWeight;
	QPushButton *enterDataButton;
	QPushButton *deleteButton;
	QCheckBox *enabled;
	bool deleted;
};

class QHLine : public QFrame
{
	public:
		QHLine(QFrame *parent = 0);
		~QHLine() {};
};

class GraphPreview : public QWidget
{
	public:
		GraphPreview(QPixmap&, QWidget *parent = 0);
		~GraphPreview() {};

	protected:
		virtual void resizeEvent(QResizeEvent *);

	private:
		QLabel *label;
};

class About : public QWidget
{
	Q_OBJECT

	public:
		About(QWidget *parent = 0);
		~About() {};

	public slots:
		void showLegal(void);
};

struct SeatingSeries
{
	bool isValid;
	int seriesNum;
	QLabel *name;
	QList<QPair<double, double> > coordinates;
	QList<QPair<double, double> > coordinates_sighters;
	QList<double> extremeSpread;
	QList<double> extremeSpread_sighters;
	QList<double> yStdev;
	QList<double> yStdev_sighters;
	QList<double> xStdev;
	QList<double> xStdev_sighters;
	QList<double> radialStdev;
	QList<double> radialStdev_sighters;
	QList<double> meanRadius;
	QList<double> meanRadius_sighters;
	int targetDistance; // in yards
	QString firstDate;
	QString firstTime;
	QDoubleSpinBox *cartridgeLength;
	QDoubleSpinBox *groupSize;
	QLabel *groupSizeLabel;
	QPushButton *deleteButton;
	QCheckBox *enabled;
	bool deleted;
};

class SeatingDepthTest : public QWidget
{
	Q_OBJECT

	public:
		SeatingDepthTest(QWidget *parent = 0);
		~SeatingDepthTest() {};
		QList<SeatingSeries *> seatingSeriesData;
		QComboBox *cartridgeUnits;

	public slots:
		void groupSizeCheckBoxChanged(bool);
		void gsdCheckBoxChanged(bool);
		void trendCheckBoxChanged(bool);
		void includeSightersCheckBoxChanged(bool);
		void xAxisSpacingChanged(int);
		void cartridgeMeasurementTypeChanged(int);
		void groupMeasurementTypeChanged(int);
		void importedGroupMeasurementTypeChanged(int);
		void importedGroupUnitsChanged(int);
		void loadNewShotData(bool);
		void selectShotMarkerFile(bool);
		void manualDataEntry(bool);
		void addNewClicked(bool);
		void deleteClicked(bool);
		void autofillClicked(bool);
		void headerCheckBoxChanged(int);
		void seriesCheckBoxChanged(int);
		void seriesManualCheckBoxChanged(int);
		void showGraph(bool);
		void saveGraph(bool);

	protected:
		void updateDisplayedData ( void );
		double calculateES ( QList<QPair<double, double> > );
		double calculateYStdev ( QList<QPair<double, double> > );
		double calculateXStdev ( QList<QPair<double, double> > );
		double calculateRSD ( QList<QPair<double, double> > );
		static double pairSumX ( double, const QPair<double, double> );
		static double pairSumY ( double, const QPair<double, double> );
		double calculateMR ( QList<QPair<double, double> > );
		QList<SeatingSeries *> ExtractShotMarkerSeriesTar ( QString );
		QList<SeatingSeries *> ExtractShotMarkerSeriesCsv ( QTextStream & );
		void optionCheckBoxChanged(QCheckBox *, QLabel *, QComboBox *);
		void DisplaySeriesData ( void );
		void renderGraph ( bool );

	private:
		GraphPreview *graphPreview;
		QString prevSaveDir;
		QString prevShotMarkerDir;
		QStackedWidget *stackedWidget;
		QWidget *scrollWidget;
		QVBoxLayout *scrollLayout;
		QScrollArea *scrollArea;
		QGridLayout *seatingSeriesGrid;
		QLabel *headerLengthType;
		QLabel *headerGroupType;
		QLineEdit *graphTitle;
		QLineEdit *rifle;
		QLineEdit *projectile;
		QLineEdit *propellant;
		QLineEdit *brass;
		QLineEdit *primer;
		QLineEdit *weather;
		QLineEdit *distance;
		QPushButton *addNewButton;
		QComboBox *cartridgeMeasurementType;
		QComboBox *groupMeasurementType;
		QComboBox *groupUnits;
		QComboBox *xAxisSpacing;
		QCheckBox *groupSizeCheckBox;
		QCheckBox *gsdCheckBox;
		QCheckBox *trendCheckBox;
		QCheckBox *includeSightersCheckBox;
		QComboBox *groupSizeLocation;
		QComboBox *gsdLocation;
		QComboBox *trendLineType;
		QLabel *groupSizeLabel;
		QLabel *gsdLabel;
		QLabel *trendLabel;
};

class PowderTest : public QWidget
{
	Q_OBJECT

	public:
		PowderTest(QWidget *parent = 0);
		~PowderTest() {};
		QList<ChronoSeries *> seriesData;
		QComboBox *weightUnits;

	public slots:
		void esCheckBoxChanged(bool);
		void sdCheckBoxChanged(bool);
		void avgCheckBoxChanged(bool);
		void vdCheckBoxChanged(bool);
		void trendCheckBoxChanged(bool);
		void xAxisSpacingChanged(int);
		void loadNewChronographData(bool);
		void selectLabRadarDirectory(bool);
		void selectMagnetoSpeedFile(bool);
		void selectProChronoFile(bool);
		void selectShotMarkerFile(bool);
		void manualDataEntry(bool);
		void rrClicked(bool);
		void addNewClicked(bool);
		void enterDataClicked(bool);
		void deleteClicked(bool);
		void autofillClicked(bool);
		void velocityUnitsChanged(int);
		void headerCheckBoxChanged(int);
		void seriesCheckBoxChanged(int);
		void seriesManualCheckBoxChanged(int);
		void showGraph(bool);
		void saveGraph(bool);

	protected:
		void optionCheckBoxChanged(QCheckBox *, QLabel *, QComboBox *);
		ChronoSeries *ExtractLabRadarSeries ( QTextStream & );
		QList<ChronoSeries *> ExtractMagnetoSpeedSeries ( QTextStream & );
		QList<ChronoSeries *> ExtractProChronoSeries ( QTextStream & );
		QList<ChronoSeries *> ExtractProChronoSeries_format2 ( QTextStream & );
		QList<ChronoSeries *> ExtractShotMarkerSeriesTar ( QString );
		void DisplaySeriesData ( void );
		void renderGraph ( bool );

	private:
		GraphPreview *graphPreview;
		QString prevLabRadarDir;
		QString prevMagnetoSpeedDir;
		QString prevProChronoDir;
		QString prevShotMarkerDir;
		QString prevSaveDir;
		QStackedWidget *stackedWidget;
		QWidget *scrollWidget;
		QScrollArea *scrollArea;
		QGridLayout *seriesGrid;
		QLabel *headerResult;
		QLineEdit *graphTitle;
		QLineEdit *rifle;
		QLineEdit *projectile;
		QLineEdit *propellant;
		QLineEdit *brass;
		QLineEdit *primer;
		QLineEdit *weather;
		QPushButton *addNewButton;
		QComboBox *graphType;
		QComboBox *velocityUnits;
		QComboBox *xAxisSpacing;
		QCheckBox *esCheckBox;
		QCheckBox *sdCheckBox;
		QCheckBox *avgCheckBox;
		QCheckBox *vdCheckBox;
		QCheckBox *trendCheckBox;
		QComboBox *esLocation;
		QComboBox *sdLocation;
		QComboBox *avgLocation;
		QComboBox *vdLocation;
		QComboBox *trendLineType;
		QLabel *esLabel;
		QLabel *sdLabel;
		QLabel *avgLabel;
		QLabel *vdLabel;
		QLabel *trendLabel;
};

class RoundRobinDialog : public QDialog
{
	Q_OBJECT

	public:
		RoundRobinDialog(PowderTest *, QDialog *parent = 0);
		~RoundRobinDialog() {};
};

class EnterVelocitiesDialog : public QDialog
{
	Q_OBJECT

	public:
		EnterVelocitiesDialog(ChronoSeries *, QDialog *parent = 0);
		~EnterVelocitiesDialog() {};
		QList<int> getValues();

	public slots:
		void textChanged();

	private:
		QLabel *velocitiesEntered;
		QTextEdit *textEdit;
};

struct AutofillValues
{
	double startingCharge;
	double interval;
	bool increasing;
};

class AutofillDialog : public QDialog
{
	Q_OBJECT

	public:
		AutofillDialog(PowderTest *, QDialog *parent = 0);
		~AutofillDialog() {};
		AutofillValues *getValues();

	private:
		QDoubleSpinBox *startingCharge;
		QDoubleSpinBox *interval;
		QComboBox *direction;
};

struct AutofillSeatingValues
{
	double startingLength;
	double interval;
	bool increasing;
};

class AutofillSeatingDialog : public QDialog
{
	Q_OBJECT

	public:
		AutofillSeatingDialog(SeatingDepthTest *, QDialog *parent = 0);
		~AutofillSeatingDialog() {};
		AutofillSeatingValues *getValues();

	private:
		QDoubleSpinBox *startingLength;
		QDoubleSpinBox *interval;
		QComboBox *direction;
};

class QCPSmoothGraph : public QCPGraph
{
	Q_OBJECT

	public:
		QCPSmoothGraph(QCPAxis *x, QCPAxis *y) : QCPGraph(x, y) {};
		~QCPSmoothGraph() {};
		//void drawLinePlot(QCPPainter *, const QVector<QPointF>&);
		void drawLinePlot(QCPPainter *painter, const QVector<QPointF> &lines) const;
		void setSmooth(bool smooth) {
			mSmooth = smooth;
		}

	private:
		bool mSmooth;

};
