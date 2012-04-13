/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

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

#include "mmstools/tools.h"
#include "mmstools/mmsmutex.h"
#include "mmsconfig/mmsconfigdata.h"
#include "mmstools/mmserror.h"
#ifdef __HAVE_WORDEXP__
#include <wordexp.h>
#endif
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#ifdef __HAVE_BACKTRACE__
#include <execinfo.h>
#endif
#ifdef __HAVE_FRIBIDI__
#include <fribidi/fribidi.h>
#endif

#ifdef __L4_RE__
#include "mmstools/vfswrapper.h"
#endif

/* Once-only initialisation of the key */
static pthread_once_t buffer_key_once = PTHREAD_ONCE_INIT;
/* contains the key to the thread specific memory */
static pthread_key_t  key_iam;
static pthread_key_t  key_logfile;
#ifdef __ENABLE_LOG__
static MMSConfigData  config;
#endif
static FILE			  *fp=NULL;
static MMSMutex       debugMsgMutex;


string substituteEnvVars(string input) {
#ifdef __HAVE_WORDEXP__
	wordexp_t p;
    char **w;
    string output = "";
    if (input != "") {
        wordexp(input.c_str(), &p, 0);
        w = p.we_wordv;
        for (unsigned i=0; i<p.we_wordc; i++)
            if (i==0) {
                output = w[i];
                break;
            }
        wordfree(&p);
    }
    return output;
#else
#warning "wordexp not found: substituteEnvVars() has no effect"
    return input;
#endif
}

string maskChars(string str) {
    string  ret;
    for (unsigned i=0; i<str.size(); i++) {
        if (str.at(i) == '\'')
            ret = ret + "''";
        else
            ret = ret + str.at(i);
    }
    return ret;
}

string *strToUpr(string *src) {
    for(string::iterator i=src->begin(); i!= src->end(); i++) {

    	if((*i >= 'a') && (*i <= 'z'))
            (*i)-=32;
    }

    return src;
}

string strToUpr(string src) {
    string s;
    s=src;
    strToUpr(&s);
    return s;
}

string *strToLwr(string *src) {
    for(string::iterator i=src->begin(); i!= src->end(); i++) {

    	if((*i >= 'A') && (*i <= 'Z'))
            (*i)+=32;
    }

    return src;
}

string strToLwr(string src) {
    string s;
    s=src;
    strToLwr(&s);
    return s;
}

int hexToInt(const char *in) {
    int ret=0;

    /* working with first char */
    if (*in>='0' && *in<='9')
        ret+=((int)*in-'0')*16;
    else
    if (*in>='A' && *in<='F')
        ret+=((int)*in-'A'+10)*16;
    else
    if (*in>='a' && *in<='f')
        ret+=((int)*in-'a'+10)*16;

    /* working with second char */
    in++;
    if (*in>='0' && *in<='9')
        ret+=(int)*in-'0';
    else
    if (*in>='A' && *in<='F')
        ret+=(int)*in-'A'+10;
    else
    if (*in>='a' && *in<='f')
        ret+=(int)*in-'a'+10;

    return ret;
}

string ucharToHex(unsigned char in) {
	char buf[3];
	sprintf(buf, "%02x", in);
	return buf;
}

string getSimpleTimeString() {
    string timstr;

    getCurrentTimeString(NULL,NULL,&timstr,NULL);

    return timstr;
}

bool getCurrentTimeBuffer(char *dtbuf, char *datebuf, char *timebuf, time_t *clock) {
    struct  tm newtime;
    time_t  aclock;

    /* get current date and time */
    time(&aclock);
    if (clock) {
        if (*clock)
            aclock=*clock;
        else
            *clock=aclock;
    }
    localtime_r(&aclock, &newtime);

    if (dtbuf)
        sprintf(dtbuf,"%04d-%02d-%02d %02d:%02d:%02d",
                        newtime.tm_year+1900,
                        newtime.tm_mon+1,
                        newtime.tm_mday,
                        newtime.tm_hour,
                        newtime.tm_min,
                        newtime.tm_sec
                        );

    if (datebuf)
        sprintf(datebuf,"%04d-%02d-%02d",
                        newtime.tm_year+1900,
                        newtime.tm_mon+1,
                        newtime.tm_mday
                        );

    if (timebuf)
        sprintf(timebuf,"%02d:%02d:%02d",
                        newtime.tm_hour,
                        newtime.tm_min,
                        newtime.tm_sec
                        );

    return true;
}

