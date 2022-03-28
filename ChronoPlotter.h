#ifndef CHRONOPLOTTER_H
#define CHRONOPLOTTER_H

#include <sstream>
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

int scaleFontSize ( int );

struct SplineSet
{
	double a;
	double b;
	double c;
	double d;
	double x;
};

std::vector<SplineSet> spline(QVector<double> &x, QVector<double> &y);

std::vector<double> GetLinearFit( QVector<double>&, QVector<double>& );

QString StringListJoin ( QStringList, const char * );

template<typename T>
double sampleStdev ( T );

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		MainWindow() : QMainWindow() {};
		~MainWindow() {};

	protected:
		void closeEvent(QCloseEvent *);
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

#endif // CHRONOPLOTTER_H
