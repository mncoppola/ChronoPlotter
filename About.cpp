#include <QMessageBox>

#include "ChronoPlotter.h"
#include "About.h"

About::About ( QWidget *parent )
	: QWidget(parent)
{
	qDebug() << "About this app";

	QLabel *logo = new QLabel();
	QPixmap logoPix(":/images/logo.png");
	logo->setPixmap(logoPix.scaledToWidth(600, Qt::SmoothTransformation));

	QLabel *about1 = new QLabel();
	about1->setTextFormat(Qt::RichText);
	about1->setText(QString("<center><h1>Version %1</h1>By Michael Coppola<br>&#169; 2024 Precision Analytics LLC<p><a href=\"https://chronoplotter.com\">ChronoPlotter.com</a>").arg(CHRONOPLOTTER_VERSION));
	about1->setOpenExternalLinks(true);

	QLabel *about2 = new QLabel();
	about2->setTextFormat(Qt::RichText);
	about2->setText("<br><center>If you found this tool helpful, please consider donating to support the project!<h1><a href=\"https://chronoplotter.com/donate\">ChronoPlotter.com/donate</a></h1><br><a href=\"https://www.doctorswithoutborders.org/\">Doctors Without Borders</a><br><a href=\"https://www.navysealfoundation.org/\">The Navy SEAL Foundation</a><br><a href=\"https://eji.org/\">Equal Justice Initiative</a><br><a href=\"https://www.mskcc.org/\">Memorial Sloan Kettering Cancer Center</a>");
	about2->setOpenExternalLinks(true);

	QPushButton *legalButton = new QPushButton("Legal Notices", this);
	legalButton->setMinimumWidth(250);
	legalButton->setMaximumWidth(250);
	connect(legalButton, &QPushButton::released, this, &About::showLegal);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addStretch(1);
	layout->addWidget(logo);
	layout->setAlignment(logo, Qt::AlignHCenter);
	layout->addWidget(about1);
	layout->addSpacing(10);
	layout->addWidget(about2);
	layout->addStretch(3);
	layout->addWidget(legalButton, 0, Qt::AlignHCenter);

	setLayout(layout);
}

void About::showLegal ( void )
{
	#define LEGAL_NOTICES "ChronoPlotter (\"This software\") is intended for informational and educational usage only. This software visualizes and performs statistical analysis of load development data provided by the user but does not make or imply any load development recommendations based on that data. Users must always exercise extreme caution when performing testing related to load development, and must operate within safe load ranges published by component manufacturers at all times.\n\nPrecision Analytics LLC and the author(s) of this software disclaim any and all warranties and liabilities pertaining to usage of this software and the results provided by it. Users assume all risk, responsibilities, and liabilities arising from the usage or mis-usage of this software, including but not limited to any and all injuries, death, or losses or damages to property.\n\nThe author(s) of this software strive to provide accurate analysis of the user’s data, but cannot guarantee the absence of bugs in this software. This software is open source, and users should review this software’s source code to verify the accuracy of its calculations, algorithms, and reporting of those results prior to using this software for load development."

	QMessageBox::information(this, "Legal Notices", LEGAL_NOTICES);
}