bool getCurrentTimeString(string *dtstr, string *datestr, string *timestr, time_t *clock) {
    char    dtbuf[20];
    char    datebuf[11];
    char    timebuf[9];

    if (getCurrentTimeBuffer((dtstr)?dtbuf:NULL,
                             (datestr)?datebuf:NULL,
                             (timestr)?timebuf:NULL,
                             clock)) {
        if (dtstr) *dtstr=dtbuf;
        if (datestr) *datestr=datebuf;
        if (timestr) *timestr=timebuf;
        return true;
    }
    else
        return false;
}

string getDayOfWeek(time_t *clock) {
    struct  tm newtime;
    time_t  aclock;

    /* get current date and time */
    time(&aclock);
    if (clock) {
        if (*clock)
            aclock=*clock;
        else
            *clock=aclock;
    }
    localtime_r(&aclock, &newtime);

    switch (newtime.tm_wday) {
        case 0:
            return "Sunday";
        case 1:
            return "Monday";
        case 2:
            return "Tuesday";
        case 3:
            return "Wednesday";
        case 4:
            return "Thursday";
        case 5:
            return "Friday";
        case 6:
            return "Saturday";
    }

    return "";
}

static void bufferDestroy(void *buffer) {
    free(buffer);
}

static void bufferKeyAlloc() {
    pthread_key_create(&key_iam, bufferDestroy);
    pthread_key_create(&key_logfile, bufferDestroy);
}

void initLogging(char *Iam, char *logfile) {
    char *dest;
    pthread_once(&buffer_key_once, bufferKeyAlloc);

    dest = (char *)pthread_getspecific(key_iam);
    if (dest == NULL)
        pthread_setspecific(key_iam, malloc(100));

    dest = (char *)pthread_getspecific(key_iam);
    memset(dest,0,100);
    strncpy(dest,Iam,99);

    dest = (char *)pthread_getspecific(key_logfile);
    if (dest == NULL)
        pthread_setspecific(key_logfile, malloc(1000));
    dest = (char *)pthread_getspecific(key_logfile);
    memset(dest,0,1000);
    strncpy(dest,logfile,999);

}


void writeMessage(const char *ctrl,...) {
    char *iam;
    char *logname;
    char currTimebuffer[128];
    FILE *File;
    char line[10000];

    va_list marker;
    va_start(marker, ctrl);

    iam     = (char *)pthread_getspecific(key_iam);
    logname = (char *)pthread_getspecific(key_logfile);

    if(logname == NULL) {
        logname = (char*)"/var/log/disko/logfile";
    }
    if(iam == NULL) {
        iam = (char*)"unkown";
    }

    getCurrentTimeBuffer(currTimebuffer);

    if ((File=fopen(logname,"at"))) {

        fprintf (File,"%s  ",currTimebuffer);
        vsprintf(line,ctrl,marker);
        if(line[0]!='[')
            fprintf(File,"[%s]: ",iam);

        fprintf (File,"%s",line);

        fprintf (File,"\n");

        fflush(File);
        fclose(File);
    }

    va_end(marker);

}

int strToInt(string s) {
	return atoi(s.c_str());
}

string iToStr(int i) {
    char mychar[24];
    string mystr;

    sprintf(mychar,"%d",i);
    mystr = mychar;
    return mystr;
}

/* ToDo: get the maximum number of digits for this */
string fToStr(double i) {
    char mychar[1024];
    string mystr;

    sprintf(mychar,"%f",i);
    mystr = mychar;
    return mystr;
}


