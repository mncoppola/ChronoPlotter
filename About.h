#include <QWidget>

#include "ChronoPlotter.h"

class About : public QWidget
{
	Q_OBJECT

	public:
		About(QWidget *parent = 0);
		~About() {};

	public slots:
		void showLegal(void);
};
