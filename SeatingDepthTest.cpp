#include "untar.h"
#include "miniz.h"
#include "ChronoPlotter.h"
#include "SeatingDepthTest.h"

using namespace SeatingDepth;

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

double SeatingDepthTest::calculateES ( QList<QPair<double, double> > coordinates )
{
	if ( coordinates.size() < 2 )
	{
		return qQNaN();
	}

	/*
	 * There's probably a smarter/more efficient way to calculate this, but you'll
	 * burn your barrel out before the algorithmic complexity becomes an issue.
	 */

	double extremeSpread = 0;

	int i;
	for ( i = 0; i < coordinates.size(); i++ )
	{
		QPair<double, double> coord1 = coordinates.at(i);
		//qDebug() << "Iter" << i;

		int j;
		for ( j = i + 1; j < coordinates.size(); j++ )
		{
			QPair<double, double> coord2 = coordinates.at(j);
			double es = sqrt( pow((coord2.first - coord1.first), 2) + pow(coord2.second - coord1.second, 2) );

			//qDebug() << "Calculated ES =" << es << "for coords" << coord1 << coord2;

			if ( es > extremeSpread )
			{
				extremeSpread = es;
				//qDebug() << "Updated ES to" << extremeSpread;
			}
		}
	}

	return extremeSpread;
}

double SeatingDepthTest::calculateXStdev ( QList<QPair<double, double> > coordinates )
{
	if ( coordinates.size() < 2 )
	{
		return qQNaN();
	}

	QList<double> xCoords;
	for ( int i = 0; i < coordinates.size(); i++ )
	{
		xCoords.append(coordinates.at(i).first);
	}

	return sampleStdev(xCoords);
}

double SeatingDepthTest::calculateYStdev ( QList<QPair<double, double> > coordinates )
{
	if ( coordinates.size() < 2 )
	{
		return qQNaN();
	}

	QList<double> yCoords;
	for ( int i = 0; i < coordinates.size(); i++ )
	{
		yCoords.append(coordinates.at(i).second);
	}

	return sampleStdev(yCoords);
}

double SeatingDepthTest::calculateRSD ( QList<QPair<double, double> > coordinates )
{
	if ( coordinates.size() < 2 )
	{
		return qQNaN();
	}

	QList<double> xCoords;
	QList<double> yCoords;
	for ( int i = 0; i < coordinates.size(); i++ )
	{
		xCoords.append(coordinates.at(i).first);
		yCoords.append(coordinates.at(i).second);
	}

	double radialStdev = sqrt( pow(sampleStdev(xCoords), 2) + pow(sampleStdev(yCoords), 2) );

	return radialStdev;
}

double SeatingDepthTest::pairSumX ( double lhs, const QPair<double, double> rhs )
{
	return lhs + rhs.first;
}

double SeatingDepthTest::pairSumY ( double lhs, const QPair<double, double> rhs )
{
	return lhs + rhs.second;
}

double SeatingDepthTest::calculateMR ( QList<QPair<double, double> > coordinates )
{
	double meanRadius = 0;

	if ( coordinates.size() < 2 )
	{
		return qQNaN();
	}

	int totalShots = coordinates.size();
	double xMean = std::accumulate(coordinates.begin(), coordinates.end(), 0.0, pairSumX) / static_cast<double>(totalShots);
	double yMean = std::accumulate(coordinates.begin(), coordinates.end(), 0.0, pairSumY) / static_cast<double>(totalShots);

	QList<double> r;
	int i;
	for ( i = 0; i < coordinates.size(); i++ )
	{
		r.append(sqrt( pow((coordinates.at(i).first - xMean), 2) + pow((coordinates.at(i).second - yMean), 2) ));
	}

	meanRadius = std::accumulate(r.begin(), r.end(), 0.0) / static_cast<double>(totalShots);

	return meanRadius;
}