char *scanForString(char *buf, char *toFind, char **ret,
					int offset, unsigned int length) {
	char 			*ptr;
	char 			*tmp;
	unsigned int	reallen;

	if ((ptr = strstr(buf, toFind))) {
		if (ret) {
			tmp = ptr + strlen(toFind);
			reallen = strlen(tmp);
			if ((int)reallen>=offset) {
				tmp+=offset;
				reallen-=offset;
			}
			else {
				tmp+=reallen;
				reallen=0;
			}
			if (!length)
				length = reallen;
			else
				if (reallen<length) length=reallen;
			*ret = (char *)malloc(length+1);
			memcpy(*ret, tmp, length);
			(*ret)[length]=0;
		}
	}

	return ptr;
}

char *scanForString(char *buf, char *toFind, string *ret,
					int offset, unsigned int length) {
	char	*rbuf;
	char 	*tmpret;

 	if (ret) *ret="";
	if ((rbuf = scanForString(buf, toFind, &tmpret, offset, length))) {
		if (tmpret) {
			if (ret) *ret = tmpret;
			free(tmpret);
		}
	}

	return rbuf;
}

string scanForString(string buf, string toFind, string *ret,
					int offset, unsigned int length) {
	int 			ptr;
	string			tmp;
	unsigned int 	reallen;

	if ((ptr=buf.find(toFind))>=0) {
		if (ret) {
			tmp=buf.substr(ptr + toFind.size());
			reallen = tmp.size();
			if ((int)reallen>=offset) {
				tmp=tmp.substr(offset);
				reallen-=offset;
			}
			else {
				tmp=tmp.substr(reallen);
				reallen=0;
			}
			if (!length)
				length = reallen;
			else
				if (reallen<length) length=reallen;
			*ret = tmp.substr(0, length);
		}

		return buf.substr(ptr);
	}

	return "";
}

void split(string str, string delim, vector<string> &results, bool allowEmpty) {
  size_t cutAt;
  while((cutAt = str.find_first_of(delim)) != str.npos) {
    if(cutAt > 0 || allowEmpty) {
      results.push_back(str.substr(0,cutAt));
    }
    str = str.substr(cutAt+1);
  }
  if(str.length() > 0 || allowEmpty) {
    results.push_back(str);
  }
}

void msleep(unsigned long msec) {
	if (msec > 0)
		usleep(msec * 1000);
}


bool scanString(string toscan, string frontkey, string backkey,
                unsigned int offset, unsigned int length, string *result, unsigned int *nextpos) {
    int pos;

    if (frontkey != "") {
        pos = (int)toscan.find(frontkey);
        if (pos<0)
            return false;
        toscan = toscan.substr(pos + frontkey.size());
        if (nextpos)
            (*nextpos)+=pos + frontkey.size();
    }
    if (backkey != "") {
        pos = (int)toscan.find(backkey);
        if (pos<0)
            return false;
        else {
            toscan = toscan.substr(0, pos);
            if (nextpos)
                (*nextpos)+=pos + backkey.size();
            if (frontkey != "") {
                while (1) {
                    pos = (int)toscan.find(frontkey);
                    if (pos<0)
                        break;
                    toscan = toscan.substr(pos + frontkey.size());
                }
            }
        }
    }

    if (length) {
        if (toscan.size() >= offset + length)
            *result = toscan.substr(offset, length);
        else
            return false;
    }
    else
        if (offset) {
            if (toscan.size() >= offset)
                *result = toscan.substr(offset);
            else
                return false;
        }
        else
            *result = toscan;

    return true;
}


string cpToStr(char *str) {
    string ret = str;
    return ret;
}


string cToStr(char chr) {
    char my[2];
    my[0]=chr;
    my[1]=0;

    string ret = my;

    return ret;
}

void trim(string& str)
{
  string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}

