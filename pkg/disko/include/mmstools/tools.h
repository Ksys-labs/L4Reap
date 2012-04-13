/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#ifndef TOOLS_H_
#define TOOLS_H_

#include "mmstools/mmstypes.h"

#include <vector>

extern "C" {
#include <stdarg.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
}

#ifdef __ENABLE_LOG__
#define DEBUGMSG(ident, msg...) writeDebugMessage(ident, __FILE__, __LINE__, msg)
#define DEBUGMSG_OUTSTR(ident, msg...) writeDebugMessage(ident, __FILE__, __LINE__, msg);printf("%s\n", ((string)(msg)).c_str())
#else
#define DEBUGMSG(ident, msg...)
#define DEBUGMSG_OUTSTR(ident, msg...) printf("%s\n", ((string)(msg)).c_str())
#endif

#ifdef __ENABLE_DEBUG__
#define DEBUGOUT(msg...) printf(msg)
#define DEBUGERR(msg...) fprintf(stderr, msg)
#else
#define DEBUGOUT(msg...)
#define DEBUGERR(msg...)
#endif

#if __ENABLE_LOG__ || __ENABLE_DEBUG__
#define TRACEOUT(ident, msg...) writeMessage2Stdout(ident, __FILE__, __LINE__, msg)
#else
#define TRACEOUT(ident, msg...)
#endif

#define MSG2OUT(ident, msg...) writeMessage2Stdout(ident, __FILE__, __LINE__, msg)


#ifdef __ENABLE_DEBUG__
#define WRITE_MSG(ident, msg...) printf("%s: ", ident);printf(msg);printf("\n");
#define WRITE_MSGI(msg...) printf("%s: ", identity.c_str());printf(msg);printf("\n");
#else
#define WRITE_MSG(ident, msg...)
#define WRITE_MSGI(msg...)
#endif

#define WRITE_ERR(ident, msg...) fprintf(stderr, "%s: ", ident);fprintf(stderr, msg);printf("\n");
#define WRITE_ERRI(msg...) fprintf(stderr, "%s: ", identity.c_str());fprintf(stderr, msg);printf("\n");


/**
 * substitutes environment variables in a string
 *
 * @param input string containing the variable components
 * @return the string with the replaced components
 *
 * @note this will only work if the libc supports _XOPEN_SOURCE
 */
string substituteEnvVars(string input);

string maskChars(string str);

string *strToUpr(string *src);

string strToUpr(string src);

string *strToLwr(string *src);

string strToLwr(string src);

int hexToInt(const char *in);

string ucharToHex(unsigned char in);

bool getCurrentTimeBuffer(char *dtbuf, char *datebuf=NULL, char *timebuf=NULL,
                          time_t *clock=NULL);

bool getCurrentTimeString(string *dtstr, string *datestr=NULL, string *timestr=NULL,
                          time_t *clock=NULL);

string getDayOfWeek(time_t *clock=NULL);

void initLogging(char *Iam, char *logfile);

string getSimpleTimeString();

void writeMessage(const char *ctrl,...);

int strToInt(string s);

string iToStr(int i);

string fToStr(double i);

string cpToStr(char *);

string cToStr(char);

char *scanForString(char *buf, char *toFind, char **ret=NULL,
					int offset=0, unsigned int length=0);

char *scanForString(char *buf, char *toFind, string *ret=NULL,
					int offset=0, unsigned int length=0);

string scanForString(string buf, string toFind, string *ret=NULL,
					int offset=0, unsigned int length=0);

void split(string str, string delim, vector<string> &results, bool allowEmpty=false);

void msleep(unsigned long msec);

bool scanString(string toscan, string frontkey, string backkey,
                unsigned int offset, unsigned int length, string *result, unsigned int *nextpos);

void trim(string& str);

bool strToBool(string s);

void executeCmd(string cmd, pid_t *cpid=NULL);

bool file_exist( string filename );

void writeDebugMessage(const char *identity, const char *filename, const int lineno, const char *msg, ...);
void writeDebugMessage(const char *identity, const char *filename, const int lineno, const string &msg);

void writeMessage2Stdout(const char *identity, const char *filename, const int lineno, const char *msg, ...);
void writeMessage2Stdout(const char *identity, const char *filename, const int lineno, const string &msg);

unsigned int getMTimeStamp();
unsigned int getMDiff(unsigned int start_ts, unsigned int end_ts);

int64_t timespecDiff(struct timespec *timeA, struct timespec *timeB);



void rotateUCharBuffer180(unsigned char *buffer, int pitch, int w, int h);
void rotateUShortIntBuffer180(unsigned short int *buffer, int pitch, int w, int h);
void rotateUIntBuffer180(unsigned int *buffer, int pitch, int w, int h);


#ifdef __HAVE_BACKTRACE__
void print_trace(char *prefix);
#endif


//! Convert a bidirectional string.
/*!
\param in_str	source string (UTF-8)
\param out_str	destination string (UTF-8)
\return true if successfully converted
\note in_str and out_str can be the same
*/
bool convBidiString(const string &in_str, string &out_str);


#endif /*TOOLS_H_*/