void SeatingDepthTest::selectShotMarkerFile ( bool state )
{
	qDebug() << "selectShotMarkerFile state =" << state;

	qDebug() << "Previous directory:" << prevShotMarkerDir;

	QString path = QFileDialog::getOpenFileName(this, "Select file", prevShotMarkerDir, "ShotMarker files (*.csv *.tar)");
	prevShotMarkerDir = path;

	qDebug() << "Selected file:" << path;

	if ( path.isEmpty() )
	{
		qDebug() << "User didn't select a file, bail";
		return;
	}

	seatingSeriesData.clear();

	/*
	 * ShotMarker records all of its series data in a single .CSV file
	 */

	QList<SeatingSeries *> allSeries;

	if ( path.endsWith(".tar") )
	{
		qDebug() << "ShotMarker .tar bundle";

		allSeries = ExtractShotMarkerSeriesTar(path);
	}
	else
	{
		qDebug() << "ShotMarker .csv export";

		QFile csvFile(path);
		csvFile.open(QIODevice::ReadOnly);
		QTextStream csv(&csvFile);

		allSeries = ExtractShotMarkerSeriesCsv(csv);

		csvFile.close();
	}

	qDebug() << "Got allSeries with size" << allSeries.size();

	if ( ! allSeries.empty() )
	{
		qDebug() << "Detected ShotMarker file";

		for ( int i = 0; i < allSeries.size(); i++ )
		{
			SeatingSeries *series = allSeries.at(i);

			series->enabled = new QCheckBox();
			series->enabled->setChecked(true);

			series->cartridgeLength = new QDoubleSpinBox();
			series->cartridgeLength->setDecimals(3);
			series->cartridgeLength->setSingleStep(0.001);
			series->cartridgeLength->setMinimumWidth(100);
			series->cartridgeLength->setMaximumWidth(100);

			/*
			 * ShotMarker internally records shot coordinates in millimeters (at least it appears to, from studying its file formats). String
			 * export files (tar/JSON) record coordinates in mm with one decimal place. System export files (CSV) contain mm, inches, MOA, and
			 * mils, but for some reason round mm to whole integers. Inches are the most precise measurement available in CSV files, but are
			 * rounded to two decimal places as well. This means group size calculations may be slightly different for the same string between
			 * .tar and .CSV files, since we're going to use highest precision values when they're available.
			 *
			 * We'd like to provide the user the ability to graph all group size calculations (ES, RSD, MR, etc.) with all units. The simplest
			 * way is to just perform every calculation upfront and save the results. This saves a ton of complexity (and opportunity for bugs),
			 * and the performance hit is pretty negligible.
			 *
			 * Iterate through each unit and perform each group size calculation, starting from inches. The results are then stored in QLists,
			 * where each index correlates to the index constants used in groupUnits.
			 */

			/* Source coordinates are already in inches, perform calculations directly */

			series->extremeSpread.append(calculateES(series->coordinates));
			series->extremeSpread_sighters.append(calculateES(series->coordinates_sighters));
			series->yStdev.append(calculateYStdev(series->coordinates));
			series->yStdev_sighters.append(calculateYStdev(series->coordinates_sighters));
			series->xStdev.append(calculateXStdev(series->coordinates));
			series->xStdev_sighters.append(calculateXStdev(series->coordinates_sighters));
			series->radialStdev.append(calculateRSD(series->coordinates));
			series->radialStdev_sighters.append(calculateRSD(series->coordinates_sighters));
			series->meanRadius.append(calculateMR(series->coordinates));
			series->meanRadius_sighters.append(calculateMR(series->coordinates_sighters));

			/* Convert inches to MOA */

			// C++ is tricky here. If we don't cast targetDistance or 100 to a double, then calculations like 650 / 100 will return 6 instead of 6.5!
			series->extremeSpread.append( series->extremeSpread.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->extremeSpread_sighters.append( series->extremeSpread_sighters.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->yStdev.append( series->yStdev.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->yStdev_sighters.append( series->yStdev_sighters.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->xStdev.append( series->xStdev.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->xStdev_sighters.append( series->xStdev_sighters.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->radialStdev.append( series->radialStdev.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->radialStdev_sighters.append( series->radialStdev_sighters.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->meanRadius.append( series->meanRadius.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );
			series->meanRadius_sighters.append( series->meanRadius_sighters.at(INCH) / (1.047 * ((double)series->targetDistance / (double)100)) );

			/* Convert inches to centimeters, then perform calculations */

			QList<QPair<double, double> > coordinatesCm;
			for ( int i = 0; i < series->coordinates.size(); i++ )
			{
				// convert inches to cm
				coordinatesCm.append( QPair<double, double>(series->coordinates.at(i).first * 2.54, series->coordinates.at(i).second * 2.54) );
			}

			QList<QPair<double, double> > coordinatesCm_sighters;
			for ( int i = 0; i < series->coordinates_sighters.size(); i++ )
			{
				// convert inches to cm
				coordinatesCm_sighters.append( QPair<double, double>(series->coordinates_sighters.at(i).first * 2.54, series->coordinates_sighters.at(i).second * 2.54) );
			}

			series->extremeSpread.append(calculateES(coordinatesCm));
			series->extremeSpread_sighters.append(calculateES(coordinatesCm_sighters));
			series->yStdev.append(calculateYStdev(coordinatesCm));
			series->yStdev_sighters.append(calculateYStdev(coordinatesCm_sighters));
			series->xStdev.append(calculateXStdev(coordinatesCm));
			series->xStdev_sighters.append(calculateXStdev(coordinatesCm_sighters));
			series->radialStdev.append(calculateRSD(coordinatesCm));
			series->radialStdev_sighters.append(calculateRSD(coordinatesCm_sighters));
			series->meanRadius.append(calculateMR(coordinatesCm));
			series->meanRadius_sighters.append(calculateMR(coordinatesCm_sighters));

			/* Convert inches to mils */

			// mils = target distance (converted from yards to inches), divided by 1000. What elegance!
			series->extremeSpread.append( series->extremeSpread.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->extremeSpread_sighters.append( series->extremeSpread_sighters.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->yStdev.append( series->yStdev.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->yStdev_sighters.append( series->yStdev_sighters.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->xStdev.append( series->xStdev.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->xStdev_sighters.append( series->xStdev_sighters.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->radialStdev.append( series->radialStdev.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->radialStdev_sighters.append( series->radialStdev_sighters.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->meanRadius.append( series->meanRadius.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );
			series->meanRadius_sighters.append( series->meanRadius_sighters.at(INCH) / (((double)series->targetDistance * 3 * 12) / (double)1000) );

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

			if ( includeSightersCheckBox->isChecked() )
			{
				if ( groupMeasurementType->currentIndex() == ES )
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->extremeSpread_sighters.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}
				else if ( groupMeasurementType->currentIndex() == YSTDEV )
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->yStdev_sighters.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}
				else if ( groupMeasurementType->currentIndex() == XSTDEV )
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->xStdev_sighters.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}
				else if ( groupMeasurementType->currentIndex() == RSD )
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->radialStdev_sighters.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}
				else
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->meanRadius_sighters.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}

				qDebug() << "Series '" << series->name->text() << "' has ES" << series->extremeSpread_sighters << ", RSD" << series->radialStdev_sighters << ", and MR" << series->meanRadius_sighters << "(with sighters) at target distance" << series->targetDistance;
			}
			else
			{
				if ( groupMeasurementType->currentIndex() == ES )
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->extremeSpread.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}
				else if ( groupMeasurementType->currentIndex() == YSTDEV )
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->yStdev.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}
				else if ( groupMeasurementType->currentIndex() == XSTDEV )
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->xStdev.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}
				else if ( groupMeasurementType->currentIndex() == RSD )
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->radialStdev.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}
				else
				{
					series->groupSizeLabel = new QLabel(QString("%1 %2").arg(series->meanRadius.at(groupUnits->currentIndex()), 0, 'f', 3).arg(groupUnits2));
				}

				qDebug() << "Series '" << series->name->text() << "' has ES" << series->extremeSpread << ", RSD" << series->radialStdev << ", and MR" << series->meanRadius << "at target distance" << series->targetDistance;
			}

			seatingSeriesData.append(series);
		}
	}

	/* We're finished parsing the file */

	if ( seatingSeriesData.empty() )
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

		// Convenience function to disable any series with too few shots to calculate
		updateDisplayedData();
	}
}

QList<SeatingSeries *> SeatingDepthTest::ExtractShotMarkerSeriesTar ( QString path )
{
	QList<SeatingSeries *> allSeries;
	SeatingSeries *curSeries = new SeatingSeries();
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

		curSeries = new SeatingSeries();
		curSeries->isValid = false;
		curSeries->seriesNum = seriesNum;
		qDebug() << "name =" << jsonObj["name"].toString();
		curSeries->name = new QLabel(jsonObj["name"].toString() + QString(" (%1%2)").arg(jsonObj["dist"].toInt()).arg(jsonObj["dist_unit"].toString()));
		curSeries->deleted = false;
		QDateTime dateTime;
		dateTime.setMSecsSinceEpoch(jsonObj["ts"].toVariant().toULongLong());
		curSeries->firstDate = dateTime.date().toString(Qt::TextDate);
		curSeries->firstTime = dateTime.time().toString(Qt::TextDate);

		qDebug() << "setting date =" << curSeries->firstDate << " time =" << curSeries->firstTime << "from ts" << jsonObj["ts"].toVariant().toULongLong();

		if ( jsonObj["dist_unit"].toString() == "y" )
		{
			// distance is in yards already
			curSeries->targetDistance = jsonObj["dist"].toInt();
		}
		else
		{
			// convert from meters to yards
			curSeries->targetDistance = jsonObj["dist"].toInt() * 1.0936133; // the result is cast to an int
		}

		foreach ( const QJsonValue& shot, jsonObj["shots"].toArray() )
		{
			// convert from millimeters to inches
			double xMm = shot["x"].toDouble() + jsonObj["cal_x"].toDouble();
			double yMm = shot["y"].toDouble() + jsonObj["cal_y"].toDouble();

			if ( shot["hidden"].toBool() )
			{
				// hidden shot

				qDebug() << "ignoring hidden shot" << shot["display_text"];
			}
			else if ( shot["sighter"].toBool() )
			{
				// sighter shot

				qDebug() << "adding coords (sighter)" << QPair<double,double>(xMm / 25.4, yMm / 25.4) << "from mm:" << QPair<double,double>(xMm, yMm);
				curSeries->coordinates_sighters.append(QPair<double,double>(xMm / 25.4, yMm / 25.4));
			}
			else
			{
				// shot for record

				qDebug() << "adding coords" << QPair<double,double>(xMm / 25.4, yMm / 25.4) << "from mm:" << QPair<double,double>(xMm, yMm);
				curSeries->coordinates_sighters.append(QPair<double,double>(xMm / 25.4, yMm / 25.4));
				curSeries->coordinates.append(QPair<double,double>(xMm / 25.4, yMm / 25.4));
			}

		}

		// Finish parsing the current series.
		qDebug() << "End of JSON";

		if ( (curSeries->coordinates.size() > 0) || (curSeries->coordinates_sighters.size() > 0) )
		{
			qDebug() << "Adding curSeries to allSeries";

			allSeries.append(curSeries);
		}

		free(destBuf);

		seriesNum++;
	}

	return allSeries;
}

QList<SeatingSeries *> SeatingDepthTest::ExtractShotMarkerSeriesCsv ( QTextStream &csv )
{
	QList<SeatingSeries *> allSeries;
	SeatingSeries *curSeries = new SeatingSeries();
	curSeries->isValid = false;
	curSeries->deleted = false;
	curSeries->seriesNum = -1;

	int i = 0;
	int seriesNum = 1;
	while ( ! csv.atEnd() )
	{
		QString line = csv.readLine();

		// ShotMarker uses comma (,) as delimeter
		QStringList rows(line.split(","));

		// Trim whitespace from cells
		QMutableStringListIterator it(rows);
		while ( it.hasNext() )
		{
			it.next();
			it.setValue(it.value().trimmed());
		}

		qDebug() << "Line" << i << ":" << rows;

		// Validate the first row header
		if ( i == 0 )
		{
			if ( (rows.size() >= 1) && (rows.at(0).contains("ShotMarker Archived Data")) )
			{
				qDebug() << "Found the ShotMarker header";
			}
			else
			{
				qDebug() << "File doesn't have the ShotMarker header, bailing";
				return allSeries;
			}
		}

		if ( rows.size() >= 5 )
		{
			// Check if first cell is a date, signifying the beginning of a new series
			QDate seriesDate;
			seriesDate = QDate::fromString(rows.at(0), "MMM d yyyy");
			if ( seriesDate.isValid() )
			{
				 // End the previous series (if necessary) and start a new one

				if ( (curSeries->coordinates.size() > 0) || (curSeries->coordinates_sighters.size() > 0) )
				{
					qDebug() << "Adding curSeries to allSeries";

					allSeries.append(curSeries);
				}

				qDebug() << "Beginning new series";

				curSeries = new SeatingSeries();
				curSeries->isValid = false;
				curSeries->seriesNum = seriesNum;
				curSeries->name = new QLabel(rows.at(1) + QString(" (%1)").arg(rows.at(3)));
				curSeries->deleted = false;
				curSeries->firstDate = rows.at(0);

				if ( rows.at(3).right(1) == "y" )
				{
					// distance is in yards already
					QString distance = rows.at(3);
					distance.chop(1);
					curSeries->targetDistance = distance.toInt(NULL, 10);
				}
				else
				{
					// convert from meters to yards
					QString distance = rows.at(3);
					distance.chop(1);
					curSeries->targetDistance = distance.toInt(NULL, 10) * 1.0936133; // the result is casted to an int
				}

				seriesNum++;
			}
			else if ( rows.size() >= 17 )
			{
				// This is either a row containing headers, shot data, or avg/SD summary data

				QTime seriesTime;
				seriesTime = QTime::fromString(rows.at(1), "h:mm:ss ap");
				if ( seriesTime.isValid() )
				{
					// Rows with a time in the second cell are shot data

					if ( curSeries->firstTime.isNull() )
					{
						curSeries->firstTime = rows.at(1);
						qDebug() << "firstTime =" << curSeries->firstTime;
					}

					if ( rows.at(3).contains("hidden") )
					{
						// hidden shots

						qDebug() << "ignoring hidden shot" << rows.at(2);
					}
					else if ( rows.at(3).contains("sighter") )
					{
						// sighter shots

						qDebug() << "adding coords (sighter)" << QPair<double,double>(rows.at(7).toDouble(), rows.at(8).toDouble());

						curSeries->coordinates_sighters.append(QPair<double,double>(rows.at(7).toDouble(), rows.at(8).toDouble()));
					}
					else
					{
						// all other shots for record

						qDebug() << "adding coords" << QPair<double,double>(rows.at(7).toDouble(), rows.at(8).toDouble());

						curSeries->coordinates_sighters.append(QPair<double,double>(rows.at(7).toDouble(), rows.at(8).toDouble()));
						curSeries->coordinates.append(QPair<double,double>(rows.at(7).toDouble(), rows.at(8).toDouble()));
					}
				}
			}
		}
		else
		{
			qDebug() << "Skipping line";
		}

		i++;
	}

	// End of the file. Finish parsing the current series.
	qDebug() << "End of file";

	if ( (curSeries->coordinates.size() > 0) || (curSeries->coordinates_sighters.size() > 0) )
	{
		qDebug() << "Adding curSeries to allSeries";

		allSeries.append(curSeries);
	}

	return allSeries;
}

AutofillDialog::AutofillDialog ( SeatingDepthTest *main, QDialog *parent )
	: QDialog(parent)
{
	qDebug() << "Autofill dialog";

	setWindowTitle("Auto-fill cartridge lengths");

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &AutofillDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &AutofillDialog::reject);

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

AutofillValues *AutofillDialog::getValues ( void )
{
	AutofillValues *values = new AutofillValues();
	values->startingLength = startingLength->value();
	values->interval = interval->value();
	values->increasing = direction->currentData().value<bool>();

	return values;
}

void SeatingDepthTest::autofillClicked ( bool state )
{
	qDebug() << "autofillClicked state =" << state;

	AutofillDialog *dialog = new AutofillDialog(this);
	int result = dialog->exec();

	qDebug() << "dialog result:" << result;

	if ( result )
	{
		qDebug() << "User OK'd dialog";

		AutofillValues *values = dialog->getValues();

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

	connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesManualCheckBoxChanged(int)));
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

	QHBoxLayout *cartridgeLengthLayout = new QHBoxLayout();
	cartridgeLengthLayout->addWidget(series->cartridgeLength);
	cartridgeLengthLayout->addStretch(0);
	seatingSeriesGrid->addLayout(cartridgeLengthLayout, numRows - 1, 2);

	series->groupSize = new QDoubleSpinBox();
	series->groupSize->setDecimals(3);
	series->groupSize->setSingleStep(0.001);
	series->groupSize->setMinimumWidth(100);
	series->groupSize->setMaximumWidth(100);

	QHBoxLayout *groupSizeLayout = new QHBoxLayout();
	groupSizeLayout->addWidget(series->groupSize);
	groupSizeLayout->addStretch(0);
	seatingSeriesGrid->addLayout(groupSizeLayout, numRows - 1, 3);

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

	graphPreview = NULL;
	prevShotMarkerDir = QDir::homePath();
	prevSaveDir = QDir::homePath();

	/* Left panel */

	QVBoxLayout *leftLayout = new QVBoxLayout();

	QLabel *selectLabel = new QLabel("Select input type\nto populate series data\n");
	selectLabel->setAlignment(Qt::AlignCenter);

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
	QGroupBox *seatingGroupBox = new QGroupBox("Seating depth data:");
	seatingGroupBox->setLayout(stackedLayout);

	leftLayout->addWidget(seatingGroupBox);

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

	// create the header object that cartridgeMeasurementType controls
	headerLengthType = new QLabel(cartridgeMeasurementType->currentText());

	cartridgeUnits = new QComboBox();
	cartridgeUnits->addItem("inches (in)");
	cartridgeUnits->addItem("centimeters (cm)");
	optionsFormLayout->addRow(new QLabel("Cartridge units:"), cartridgeUnits);

	groupMeasurementType = new QComboBox();
	groupMeasurementType->addItem("Extreme Spread (ES)");
	groupMeasurementType->addItem("Y Stdev");
	groupMeasurementType->addItem("X Stdev");
	groupMeasurementType->addItem("Radial Stdev (RSD)");
	groupMeasurementType->addItem("Mean Radius (MR)");
	connect(groupMeasurementType, SIGNAL(activated(int)), this, SLOT(groupMeasurementTypeChanged(int)));
	optionsFormLayout->addRow(new QLabel("Group size measurement:"), groupMeasurementType);

	// create the header object that groupMeasurementType controls
	const char *headerGroupType2;
	if ( groupMeasurementType->currentIndex() == ES )
	{
		headerGroupType2 = "Group Size (ES)";
	}
	else if ( groupMeasurementType->currentIndex() == YSTDEV )
	{
		headerGroupType2 = "Group Size (Y Stdev)";
	}
	else if ( groupMeasurementType->currentIndex() == XSTDEV )
	{
		headerGroupType2 = "Group Size (X Stdev)";
	}
	else if ( groupMeasurementType->currentIndex() == RSD )
	{
		headerGroupType2 = "Group Size (RSD)";
	}
	else
	{
		headerGroupType2 = "Group Size (MR)";
	}
	headerGroupType = new QLabel(headerGroupType2);

	groupUnits = new QComboBox();
	groupUnits->addItem("inches (in)");
	groupUnits->addItem("minutes (MOA)");
	groupUnits->addItem("centimeters (cm)");
	groupUnits->addItem("milliradians (mil)");
	optionsFormLayout->addRow(new QLabel("Group size units:"), groupUnits);

	xAxisSpacing = new QComboBox();
	xAxisSpacing->addItem("Proportional");
	xAxisSpacing->addItem("Constant");
	connect(xAxisSpacing, SIGNAL(currentIndexChanged(int)), this, SLOT(xAxisSpacingChanged(int)));
	optionsFormLayout->addRow(new QLabel("X-axis spacing:"), xAxisSpacing);

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
	trendLayout->addWidget(trendLabel, 1);
	trendLineType = new QComboBox();
	trendLineType->addItem("solid line");
	trendLineType->addItem("dashed line");
	trendLineType->setCurrentIndex(1);
	trendLineType->setEnabled(false);
	trendLayout->addWidget(trendLineType);
	optionsLayout->addLayout(trendLayout);

	QHBoxLayout *includeSightersLayout = new QHBoxLayout();
	includeSightersCheckBox = new QCheckBox();
	includeSightersCheckBox->setChecked(false);
	connect(includeSightersCheckBox, SIGNAL(clicked(bool)), this, SLOT(includeSightersCheckBoxChanged(bool)));
	includeSightersLayout->addWidget(includeSightersCheckBox, 0);
	QLabel *includeSightersLabel = new QLabel("Include sighters");
	includeSightersLabel->setFixedHeight(trendLineType->sizeHint().height());
	includeSightersLayout->addWidget(includeSightersLabel, 1);
	optionsLayout->addLayout(includeSightersLayout);

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

void SeatingDepthTest::loadNewShotData ( bool state )
{
	qDebug() << "loadNewShotData state =" << state;

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Load new data", "Are you sure you want to load new shot data?\n\nThis will clear your current work.", QMessageBox::Yes | QMessageBox::Cancel);

	if ( reply == QMessageBox::Yes )
	{
		qDebug() << "User said yes";

		// Hide the shot data screen. This returns to the initial screen to choose a new shot data file.
		stackedWidget->removeWidget(scrollWidget);

		// Delete the loaded shot data
		seatingSeriesData.clear();

		// Disconnect the group measurement type signal (used in imported data entry), if necessary
		disconnect(groupMeasurementType, SIGNAL(activated(int)), this, SLOT(importedGroupMeasurementTypeChanged(int)));
		disconnect(groupUnits, SIGNAL(activated(int)), this, SLOT(importedGroupUnitsChanged(int)));
	}
	else
	{
		qDebug() << "User said cancel";
	}
}

static bool SeatingSeriesComparator ( SeatingSeries *one, SeatingSeries *two )
{
	return (one->seriesNum < two->seriesNum);
}

void SeatingDepthTest::DisplaySeriesData ( void )
{
	// Sort the list by series number
	std::sort(seatingSeriesData.begin(), seatingSeriesData.end(), SeatingSeriesComparator);

	// If we already have series data displayed, clear it out first. This call is a no-op if scrollWidget is not already added to stackedWidget.
	stackedWidget->removeWidget(scrollWidget);

	QVBoxLayout *scrollLayout = new QVBoxLayout();

	// Wrap grid in a widget to make the grid scrollable
	QWidget *scrollAreaWidget = new QWidget();

	seatingSeriesGrid = new QGridLayout(scrollAreaWidget);
	seatingSeriesGrid->setColumnStretch(0, 0);
	seatingSeriesGrid->setColumnStretch(1, 1);
	seatingSeriesGrid->setColumnStretch(2, 2);
	seatingSeriesGrid->setColumnStretch(3, 3);
	seatingSeriesGrid->setColumnStretch(4, 3);
	seatingSeriesGrid->setHorizontalSpacing(25);

	scrollArea = new QScrollArea();
	scrollArea->setWidget(scrollAreaWidget);
	scrollArea->setWidgetResizable(true);

	scrollLayout->addWidget(scrollArea);

	/* Create utilities toolbar under scroll area */

	QPushButton *loadNewButton = new QPushButton("Load new shot data file");
	connect(loadNewButton, SIGNAL(clicked(bool)), this, SLOT(loadNewShotData(bool)));
	loadNewButton->setMinimumWidth(225);
	loadNewButton->setMaximumWidth(225);

	QPushButton *autofillButton = new QPushButton("Auto-fill cartridge lengths");
	connect(autofillButton, SIGNAL(clicked(bool)), this, SLOT(autofillClicked(bool)));
	autofillButton->setMinimumWidth(225);
	autofillButton->setMaximumWidth(225);

	QHBoxLayout *utilitiesLayout = new QHBoxLayout();
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
	seatingSeriesGrid->addWidget(headerCheckBox, 0, 0);

	/*
	 * Connect signals to update all calculations in the Group Size column when the user selects a new group measurement type (ES, RSD, MR, etc.) or
	 * measurement unit (in, MOA, cm, mil, etc.). This is only done for imported shot data, where the group sizes are QLabels and not user-controllable.
	 */

	connect(groupMeasurementType, SIGNAL(activated(int)), this, SLOT(importedGroupMeasurementTypeChanged(int)));
	connect(groupUnits, SIGNAL(activated(int)), this, SLOT(importedGroupUnitsChanged(int)));

	/* Headers for series data */

	QLabel *headerNumber = new QLabel("Series Name");
	seatingSeriesGrid->addWidget(headerNumber, 0, 1, Qt::AlignVCenter);

	// these two objects were previously created
	seatingSeriesGrid->addWidget(headerLengthType, 0, 2, Qt::AlignVCenter);
	seatingSeriesGrid->addWidget(headerGroupType, 0, 3, Qt::AlignVCenter);

	QLabel *headerDate = new QLabel("Series Date");
	seatingSeriesGrid->addWidget(headerDate, 0, 4, Qt::AlignVCenter);

	for ( int i = 0; i < seatingSeriesData.size(); i++ )
	{
		SeatingSeries *series = seatingSeriesData.at(i);

		connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesCheckBoxChanged(int)));

		seatingSeriesGrid->addWidget(series->enabled, i + 1, 0);
		seatingSeriesGrid->addWidget(series->name, i + 1, 1, Qt::AlignVCenter);

		QHBoxLayout *cartridgeLengthLayout = new QHBoxLayout();
		cartridgeLengthLayout->addWidget(series->cartridgeLength);
		cartridgeLengthLayout->addStretch(0);
		seatingSeriesGrid->addLayout(cartridgeLengthLayout, i + 1, 2);

		seatingSeriesGrid->addWidget(series->groupSizeLabel, i + 1, 3, Qt::AlignVCenter);

		QLabel *datetimeLabel = new QLabel(QString("%1 %2").arg(series->firstDate).arg(series->firstTime));
		seatingSeriesGrid->addWidget(datetimeLabel, i + 1, 4, Qt::AlignVCenter);

		seatingSeriesGrid->setRowMinimumHeight(0, series->cartridgeLength->sizeHint().height());
	}

	// Add an empty stretch row at the end for proper spacing
	seatingSeriesGrid->setRowStretch(seatingSeriesData.size() + 1, 1);

	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);
}

void SeatingDepthTest::manualDataEntry ( bool state )
{
	qDebug() << "manualDataEntry state =" << state;

	// If we already have series data displayed, clear it out first. This call is a no-op if scrollWidget is not already added to stackedWidget.
	stackedWidget->removeWidget(scrollWidget);

	QVBoxLayout *scrollLayout = new QVBoxLayout();

	// Wrap grid in a widget to make the grid scrollable
	QWidget *scrollAreaWidget = new QWidget();

	seatingSeriesGrid = new QGridLayout(scrollAreaWidget);
	seatingSeriesGrid->setColumnStretch(0, 0);
	seatingSeriesGrid->setColumnStretch(1, 1);
	seatingSeriesGrid->setColumnStretch(2, 2);
	seatingSeriesGrid->setColumnStretch(3, 2);
	seatingSeriesGrid->setColumnStretch(4, 3);
	seatingSeriesGrid->setHorizontalSpacing(25);

	scrollArea = new QScrollArea();
	scrollArea->setWidget(scrollAreaWidget);
	scrollArea->setWidgetResizable(true);

	scrollLayout->addWidget(scrollArea);

	/* Create utilities toolbar under scroll area */

	addNewButton = new QPushButton("Add new group");
	addNewButton->setStyleSheet("font-weight: bold");
	connect(addNewButton, SIGNAL(clicked(bool)), this, SLOT(addNewClicked(bool)));
	addNewButton->setMinimumWidth(225);
	addNewButton->setMaximumWidth(225);

	QPushButton *loadNewButton = new QPushButton("Load new shot data file");
	connect(loadNewButton, SIGNAL(clicked(bool)), this, SLOT(loadNewShotData(bool)));
	loadNewButton->setMinimumWidth(225);
	loadNewButton->setMaximumWidth(225);

	QPushButton *autofillButton = new QPushButton("Auto-fill cartridge lengths");
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
	seatingSeriesGrid->addWidget(headerCheckBox, 0, 0);

	/* Headers for series data */

	QLabel *headerNumber = new QLabel("Series Name");
	seatingSeriesGrid->addWidget(headerNumber, 0, 1, Qt::AlignVCenter);

	// this object was previously created
	seatingSeriesGrid->addWidget(headerLengthType, 0, 2, Qt::AlignVCenter);

	const char *headerGroupType2;
	if ( groupMeasurementType->currentIndex() == ES )
	{
		headerGroupType2 = "Group Size (ES)";
	}
	else if ( groupMeasurementType->currentIndex() == YSTDEV )
	{
		headerGroupType2 = "Group Size (Y Stdev)";
	}
	else if ( groupMeasurementType->currentIndex() == XSTDEV )
	{
		headerGroupType2 = "Group Size (X Stdev)";
	}
	else if ( groupMeasurementType->currentIndex() == RSD )
	{
		headerGroupType2 = "Group Size (RSD)";
	}
	else
	{
		headerGroupType2 = "Group Size (MR)";
	}

	headerGroupType = new QLabel(headerGroupType2);
	seatingSeriesGrid->addWidget(headerGroupType, 0, 3, Qt::AlignVCenter);
	QLabel *headerDelete = new QLabel("");
	seatingSeriesGrid->addWidget(headerDelete, 0, 4, Qt::AlignVCenter);

	/* Create initial row */

	SeatingSeries *series = new SeatingSeries();

	series->deleted = false;

	series->enabled = new QCheckBox();
	series->enabled->setChecked(true);

	connect(series->enabled, SIGNAL(stateChanged(int)), this, SLOT(seriesManualCheckBoxChanged(int)));
	seatingSeriesGrid->addWidget(series->enabled, 1, 0);

	series->seriesNum = 1;

	series->name = new QLabel("Series 1");
	seatingSeriesGrid->addWidget(series->name, 1, 1);

	series->cartridgeLength = new QDoubleSpinBox();
	series->cartridgeLength->setDecimals(3);
	series->cartridgeLength->setSingleStep(0.001);
	series->cartridgeLength->setMinimumWidth(100);
	series->cartridgeLength->setMaximumWidth(100);

	QHBoxLayout *cartridgeLengthLayout = new QHBoxLayout();
	cartridgeLengthLayout->addWidget(series->cartridgeLength);
	cartridgeLengthLayout->addStretch(0);
	seatingSeriesGrid->addLayout(cartridgeLengthLayout, 1, 2);

	series->groupSize = new QDoubleSpinBox();
	series->groupSize->setDecimals(3);
	series->groupSize->setSingleStep(0.001);
	series->groupSize->setMinimumWidth(100);
	series->groupSize->setMaximumWidth(100);

	QHBoxLayout *groupSizeLayout = new QHBoxLayout();
	groupSizeLayout->addWidget(series->groupSize);
	groupSizeLayout->addStretch(0);
	seatingSeriesGrid->addLayout(groupSizeLayout, 1, 3);

	series->deleteButton = new QPushButton();
	connect(series->deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteClicked(bool)));
	series->deleteButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
	series->deleteButton->setFixedSize(series->deleteButton->minimumSizeHint());
	seatingSeriesGrid->addWidget(series->deleteButton, 1, 4, Qt::AlignLeft);

	seatingSeriesGrid->setRowMinimumHeight(0, series->cartridgeLength->sizeHint().height());

	seatingSeriesData.append(series);

	// Add an empty stretch row at the end for proper spacing
	seatingSeriesGrid->setRowStretch(seatingSeriesGrid->rowCount(), 1);
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
			QWidget *cartridgeLength = seatingSeriesGrid->itemAtPosition(i, 2)->layout()->itemAt(0)->widget();
			QWidget *groupSizeLabel = seatingSeriesGrid->itemAtPosition(i, 3)->widget();
			QWidget *seriesDate = seatingSeriesGrid->itemAtPosition(i, 4)->widget();

			if ( checkBox->isChecked() )
			{
				qDebug() << "Grid row" << i << "was checked";

				seriesName->setEnabled(true);
				cartridgeLength->setEnabled(true);
				groupSizeLabel->setEnabled(true);
				seriesDate->setEnabled(true);
			}
			else
			{
				qDebug() << "Grid row" << i << "was unchecked";

				seriesName->setEnabled(false);
				cartridgeLength->setEnabled(false);
				groupSizeLabel->setEnabled(false);
				seriesDate->setEnabled(false);
			}

			return;
		}
	}
}

