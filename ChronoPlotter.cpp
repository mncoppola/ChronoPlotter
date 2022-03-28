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
#include <QList>
#include <QDialogButtonBox>
#include <QDialog>
#include <QByteArray>
#include <QJsonDocument>
#include "qcustomplot/qcustomplot.h"

#include "miniz.c"
#include "untar.h"
#include "ChronoPlotter.h"
#include "PowderTest.h"
#include "SeatingDepthTest.h"
#include "TunerTest.h"
#include "About.h"

int scaleFontSize ( int size )
{
	#if __APPLE__
	return size * 1.3;
	#else
	return size;
	#endif
}

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

QHLine::QHLine ( QFrame *parent )
	: QFrame(parent)
{
	setFrameShape(QFrame::HLine);
	setFrameShadow(QFrame::Sunken);
}

template<typename T>
double sampleStdev ( T vals )
{
	int totalShots = vals.size();
	double mean = std::accumulate(vals.begin(), vals.end(), 0LL) / static_cast<double>(totalShots);

	double temp = 0;
	for ( int i = 0; i < totalShots; i++ )
	{
		double val = vals.at(i);
		temp += pow(val - mean, 2);
	}

	// subtract 1 in denominator for sample variance
	return std::sqrt(temp / (totalShots - 1));
}

template double sampleStdev<QList<int> >(QList<int>);
template double sampleStdev<QList<double> >(QList<double>);

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

	QWidget *powderTab = new Powder::PowderTest();

	QWidget *seatingTab = new SeatingDepth::SeatingDepthTest();

	QWidget *tunerTab = new Tuner::TunerTest();

	QWidget *aboutTab = new About();

	QTabWidget *tabs = new QTabWidget();
	tabs->setDocumentMode(true);
	tabs->addTab(powderTab, "Powder charge");
	tabs->addTab(seatingTab, "Seating depth");
	tabs->addTab(tunerTab, "Tuner");
	tabs->addTab(aboutTab, "About this app");
	tabs->tabBar()->setExpanding(true);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addWidget(tabs);

	MainWindow *mainWindow = new MainWindow();
	QWidget *w = new QWidget();
	w->setLayout(mainLayout);
	mainWindow->setCentralWidget(w);
	mainWindow->setGeometry(300, 300, 1200, mainLayout->sizeHint().height());
	mainWindow->setWindowTitle("ChronoPlotter");
	mainWindow->show();

	return a.exec();
}