bool strToBool(string s) {
	if(s.empty())
		return false;

	if(strcasecmp(s.c_str(), "true") == 0)
		return true;
	else
		return false;
}

void executeCmd(string cmd, pid_t *cpid) {
#ifdef __L4_RE__
#else
	pid_t pid;
	int i,y;
	int argc;
	char *argv[256];
	char buffer[4096];


    for (i=0;i<256;i++)
    	argv[i]=NULL;
    argc=0;
    sprintf(buffer,"%s",cmd.c_str());
    DEBUGOUT("\n%s\n",buffer);
    argv[0]=buffer;

    i=0;
    while ((buffer[i]!=0)&&(argc<256)) {
        while(buffer[i]==' ')
        	i++;

        if(buffer[i]==0)
        	break;

        if(buffer[i]=='\'') {
            i++;
            y=i;
            while ((buffer[i]!='\'')&&(buffer[i]!=0))
            	i++;

            if (buffer[i]=='\'') {
                buffer[i]=0;
                i++;
            }
        } else if (buffer[i]=='"') {
            i++;
            y=i;
            while ((buffer[i]!='"')&&(buffer[i]!=0))
            	i++;

            if (buffer[i]=='"') {
                buffer[i]=0;
                i++;
            }
        } else {
            y=i;
            while ((buffer[i]!=' ')&&(buffer[i]!=0))
            	i++;

            if (buffer[i]==' ') {
                buffer[i]=0;
                i++;
            }
        }
        argv[argc]=&buffer[y];
        argc++;
    }


	pid = fork();
		if(pid!=-1) {
		if(pid>0) {
		    if (cpid) {
		      (*cpid) = pid;
		    }
			return;
		}
		if(pid==0) {
			unsetenv("LD_PRELOAD");
			execvp(argv[0],argv);
			printf("\nError while exec: %s",strerror(errno));
			printf("\nargv[0]: %s",argv[0]);
			printf("\nargv[1]: %s",argv[1]);
			exit(1);
		}
	}
#endif
}


bool file_exist( string filename ) {
#ifdef __L4_RE__
    return VfsWrapper::instance().exists(filename.c_str());
#else
	struct stat buffer ;

	if ( stat( filename.c_str(), &buffer ) == 0)
		return true;

	return false;
#endif
}

void writeDebugMessage(const char *identity, const char *filename, const int lineno, const char *msg, ...) {
#if defined(__ENABLE_LOG__) && !defined(__L4_RE__)
	va_list 	arglist;
	struct  	timeval tv;
	char    	timebuf[12];
	int			num;
	const char 	*logfile = config.getLogfile().c_str();

    debugMsgMutex.lock();
    if(!strlen(logfile))
    	fp = stderr;
	else if((fp=fopen(logfile, "a+"))==NULL)
		throw MMSError(errno, "Can't open logfile [" + string(strerror(errno)) + "]");

	gettimeofday(&tv, NULL);
    getCurrentTimeBuffer(NULL, NULL, timebuf, NULL);

	num = fprintf(fp, "%s:%02ld %010u %s: ", timebuf, tv.tv_usec/10000, (unsigned int)pthread_self(), identity);
	if(num) {
		va_start(arglist, (char *)msg);
		num = vfprintf(fp, msg, arglist);
		va_end(arglist);
	}
	if(num) num = fprintf(fp, " [%s:%d]\n", filename, lineno);
	if(!num)
		fprintf(stderr, "DISKO: Error writing to logfile\n");

	if(fp != stderr)
		fclose(fp);
    debugMsgMutex.unlock();

	return;
#endif
#if defined(__ENABLE_LOG__) && defined(__L4_RE__)
    va_list     arglist;

    printf("%s: ", identity);

    va_start(arglist, (char *)msg);
    printf(msg, arglist);
    va_end(arglist);

    printf(" [%s:%d]\n", filename, lineno);
#endif
}

