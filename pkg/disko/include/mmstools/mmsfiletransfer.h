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

#ifndef MMSFILETRANSFER_H_
#define MMSFILETRANSFER_H_
#ifdef __HAVE_CURL__

using namespace std;

#include <string>
#include <sigc++/sigc++.h>

extern "C" {
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
}

/** Specifies a structure for file operations */
typedef struct {
	const char *filename;
	FILE *stream;
} FtpFile;


/**
 *  A ftp operations class.
 *
 *  This is a class that provides the functions to down- and upload files from/to a ftp server.
 *  As far as it is supported by the ftp server, resuming is also possible.
 *
 *  @author Patrick Helterhoff
 */
class MMSFiletransfer {

private:
	CURL *ehandle;
	string remoteUrl;
	string logindata;
	CURLcode lasterror;
	long timeout;
	long lowSpeedLimit;
	unsigned int port;

	/** buffer to cached data from url */
    char        *buffer;

    /** buffer length */
    unsigned    buf_len;

    /** fill pointer within buffer */
    unsigned    buf_pos;


public:
	/** A signal that emits the progress (in percentage) of the current up- or download. */
	sigc::signal<void, const unsigned int> progress;

    /** Virtual function for the curl write callback. */
    virtual size_t mem_write_callback(char *buffer, size_t size, size_t nitems, void *outstream);

	/**
	 * Constructor of class MMSFiletransfer.
	 *
	 * @param url     [in] the remote host and desired directory ("localhost/dir")
	 */
	MMSFiletransfer(const string url, const unsigned int ftpPort);

	/** Destructor of class MMSFiletransfer. */
	virtual ~MMSFiletransfer();

	/**
	 * Performs a ftp upload for the specified local file.
	 *
	 * @param   localfile    [in] the local file to be uploaded
	 * @param   remoteName   [in] name and path of the remote file
	 * @param   resume    	 [in] resume a prior upload
	 */
	bool performUpload(const string localfile, const string remoteName, bool resume = false);

	/**
	 * Performs a ftp download for the specified remote file.
	 *
	 * @param   localfile    [in] the local file to be saved
	 * @param   remoteName   [in] name and path of the remote file
	 * @param   resume    	 [in] resume a prior download
	 */
	bool performDownload(const string localfile, const string remoteName, bool resume = false);

	/**
	 * Deletes the specified remote file.
	 *
	 * @param   remoteFile   [in] name and path of the remote file
	 */
	bool deleteRemoteFile(const string remoteFile);

	/**
	 * 	Retrieves a directory listing and writes it into the memory
	 *
	 * @param	buffer		[out] pointer to a char buffer
	 * @param	directory	[in] the remote directory (path from root)
	 * @param	namesOnly	[in] flag to retrieve only the names
	 */
	bool getListing(char **buffer, string directory, bool namesOnly = false);

	/**
	 * Enables verbose output of from the curl lib.
	 */
	void setVerboseInformation(bool enable);

	/**
	 * Use this to set user and password for the ftp server connection, if necessary.
	 *
	 * @param   user    	[in] the ftp user
	 * @param   password   	[in] the password
	 */
	void setAuthData(const string user, const string password);

	/**
	 * Changes the remote url.
	 * The change will be performed on the following ftp operation (upload / download).
	 *
	 * @param url	[in] the remote host (e.g. "127.0.0.1")
	 */
	void setRemoteUrl(const string url);

	/** Returns the current remote url. */
	const string getRemoteUrl();

	/**
	 * Sets the port for the ftp connection.
	 *
	 * @param ftpPort 		[in] The port for the ftp connection to the remote server.
	 */
	void setFtpPort(const unsigned int ftpPort);

	/** Returns the current ftp port. */
	const unsigned int getFtpPort();

	/**
	 * Sets the timeout.
	 *
	 * @param timeouts 		[in] The timeout in seconds.
	 */
	void setTimeout(const long timemout);

	/** Returns the current timeout in seconds. */
	const long getTimeout();

	/**
	 * Sets the low speed limit to be considered as timeout (default: 100 kb/s).
	 *
	 * @param limit		[in] The low speed limit in byte per second
	 */
	void setLowSpeedLimit(const long limit);

	/** Returns the current speed limit (bytes per second) to be considered as timeout. */
	const long getLowSpeedLimit();

	/**
	 * Returns the error number of the last operation, or 0 if no error has occured.
	 * If the errormsg parameter is supplied it will be filled with a human readable
	 * errormessage.
	 *
	 * @param errormsg 		[out] If supplied it will be filled with an error message.
	 *
	 * @return the errornumber or 0
	 */
	int getLastError(string *errormsg);
};

#endif /*__HAVE_CURL__*/
#endif /*MMSFILETRANSFER_H_*/