void SeatingDepthTest::seriesManualCheckBoxChanged ( int state )
{
	QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());

	qDebug() << "seriesManualCheckBoxChanged state =" << state;

	// We have the checkbox object, now locate its row in the grid
	for ( int i = 0; i < seatingSeriesGrid->rowCount() - 1; i++ )
	{
		QWidget *rowCheckBox = seatingSeriesGrid->itemAtPosition(i, 0)->widget();
		if ( checkBox == rowCheckBox )
		{
			QWidget *seriesName = seatingSeriesGrid->itemAtPosition(i, 1)->widget();
			QWidget *cartridgeLength = seatingSeriesGrid->itemAtPosition(i, 2)->layout()->itemAt(0)->widget();
			QWidget *groupSize = seatingSeriesGrid->itemAtPosition(i, 3)->layout()->itemAt(0)->widget();
			QWidget *deleteButton = seatingSeriesGrid->itemAtPosition(i, 4)->widget();

			if ( checkBox->isChecked() )
			{
				qDebug() << "Grid row" << i << "was checked";

				seriesName->setEnabled(true);
				cartridgeLength->setEnabled(true);
				groupSize->setEnabled(true);
				deleteButton->setEnabled(true);;
			}
			else
			{
				qDebug() << "Grid row" << i << "was unchecked";

				seriesName->setEnabled(false);
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
			if ( checkBox->isEnabled() )
			{
				checkBox->setChecked(true);
			}
		}
	}
	else
	{
		qDebug() << "Header checkbox was unchecked";

		for ( int i = 0; i < seatingSeriesGrid->rowCount() - 1; i++ )
		{
			QCheckBox *checkBox = qobject_cast<QCheckBox *>(seatingSeriesGrid->itemAtPosition(i, 0)->widget());
			if ( checkBox->isEnabled() )
			{
				checkBox->setChecked(false);
			}
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

void SeatingDepthTest::updateDisplayedData ( void )
{
	// Update the series data to reflect any changes. The user could've either included/excluded sighters
	// or changed the group measurement type.

	int index = groupMeasurementType->currentIndex();

	const char *groupMeasurementType2;
	if ( index == ES )
	{
		groupMeasurementType2 = "ES";
	}
	else if ( index == YSTDEV )
	{
		groupMeasurementType2 = "Y Stdev";
	}
	else if ( index == XSTDEV )
	{
		groupMeasurementType2 = "X Stdev";
	}
	else if ( index == RSD )
	{
		groupMeasurementType2 = "RSD";
	}
	else
	{
		groupMeasurementType2 = "MR";
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

	for ( int i = 0; i < seatingSeriesData.size(); i++ )
	{
		SeatingSeries *series = seatingSeriesData.at(i);

		double groupSize;
		double groupSize_sighters;
		if ( index == ES )
		{
			groupSize = series->extremeSpread.at(groupUnits->currentIndex());
			groupSize_sighters = series->extremeSpread_sighters.at(groupUnits->currentIndex());
		}
		else if ( index == YSTDEV )
		{
			groupSize = series->yStdev.at(groupUnits->currentIndex());
			groupSize_sighters = series->yStdev_sighters.at(groupUnits->currentIndex());
		}
		else if ( index == XSTDEV )
		{
			groupSize = series->xStdev.at(groupUnits->currentIndex());
			groupSize_sighters = series->xStdev_sighters.at(groupUnits->currentIndex());
		}
		else if ( index == RSD )
		{
			groupSize = series->radialStdev.at(groupUnits->currentIndex());
			groupSize_sighters = series->radialStdev_sighters.at(groupUnits->currentIndex());
		}
		else
		{
			groupSize = series->meanRadius.at(groupUnits->currentIndex());
			groupSize_sighters = series->meanRadius_sighters.at(groupUnits->currentIndex());
		}


		if ( includeSightersCheckBox->isChecked() )
		{

			qDebug() << "Setting series (sighters)" << i << "to" << groupMeasurementType2 << groupSize_sighters;

			if ( qIsNaN(groupSize_sighters) )
			{
				// series doesn't have enough shots to calculate, so disable it altogether. unchecking the box triggers seriesCheckBoxChanged() to run
				series->groupSizeLabel->setText("2+ shots required");
				series->enabled->setChecked(false);
				series->enabled->setEnabled(false);
			}
			else
			{
				series->groupSizeLabel->setText(QString("%1 %2").arg(groupSize_sighters, 0, 'f', 3).arg(groupUnits2));

				if ( qIsNaN(groupSize) )
				{
					// transition from disabled series (no sighters) to enabled series (with sighters)
					series->enabled->setChecked(true);
					series->enabled->setEnabled(true);
				}
			}
		}
		else
		{
			qDebug() << "Setting series" << i << "to" << groupMeasurementType2 << groupSize;

			if ( qIsNaN(groupSize) )
			{
				// series doesn't have enough shots to calculate, so disable it altogether. unchecking the box triggers seriesCheckBoxChanged() to run
				series->groupSizeLabel->setText("2+ shots required");
				series->enabled->setChecked(false);
				series->enabled->setEnabled(false);
			}
			else
			{
				series->groupSizeLabel->setText(QString("%1 %2").arg(groupSize, 0, 'f', 3).arg(groupUnits2));

				if ( qIsNaN(groupSize_sighters) )
				{
					// transition from disabled series (with sighters) to enabled series (no sighters)
					series->enabled->setChecked(true);
					series->enabled->setEnabled(true);
				}
			}
		}
	}
}

void SeatingDepthTest::includeSightersCheckBoxChanged ( bool state )
{
	qDebug() << "includeSightersCheckBoxChanged state =" << state;

	updateDisplayedData();
}

void SeatingDepthTest::xAxisSpacingChanged ( int index )
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


void SeatingDepthTest::optionCheckBoxChanged ( QCheckBox *checkBox, QLabel *label, QComboBox *comboBox )
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
	else if ( index == YSTDEV )
	{
		headerGroupType->setText("Group Size (Y Stdev)");
	}
	else if ( index == XSTDEV )
	{
		headerGroupType->setText("Group Size (X Stdev)");
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

void SeatingDepthTest::importedGroupMeasurementTypeChanged ( int index )
{
	qDebug() << "importedGroupMeasurementTypeChanged index =" << index;

	/*
	 * This signal handler is only connected in imported data entry mode. When the group measurement type is changed,
	 * we need to iterate through and update every Group Size row to display the new measurement type.
	 */

	updateDisplayedData();
}

void SeatingDepthTest::importedGroupUnitsChanged ( int index )
{
	qDebug() << "importedGroupUnitsChanged index =" << index;

	/*
	 * This signal handler is only connected in imported data entry mode. When the group size units is changed,
	 * we need to iterate through and update every Group Size row to display the new unit.
	 */

	updateDisplayedData();
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

			if ( series->groupSize && (series->groupSize->value() == 0) )
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

	/* Check if any cartridge lengths are duplicated. We can rely on series being sorted and not zero. */

	if ( xAxisSpacing->currentIndex() == PROPORTIONAL )
	{
		double lastCartridgeLength = 0;
		for ( int i = 0; i < seriesToGraph.size(); i++ )
		{
			SeatingSeries *series = seriesToGraph.at(i);

			double cartridgeLength = series->cartridgeLength->value();

			if ( cartridgeLength == lastCartridgeLength )
			{
				qDebug() << "Duplicate cartridge length detected" << cartridgeLength << ", prompting user to switch to constant x-axis spacing";

				QMessageBox::StandardButton reply;
				reply = QMessageBox::question(this, "Duplicate cartridge lengths", "Duplicate cartridge lengths detected. Switching graph to constant spacing mode.", QMessageBox::Ok | QMessageBox::Cancel);

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

			lastCartridgeLength = cartridgeLength;
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
	QVector<double> yError;

	for ( int i = 0; i < seriesToGraph.size(); i++ )
	{
		SeatingSeries *series = seriesToGraph.at(i);

		double cartridgeLength = series->cartridgeLength->value();

		double groupSize;
		if ( series->groupSize )
		{
			// If the user selected manual data entry
			groupSize = series->groupSize->value();
		}
		else
		{
			// If the user imported data from a .CSV
			if ( includeSightersCheckBox->isChecked() )
			{
				// If the user imported data from a .CSV
				if ( groupMeasurementType->currentIndex() == ES )
				{
					groupSize = series->extremeSpread_sighters.at(groupUnits->currentIndex());
				}
				else if ( groupMeasurementType->currentIndex() == YSTDEV )
				{
					groupSize = series->yStdev_sighters.at(groupUnits->currentIndex());
				}
				else if ( groupMeasurementType->currentIndex() == XSTDEV )
				{
					groupSize = series->xStdev_sighters.at(groupUnits->currentIndex());
				}
				else if ( groupMeasurementType->currentIndex() == RSD )
				{
					groupSize = series->radialStdev_sighters.at(groupUnits->currentIndex());
				}
				else
				{
					groupSize = series->meanRadius_sighters.at(groupUnits->currentIndex());
				}
			}
			else
			{
				if ( groupMeasurementType->currentIndex() == ES )
				{
					groupSize = series->extremeSpread.at(groupUnits->currentIndex());
				}
				else if ( groupMeasurementType->currentIndex() == YSTDEV )
				{
					groupSize = series->yStdev.at(groupUnits->currentIndex());
				}
				else if ( groupMeasurementType->currentIndex() == XSTDEV )
				{
					groupSize = series->xStdev.at(groupUnits->currentIndex());
				}
				else if ( groupMeasurementType->currentIndex() == RSD )
				{
					groupSize = series->radialStdev.at(groupUnits->currentIndex());
				}
				else
				{
					groupSize = series->meanRadius.at(groupUnits->currentIndex());
				}
			}
		}

		qDebug() << QString("%1 - %2, %3").arg(series->name->text()).arg(cartridgeLength).arg(groupSize);
		qDebug() << "";

		/*
		 * The user can select either default x-axis spacing (x-ticks are spaced proportionally and irregular values will create "holes" in the
		 * graph) or force equal spacing regardless of value. The default case plots the x-values as usual, while the latter case plots a regularly
		 * incrementing index and overrides the xAxis ticker to display custom tick labels.
		 */

		if ( xAxisSpacing->currentIndex() == CONSTANT )
		{
			xPoints.push_back(i);
			textTicker->addTick(i, QString::number(cartridgeLength));
		}
		else
		{
			xPoints.push_back(cartridgeLength);
			textTicker->addTick(cartridgeLength, QString::number(cartridgeLength));
		}
		yPoints.push_back(groupSize);
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
		xTrendPoints.push_back(xPoints.first());
		yTrendPoints.push_back(res[1] + (xPoints.first() * res[0]));
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
	else if ( groupMeasurementType->currentIndex() == YSTDEV )
	{
		groupMeasurementType2 = "Y Standard Deviation";
	}
	else if ( groupMeasurementType->currentIndex() == XSTDEV )
	{
		groupMeasurementType2 = "X Standard Deviation";
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
		// Collect annotation contents
		QStringList aboveAnnotationText;
		QStringList belowAnnotationText;

		if ( groupSizeCheckBox->isChecked() )
		{
			QString annotation = QString::number(yPoints.at(i), 'f', 3);
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
				double delta = yPoints.at(i) - prevSize;
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

		yCoord = yPoints.at(i);

		QCPItemText *belowAnnotation = new QCPItemText(customPlot);
		belowAnnotation->setText(belowAnnotationText.join('\n'));
		belowAnnotation->setFont(QFont("DejaVu Sans", scaleFontSize(9)));
		belowAnnotation->setColor(QColor("#4d4d4d"));
		belowAnnotation->position->setType(QCPItemPosition::ptAbsolute);
		if ( xAxisSpacing->currentIndex() == CONSTANT )
		{
			belowAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(i), customPlot->yAxis->coordToPixel(yCoord) + 10);
		}
		else
		{
			belowAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(xPoints.at(i)), customPlot->yAxis->coordToPixel(yCoord) + 10);
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
			aboveAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(i), customPlot->yAxis->coordToPixel(yCoord) - 10);
		}
		else
		{
			aboveAnnotation->position->setCoords(customPlot->xAxis->coordToPixel(xPoints.at(i)), customPlot->yAxis->coordToPixel(yCoord) - 10);
		}
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
