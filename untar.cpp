/*
 * This file is in the public domain.  Use it as you see fit.
 * Edited by Michael Coppola
 */

/*
 * "untar" is an extremely simple tar extractor:
 *  * A single C source file, so it should be easy to compile
 *    and run on any system with a C compiler.
 *  * Reads basic ustar tar archives.
 *  * Does not require libarchive or any other special library.
 *
 * To compile: cc -o untar untar.c
 *
 * Usage:  untar <archive>
 *
 * In particular, this program should be sufficient to extract the
 * distribution for libarchive, allowing people to bootstrap
 * libarchive on systems that do not already have a tar program.
 *
 * To unpack libarchive-x.y.z.tar.gz:
 *    * gunzip libarchive-x.y.z.tar.gz
 *    * untar libarchive-x.y.z.tar
 *
 * Written by Tim Kientzle, March 2009.
 *
 * Released into the public domain.
 */

/* These are all highly standard and portable headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QDebug>
#include <QFile>
#include <QIODevice>

/* Parse an octal number, ignoring leading and trailing nonsense. */
static int
parseoct(const char *p, size_t n)
{
	int i = 0;

	while ((*p < '0' || *p > '7') && n > 0) {
		++p;
		--n;
	}
	while (*p >= '0' && *p <= '7' && n > 0) {
		i *= 8;
		i += *p - '0';
		++p;
		--n;
	}
	return (i);
}

/* Returns true if this is 512 zero bytes. */
static int
is_end_of_archive(const char *p)
{
	int n;
	for (n = 511; n >= 0; --n)
		if (p[n] != '\0')
			return (0);
	return (1);
}

/* Verify the tar checksum. */
static int
verify_checksum(const char *p)
{
	int n, u = 0;
	for (n = 0; n < 512; ++n) {
		if (n < 148 || n > 155)
			/* Standard tar checksum adds unsigned bytes. */
			u += ((unsigned char *)p)[n];
		else
			u += 0x20;

	}
	return (u == parseoct(p + 148, 8));
}

/* Extract a tar archive. */
int
untar(QFile &rf, QString tempDir)
{
	char buff[512];
	QFile *wf = NULL;
	size_t bytes_read;
	int filesize;

	for (;;) {
		bytes_read = rf.read(buff, 512);
		if (bytes_read < 512) {
			qDebug() << "Short read: expected 512, got" << bytes_read;
			return -1;
		}
		if (is_end_of_archive(buff)) {
			qDebug() << "End of archive";
			break;
		}
		if (!verify_checksum(buff)) {
			qDebug() << "Checksum failure";
			return -1;
		}
		filesize = parseoct(buff + 124, 12);
		switch (buff[156]) {
		case '1':
			qDebug() << " Ignoring hardlink" << buff;
			break;
		case '2':
			qDebug() << " Ignoring symlink" << buff;
			break;
		case '3':
			qDebug() << " Ignoring character device" << buff;
			break;
		case '4':
			qDebug() << " Ignoring block device" << buff;
			break;
		case '5':
			qDebug() << " Ignoring dir" << buff;
			break;
		case '6':
			qDebug() << " Ignoring FIFO" << buff;
			break;
		default:
			qDebug() << " Extracting file" << buff;

			QString path(tempDir);
			path.append("/");
			path.append(buff);

			wf = new QFile(path);
			if ( ! wf->open(QIODevice::WriteOnly) )
			{
				qDebug() << "Failed to create file, skipping...";
			}
			break;
		}
		while (filesize > 0) {
			bytes_read = rf.read(buff, 512);
			if (bytes_read < 512) {
				qDebug() << "Short read: Expected 512, got" << bytes_read;
				return -1;
			}
			if (filesize < 512)
			{
				bytes_read = filesize;
			}
			if ( wf )
			{
				wf->write(buff, bytes_read);
			}
			filesize -= bytes_read;
		}
		if ( wf )
		{
			wf->close();
			wf = NULL;
		}
	}

	return 0;
}