void writeDebugMessage(const char *identity, const char *filename, const int lineno, const string &msg) {
#if defined(__ENABLE_LOG__) && !defined(__L4_RE__)
	struct  	timeval tv;
	char    	timebuf[12];
	const char 	*logfile = config.getLogfile().c_str();

    debugMsgMutex.lock();
    if(!strlen(logfile))
    	fp = stderr;
	else if((fp=fopen(logfile, "a+"))==NULL)
		throw MMSError(errno, "Can't open logfile [" + string(strerror(errno)) + "]");

	gettimeofday(&tv, NULL);
    getCurrentTimeBuffer(NULL, NULL, timebuf, NULL);

	if(fprintf(fp, "%s:%02ld %010u %s: %s [%s:%d]\n", timebuf, tv.tv_usec/10000,
	           (unsigned int)pthread_self(), identity, msg.c_str(), filename, lineno) == 0)
		fprintf(stderr, "DISKO: Error writing to logfile\n");

	if(fp != stderr)
		fclose(fp);
    debugMsgMutex.unlock();

	return;
#endif
#if defined(__ENABLE_LOG__) && defined(__L4_RE__)
    printf("%s: %s [%s:%d]\n", identity, msg.c_str(), filename, lineno);
#endif
}


void writeMessage2Stdout(const char *identity, const char *filename, const int lineno, const char *msg, ...) {
	va_list arglist;
	struct  timeval tv;
	char    timebuf[12];
	int		num;

	gettimeofday(&tv, NULL);
    getCurrentTimeBuffer(NULL, NULL, timebuf, NULL);

	num = fprintf(stdout, "%s:%02ld %010u %s: ", timebuf, tv.tv_usec/10000, (unsigned int)pthread_self(), identity);

	if(num) {
		va_start(arglist, (char *)msg);
		num = vfprintf(stdout, msg, arglist);
		va_end(arglist);
	}
	if(num) num = fprintf(stdout, " [%s:%d]\n", filename, lineno);
	if(!num)
		fprintf(stderr, "DISKO: Error writing to stdout\n");

	return;
}


void writeMessage2Stdout(const char *identity, const char *filename, const int lineno, const string &msg) {
	struct  timeval tv;
	char    timebuf[12];

	gettimeofday(&tv, NULL);
    getCurrentTimeBuffer(NULL, NULL, timebuf, NULL);

	if(printf("%s:%02ld %010u %s: %s [%s:%d]\n", timebuf, tv.tv_usec/10000,
	           (unsigned int)pthread_self(), identity, msg.c_str(), filename, lineno) == 0)
		fprintf(stderr, "DISKO: Error writing to logfile\n");

	return;
}


#define MAX_MTIMESTAMP	999999

unsigned int getMTimeStamp() {
	struct  timeval tv;

	// get seconds and milli seconds
	gettimeofday(&tv, NULL);

	// build timestamp
	return ((tv.tv_sec % 1000) * 1000) + tv.tv_usec / 1000;
}

unsigned int getMDiff(unsigned int start_ts, unsigned int end_ts) {
	unsigned int diff;
	if (start_ts <= end_ts)
		diff = end_ts - start_ts;
	else
		diff = MAX_MTIMESTAMP - start_ts + 1 + end_ts;

	return diff;
}

int64_t timespecDiff(struct timespec *timeA, struct timespec *timeB) {
  return ((timeA->tv_sec * 1000000000) + timeA->tv_nsec) -
           ((timeB->tv_sec * 1000000000) + timeB->tv_nsec);
}





void rotateUCharBuffer180(unsigned char *buffer, int pitch, int w, int h) {
	for (int y = 0; y < (h + 1) / 2; y++) {
		unsigned char tmp;
		unsigned char *ptr1 = &buffer[y * pitch];
		unsigned char *ptr2 = &buffer[(h - 1 - y) * pitch];
		bool sameline = (ptr1 == ptr2);
		ptr2+= w - 1;
		for (int x = 0; x < ((!sameline) ? w : w / 2); x++) {
			tmp = *ptr2;
			*ptr2 = *ptr1;
			*ptr1 = tmp;
			ptr1++;
			ptr2--;
		}
	}
}

