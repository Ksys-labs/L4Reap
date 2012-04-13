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

#ifndef MMSFILE_H_
#define MMSFILE_H_

#ifdef __HAVE_CURL__
#include <curl/curl.h>
#endif
#include <sys/stat.h>
#include "mmstools/mmserror.h"

#ifdef __L4_RE__
struct whefs_file;
#endif

MMS_CREATEERROR(MMSFileError);


//! Specifies supported types of files.
typedef enum {
	//! not set
    MMSFT_NOTSET    = 0,
    //! normal file
    MMSFT_FILE      = 1,
    //! url
    MMSFT_URL       = 2
} MMSFileType;

//! Specifies supported modes for working with files.
typedef enum {
	//! read binary (file must exist)
    MMSFM_READ      = 0,
    //! write binary (file size will be set to 0)
    MMSFM_WRITE     = 1,
    //! append binary (always writing to the end of the file)
    MMSFM_APPEND    = 2,
    //! read & write binary (file must exist)
    MMSFM_READWRITE = 3,
    //! write & read binary (file size will be set to 0)
    MMSFM_WRITEREAD = 4,
    //! append & read binary (always writing to the end of the file)
    MMSFM_APPENDREAD= 5
} MMSFileMode;

//! Specifies the origins for function setFilePos().
typedef enum {
	//! from beginning of file
    MMSFPO_SET      = 0,
    //! from current position of file pointer
    MMSFPO_CUR      = 1,
    //! from end of file
    MMSFPO_END      = 2
} MMSFilePosOrigin;

//! A file class.
/*!
With this class you can access normal files (/mms/data/file) and
urls (http://address/file) with one interface.
\author Jens Schneider
*/
class MMSFile {
    private:
    	//! ALL files: name of the file
        string      name;

        //! ALL files: open mode
        MMSFileMode mode;

        //! ALL files: use a separate cache for all file types?
        bool        usecache;

        //! ALL files: type of file
        MMSFileType type;

        //! ALL files: last error (0 if last call was sucessfull)
        int         lasterror;

        //! MMSFT_FILE: pointer to a file
#ifdef __L4_RE__
        whefs_file  *file;
#else
        FILE        *file;
#endif

#ifdef __HAVE_CURL__
        //! MMSFT_URL: pointer to a multi handle, if an url is used
        CURLM       *mhandle;

        //! MMSFT_URL: pointer to a curl, if an url is used
        CURL        *curl;
#else
        //! MMSFT_URL: pointer to a multi handle, if an url is used
        void       *mhandle;

        //! MMSFT_URL: pointer to a curl, if an url is used
        void        *curl;
#endif

        //! MMSFT_URL: buffer to cached data from url
        char        *buffer;

        //! MMSFT_URL: buffer length
        unsigned    buf_len;

        //! MMSFT_URL: fill pointer within buffer
        unsigned    buf_pos;

        //! MMSFT_URLL: url fetch is not finished and working in background
        int         still_progr;

        //! USECACHE=TRUE: pointer to the whole file data
        char        *cache;

        //! USECACHE=TRUE: file size
        size_t      cache_fsize;

        //! USECACHE=TRUE: current position in cache
        size_t      cache_fpos;

        //! Internal function which resets the most private variables.
        void resetAll();

        //! Internal function which work with the own curl buffer.
        bool fillCurlBuffer(size_t want, unsigned waittime=20);

        //! Internal function which work with the own curl buffer.
        void freeCurlBuffer(size_t want);

        //! Internal function which opens a file.
        bool openFile();

        //! Internal function which closes a file.
        bool closeFile();
	
        long int file_size;

    public:
#ifdef __HAVE_CURL__

        //! Virtual function for the curl write callback.
        virtual size_t write_cb(char *buffer, size_t size, size_t nitems, void *outstream);
#endif
        //! Constructor of class MMSFile.
        /*!
        \param name     name of the file ("/mms/data/file" or "http://address/file")
        \param mode     optional, open mode (default is MMSFM_READ) see definition of MMSFileMode
        \param usecache optional, enable caching for this file (default is true)
        */
        MMSFile(string name, MMSFileMode mode=MMSFM_READ, bool usecache=true);

        //! Destructor of class MMSFile.
        virtual ~MMSFile();

        /**
         * Returns the name of the file which was set with the constructor MMSFile().
         *
         * @param   effectiveUrl    [in] if set and filetype is MMSFT_URL the effective URL will be returned.
         *
         */
        string getName(const bool effectiveUrl = false);

