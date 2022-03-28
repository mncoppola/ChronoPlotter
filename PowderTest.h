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

namespace Powder
{
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
};