void rotateUShortIntBuffer180(unsigned short int *buffer, int pitch, int w, int h) {
	for (int y = 0; y < (h + 1) / 2; y++) {
		unsigned short int tmp;
		unsigned short int *ptr1 = (unsigned short int *)&((unsigned char *)buffer)[y * pitch];
		unsigned short int *ptr2 = (unsigned short int *)&((unsigned char *)buffer)[(h - 1 - y) * pitch];
		bool sameline = (ptr1 == ptr2);
		ptr2+= w - 1;
		for (int x = 0; x < ((!sameline) ? w : w / 2); x++) {
			tmp = *ptr2;
			*ptr2 = *ptr1;
			*ptr1 = tmp;
			ptr1++;
			ptr2--;
		}
	}
}

void rotateUIntBuffer180(unsigned int *buffer, int pitch, int w, int h) {
	for (int y = 0; y < (h + 1) / 2; y++) {
		unsigned int tmp;
		unsigned int *ptr1 = (unsigned int *)&((unsigned char *)buffer)[y * pitch];
		unsigned int *ptr2 = (unsigned int *)&((unsigned char *)buffer)[(h - 1 - y) * pitch];
		bool sameline = (ptr1 == ptr2);
		ptr2+= w - 1;
		for (int x = 0; x < ((!sameline) ? w : w / 2); x++) {
			tmp = *ptr2;
			*ptr2 = *ptr1;
			*ptr1 = tmp;
			ptr1++;
			ptr2--;
		}
	}
}

#ifdef __HAVE_BACKTRACE__
void print_trace(char *prefix) {
	void 	*array[10];
	size_t 	size;
	char 	**strings;
	size_t 	i;

	size 	= backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	fprintf(stderr, "******************* %s ****************\n", prefix);

	for(i = 2; i < size; i++)
	  fprintf(stderr, "%s\n", strings[i]);

	free(strings);
}
#endif



bool convBidiString(const string &in_str, string &out_str) {
#ifdef __HAVE_FRIBIDI__
	bool ret = false;
	const char *char_set = "UTF-8";
	FriBidiCharSet char_set_num;

	// get charset and init
	if (!(char_set_num = fribidi_parse_charset((char *)char_set))) {
		printf("DISKO: FriBidi error, unrecognized character set '%s'\n", char_set);
		return false;
	}
	fribidi_set_mirroring(true);
	fribidi_set_reorder_nsm(false);

	// check input length
	FriBidiStrIndex len = in_str.length();
	if (len <= 0) {
		out_str = "";
		return true;
	}

	// allocate temp buffers
	int memsize = sizeof(FriBidiChar) * (len + 1);
	FriBidiChar *logical = (FriBidiChar *)malloc(memsize);
	FriBidiChar *visual  = (FriBidiChar *)malloc(memsize);
	char        *ostr    = (char *)malloc(memsize);

	if (logical && visual && ostr) {
		// convert input string into FriBidiChar buffer
		len = fribidi_charset_to_unicode(char_set_num, (char *)in_str.c_str(), len, logical);

		// create a bidi visual string
		FriBidiCharType base = FRIBIDI_TYPE_ON;
		if ((ret = fribidi_log2vis(logical, len, &base, visual, NULL, NULL, NULL))) {
			// convert it back to output string
			FriBidiStrIndex new_len = fribidi_unicode_to_charset(char_set_num, visual, len, ostr);
			if (new_len <= 0) {
				printf("DISKO: FriBidi error, fribidi_unicode_to_charset() failed\n");
				ret = false;
			}
			else {
				out_str = ostr;
			}
		}
	}

	// free temp buffers
	if (logical) free(logical);
	if (visual)  free(visual);
	if (ostr)    free(ostr);

	return ret;
#else
	// no bidi conversion lib
	return false;
#endif
}

