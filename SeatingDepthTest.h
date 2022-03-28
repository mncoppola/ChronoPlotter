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
#include <QTextEdit>

#include "ChronoPlotter.h"

namespace SeatingDepth
{
	struct SplineSet
	{
		double a;
		double b;
		double c;
		double d;
		double x;
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

	struct AutofillValues
	{
		double startingLength;
		double interval;
		bool increasing;
	};

	class AutofillDialog : public QDialog
	{
		Q_OBJECT

		public:
			AutofillDialog(SeatingDepthTest *, QDialog *parent = 0);
			~AutofillDialog() {};
			AutofillValues *getValues();

		private:
			QDoubleSpinBox *startingLength;
			QDoubleSpinBox *interval;
			QComboBox *direction;
	};
};