        //! Returns the open mode of the file which was set with the constructor MMSFile().
        MMSFileMode getMode();

        //! Returns the type of the file. If set to MMSFT_NOTSET, the file was not opened.
        MMSFileType getType();

        //! Returns the error of the last method call or 0, if no error has occured.
        /*!
        \return EBADF, EINVAL, ENOENT, ENOMEM, EOF
        */
        int getLastError();

        //! Returns the end of file status.
        /*!
        It returns EOF (value -1) if the end of the file ist reached. If no error has
        occured but the end is not reached, the method returns 0. If the return value
        is 1, a error has occured, use getLastError() to determine the error.
        \return EOF(-1), 0, 1
        */
        int  endOfFile();

        //! Moves the file pointer to the beginning of the file.
        bool rewindFile();

        //! Moves the file pointer to a specified position.
        /*!
        \param offset   number of bytes from the origin
        \param origin   optional, initial position, see definiton of MMSFilePosOrigin
        \return true if successful
        */
        bool setFilePos(long offset, MMSFilePosOrigin origin=MMSFPO_SET);

        //! Gets the current postion of the file pointer.
        /*!
        \param pos  address of a long for storing the current position
        \return true if successful
        */
        bool getFilePos(long *pos);

        //! Reads data from the file.
        /*!
        \param ptr      storage location for data
        \param ritems   address of a size_t for returning the number of full items actually read
        \param size     item size in bytes
        \param nitems   maximum number of items to be read
        \return true if successful
        \note The pointer ptr must addresses size*nitems bytes in memory.
        */
        bool readBuffer(void *ptr, size_t *ritems, size_t size, size_t nitems);

        //! Reads data from the file.
        /*!
        \param ptr      address of a pointer for returning the allocated memory
        \param ritems   address of a size_t for returning the number of full items actually read
        \param size     optional, item size in bytes
        \param nitems   optional, maximum number of items to be read
        \return true if successful
        \note This method allocates memory for the data and returns the pointer to it.
        \note If you do not use the optional parameters, the method reads the whole file.
        */
        bool readBufferEx(void **ptr, size_t *ritems, size_t size=1, size_t nitems=0xffffffff);

        //! Gets a string from the file.
        /*!
        This method reads up to the next \n or up to size bytes in the current line.
        If a \n is found, it is the last byte in the returned storage.
        \param ptr  storage location for data
        \param size maximum number of bytes to be read
        \return true if successful
        \note The pointer ptr must addresses size bytes in memory.
        */
        bool getString(char *ptr, size_t size);

        //! Gets a string from the file.
        /*!
        This method reads up to the next \n or up to size bytes in the current line.
        If a \n is found, it is the last byte in the returned storage.
        \param ptr  address of a pointer for returning the allocated memory
        \param size optional, maximum number of bytes to be read
        \return true if successful
        \note This method allocates memory for the data and returns the pointer to it.
        \note If you do not use the parameter size, the method reads the whole line up to the next \n.
        */
        bool getStringEx(char **ptr, size_t size=0xffffffff);

        //! Gets a line from the file.
        /*!
        This method reads up to the next \n in the current line. The \n will be
        stripped from the returned storage.
        \param ptr  address of a pointer for returning the allocated memory
        \return true if successful
        \note This method allocates memory for the data and returns the pointer to it.
        */
        bool getLine(char **ptr);

        //! Gets a line from the file.
        /*!
        This method reads up to the next \n in the current line. The \n will be
        stripped from the returned storage.
        \param ptr  address of a pointer for returning the allocated memory
        \return true if successful
        \note This method allocates memory for the data and returns the pointer to it.
        */
        bool getLine(string &line);

        //! Read a character from the file.
        /*!
        \param ptr  address of a character for returning the byte which is to read
        \return true if successful
        */
        bool getChar(char *ptr);

        //! Writes data to the file.
        /*!
        \param ptr      storage location for data
        \param ritems   address of a size_t for returning the number of full items actually written
        \param size     item size in bytes
        \param nitems   number of items to be written
        \return true if successful
        \note The pointer ptr must addresses size*nitems bytes in memory.
        */
        bool writeBuffer(void *ptr, size_t *ritems, size_t size, size_t nitems);

        long int getSize() { return file_size; }

        /*noch implementieren!!!!!!!!!!!
        flush();*/
};

#endif /*MMSFILE_H_*/
