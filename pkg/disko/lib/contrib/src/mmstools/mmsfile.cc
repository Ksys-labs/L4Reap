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

#include <cstdio>
#include <stdlib.h>
#include <errno.h>
#include <cstring>

#ifdef __HAVE_CURL__
#include <curl/curl.h>
#endif

#include "mmstools/mmsfile.h"
#include "mmstools/tools.h"

#ifdef __L4_RE__
#include "mmstools/vfswrapper.h"
#endif

#ifdef __HAVE_CURL__
/* curl calls this c-routine to transfer data to the object */
size_t c_write_cb(char *buffer, size_t size, size_t nitems, void *outstream) {
    if (outstream)
        return ((MMSFile *)outstream)->write_cb(buffer, size, nitems, outstream);
    else
        return 0;
}


size_t MMSFile::write_cb(char *buffer, size_t size, size_t nitems, void *outstream) {
    char    *newbuff;                           /* pointer to new buffer */
    unsigned int     freebuff;                  /* free memory in old buffer */

    /* get the byte number */
    size *= nitems;

    /* calculate free buffer space */
    freebuff=this->buf_len - this->buf_pos;

    if(size > freebuff) {
        /* not enough space in the old buffer */
        newbuff=(char*)realloc(this->buffer,this->buf_len + (size - freebuff));
        if(newbuff==NULL) {
            /*TODO: ERROR HANDLING */
        	DEBUGERR("callback buffer grow failed\n");
            size=freebuff;
        }
        else {
            /* new buffer size */
            this->buf_len+=size - freebuff;
            this->buffer=newbuff;
        }
    }

    memcpy(&(this->buffer[this->buf_pos]), buffer, size);
    this->buf_pos += size;

    return size;
}
#endif /* __HAVE_CURL__ */


void MMSFile::resetAll() {
    this->type=MMSFT_NOTSET;
    this->mhandle=NULL;
    this->curl=NULL;
    this->file=NULL;
    this->buffer=NULL;
    this->buf_len=0;
    this->buf_pos=0;
    this->still_progr=0;
    this->cache=NULL;
    this->cache_fsize=0;
    this->cache_fpos=0;
}


bool MMSFile::fillCurlBuffer(size_t want, unsigned waittime) {
#ifdef __HAVE_CURL__
    fd_set          fdread;
    fd_set          fdwrite;
    fd_set          fdexcep;
    int             maxfd;
    struct timeval  timeout;
    int             rc;
    CURLMcode       cres;

    if((!this->still_progr) || (this->buf_pos > want))
        return true;

    /* attempt to fill buffer */
    do {
        /* maximum loops reached */
        if (!waittime) return false;
        waittime--;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        /* set a timeout */
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;

        /* get file descriptors from the transfers */
        cres=curl_multi_fdset(this->mhandle, &fdread, &fdwrite, &fdexcep, &maxfd);

        if(cres!=CURLM_OK) {
            /*TODO: ERROR HANDLING */
        	DEBUGERR("curl_multi_fdset failed %d\n",cres);
            return false;
        }

        /* do a select */
        rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

        if (rc>0) {
            while(curl_multi_perform(this->mhandle, &this->still_progr) == CURLM_CALL_MULTI_PERFORM)
                usleep(10);
        }
        else
        if (rc<0) {
            return false;
        }

    } while(this->still_progr && (this->buf_pos < want));

    return true;
#else
    return false;
#endif
}


void MMSFile::freeCurlBuffer(size_t want) {
#ifdef __HAVE_CURL__

    if ((this->buf_pos - want) <=0) {
        if (this->buffer) free(this->buffer);
        this->buffer  = NULL;
        this->buf_pos = 0;
        this->buf_len = 0;
    }
    else {
        memmove(this->buffer,
                &(this->buffer[want]),
                (this->buf_pos - want));

        this->buf_pos -= want;
    }
#endif /*__HAVE_CURL__*/
}

bool MMSFile::openFile() {
    string tmp;

    /* check if already opened */
    if ((this->file)||(this->curl)) {
        this->lasterror = EBADF;
        return false;
    }

    /* reset the type */
    this->type=MMSFT_NOTSET;

    /* check name if it is an url string */
    tmp=this->name.substr(0, 7);
    strToUpr(&tmp);
    if (tmp=="HTTP://")
    	this->type=MMSFT_URL;
    else
    	this->type=MMSFT_FILE;

    if (this->type!=MMSFT_URL) {
        /* try to open normal file */
        char tmpmode[4];
        switch (this->mode) {
            case MMSFM_READ:
                strcpy(tmpmode, "rb");
                break;
            case MMSFM_WRITE:
                strcpy(tmpmode, "wb");
                this->usecache=false;
                break;
            case MMSFM_APPEND:
                strcpy(tmpmode, "ab");
                this->usecache=false;
                break;
            case MMSFM_READWRITE:
                strcpy(tmpmode, "r+b");
                this->usecache=false;
                break;
            case MMSFM_WRITEREAD:
                strcpy(tmpmode, "w+b");
                this->usecache=false;
                break;
            case MMSFM_APPENDREAD:
                strcpy(tmpmode, "a+b");
                this->usecache=false;
                break;
            default:
                this->lasterror = EINVAL;
                return false;
        }

#ifdef __L4_RE__
        this->file=VfsWrapper::instance().fopen(name.c_str(), tmpmode);
#else
        this->file=fopen(name.c_str(), tmpmode)
#endif
        if (!this->file) {
        	this->lasterror=ENOENT;
        	return false;
        }
        
#ifdef __L4_RE__
        file_size = VfsWrapper::instance().fseek(this->file, 0, SEEK_END);
	    VfsWrapper::instance().fseek(this->file, 0, 0);
        //printf("MMSFile::openFile %x size=%d\n",this->file, file_size);
#endif
    }

    if (this->type!=MMSFT_FILE)
    {
#ifdef __HAVE_CURL__

        /* try to open a url */
        if (this->mode!=MMSFM_READ) {
            /* I can't use this mode here */
            this->lasterror=EINVAL;
            return false;
        }

        this->curl = curl_easy_init();

        curl_easy_setopt(this->curl, CURLOPT_URL, this->name.c_str());
        curl_easy_setopt(this->curl, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(this->curl, CURLOPT_VERBOSE, false);
        curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, c_write_cb);

        this->mhandle = curl_multi_init();

        curl_multi_add_handle(mhandle, this->curl);

        /* start the fetch */
        while (curl_multi_perform(this->mhandle, &this->still_progr) == CURLM_CALL_MULTI_PERFORM)
            usleep(10);

        if((this->buf_pos == 0) && (!this->still_progr)) {
            /* an error occurred */
            curl_multi_remove_handle(this->mhandle, this->curl);
            curl_multi_cleanup(this->mhandle);
            curl_easy_cleanup(this->curl);
            resetAll();
            this->lasterror=ENOENT;
            return false;
        }

        /* it is a url */
        this->type = MMSFT_URL;
#else
		throw MMSFileError(-1, "compile curl support!");
#endif
    }

    if (this->usecache) {
        /* use separate cache for better performance */
        /* the next function has to read real */
        this->usecache=false;

        /* read the whole file */
        if (!readBufferEx((void**)&(this->cache), &(this->cache_fsize))) {
            /* failed to read file, close it */
            int err=this->lasterror;
            closeFile();
            this->lasterror=err;
            this->usecache=true; /* back to true */
            return false;
        }

        /* back to true */
        this->usecache=true;

        /* all right, set pos to the begin of cache */
        this->cache_fpos=0;
    }

    /* clear error */
    this->lasterror = 0;
    return true;
}


bool MMSFile::closeFile() {
    bool    retcode=true;

    /* clear error */
    this->lasterror = 0;

    switch(this->type) {
        case MMSFT_FILE:
            if (this->file) {
#ifdef __L4_RE__
                if (VfsWrapper::instance().fclose(this->file)!=0)
#else
                if (fclose(this->file)!=0)
#endif
                    this->lasterror = EOF;
            }
            else {
                this->lasterror = EBADF;
                retcode=false;
            }
            break;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            /* free curl */
            if (this->curl) {
            	if(this->mhandle) {
	                curl_multi_remove_handle(this->mhandle, this->curl);
	                curl_multi_cleanup(this->mhandle);
            	}
                curl_easy_cleanup(this->curl);
            }
            else {
                this->lasterror = EBADF;
                retcode=false;
            }
            break;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif
        default:
            /* unknown type */
            this->lasterror = EBADF;
            retcode=false;
            break;
    }

    /* free allocated buffer */
    if (this->buffer) free(this->buffer);
    if (this->cache) free(this->cache);

    /* reset values */
    resetAll();

    return retcode;
}


MMSFile::MMSFile(string _name, MMSFileMode _mode, bool _usecache) :
    name(_name),
    mode(_mode),
    usecache(_usecache),
    lasterror(0) {

    /* reset values */
    resetAll();

    /* open the file */
    openFile();
}


MMSFile::~MMSFile() {
    /* free all */
    closeFile();
}


string MMSFile::getName(const bool effectiveUrl) {
#ifdef __L4_RE__
    if (this->type != MMSFT_URL)
    {
        return this->name;
    }
#endif
#ifdef __HAVE_CURL__
    if(effectiveUrl && this->type == MMSFT_URL) {
        char *buf = NULL;
        if(curl_easy_getinfo(this->curl, CURLINFO_EFFECTIVE_URL, buf) == CURLE_OK)
            return string(buf);
    }

    return this->name;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif
}

MMSFileMode MMSFile::getMode() {
    return this->mode;
}


MMSFileType MMSFile::getType() {
    return this->type;
}


int MMSFile::getLastError() {
    return lasterror;
}


int MMSFile::endOfFile() {

    /* clear error */
    this->lasterror = 0;

    /* cache given? */
    if (this->usecache) {
        /* work with separate cache */
        if (this->cache) {
            if (this->cache_fpos < this->cache_fsize)
                /* not the end of the file */
                return 0;

            /* end of file */
            this->lasterror = EOF;
            return EOF;
        }
        /* no cache available */
        this->lasterror = EBADF;
        return 1;
    }

    /* normal access without separate cache */
    switch(this->type) {
        case MMSFT_FILE:
            if (this->file) {
#ifdef __L4_RE__
                if (VfsWrapper::instance().f_eof(this->file)==0)
#else
                if (feof(this->file)==0)
#endif
                    /* not the end of the file */
                    return 0;

                /* end of file */
                this->lasterror = EOF;
                return EOF;
            }
            this->lasterror = EBADF;
            return 1;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            if (this->curl) {
                if((this->buf_pos == 0) && (!this->still_progr)) {
                    /* end of file */
                    this->lasterror = EOF;
                    return EOF;
                }
                /* not the end of the file */
                return 0;
            }
            this->lasterror = EBADF;
            return 1;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif

        default:
            /* unknown type */
            this->lasterror = EBADF;
            return 1;
    }
}


bool MMSFile::rewindFile() {

    /* clear error */
    this->lasterror = 0;

    /* cache given? */
    if (this->usecache) {
        /* work with separate cache */
        if (this->cache) {
            this->cache_fpos = 0;
            return true;
        }
        /* no cache available */
        this->lasterror = EBADF;
        return false;
    }

    /* normal access without separate cache */
    switch(this->type) {
        case MMSFT_FILE:
            if (this->file) {
#ifdef __L4_RE__
                VfsWrapper::instance().rewind(this->file);
#else
                std::rewind(this->file);
#endif
                return true;
            }
            this->lasterror = EBADF;
            return false;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            if (this->curl) {
                /* close url */
                closeFile();

                /* re-open url */
                return openFile();
            }
            this->lasterror = EBADF;
            return false;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif

        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }
}


bool MMSFile::setFilePos(long offset, MMSFilePosOrigin origin) {

    /* clear error */
    this->lasterror = 0;

    /* cache given? */
    if (this->usecache) {
        /* work with separate cache */
        if (this->cache) {
            long newfpos;
            switch (origin) {
                case MMSFPO_CUR:
                    newfpos=(long)this->cache_fpos + offset;
                    break;
                case MMSFPO_END:
                    newfpos=(long)this->cache_fsize + offset;
                    break;
                case MMSFPO_SET:
                    newfpos=offset;
                    break;
                default:
                    this->lasterror = EINVAL;
                    return false;
            }
            if (newfpos<0) {
                this->lasterror = EINVAL;
                return false;
            }
            if (newfpos>(long)this->cache_fsize) {
                this->lasterror = EINVAL;
                return false;
            }
            this->cache_fpos = (size_t)newfpos;
            return true;
        }
        /* no cache available */
        this->lasterror = EBADF;
        return false;
    }

    /* normal access without separate cache */
    switch(this->type) {
        case MMSFT_FILE:
            if (this->file) {
                int tmporigin;
                switch (origin) {
                    case MMSFPO_CUR:
                        tmporigin=SEEK_CUR;
                        break;
                    case MMSFPO_END:
                        tmporigin=SEEK_END;
                        break;
                    case MMSFPO_SET:
                        tmporigin=SEEK_SET;
                        break;
                    default:
                        this->lasterror = EINVAL;
                        return false;
                }
#ifdef __L4_RE__
                if (VfsWrapper::instance().fseek(this->file, offset, tmporigin)==0)
#else
                if (fseek(this->file, offset, tmporigin)==0)
#endif
                    return true;
            }
            this->lasterror = EBADF;
            return false;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            if (this->curl) {
                /* currently I cannot set the pointer in an url stream */
                /* so I will always fail */
                /* if you use the separate cache (usecache=true), you can */
                /* use this function as for normal files */
            }
            this->lasterror = EBADF;
            return false;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif
        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }
}


bool MMSFile::getFilePos(long *pos) {
    long    mypos;

    /* clear error */
    this->lasterror = 0;

    /* cache given? */
    if (this->usecache) {
        /* work with separate cache */
        if (this->cache) {
            *pos=(long)this->cache_fpos;
            return true;
        }
        /* no cache available */
        this->lasterror = EBADF;
        return false;
    }

    /* normal access without separate cache */
    switch(this->type) {
        case MMSFT_FILE:
            if (this->file) {
#ifdef __L4_RE__
                if ((mypos=VfsWrapper::instance().ftell(this->file))>=0) {
#else
                if ((mypos=ftell(this->file))>=0) {
#endif
                    *pos=mypos;
                    return true;
                }
                this->lasterror = errno;
                return false;
            }
            this->lasterror = EBADF;
            return false;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            if (this->curl) {
                /* currently I cannot get the pointer from an url stream */
                /* so I will always fail */
                /* if you use the separate cache (usecache=true), you can */
                /* use this function as for normal files */
            }
            this->lasterror = EBADF;
            return false;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif

        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }
}


bool MMSFile::readBuffer(void *ptr, size_t *ritems, size_t size, size_t nitems) {
    size_t  myri;

    /* clear error */
    this->lasterror = 0;

    /* clear ritems */
    if (!ritems) ritems=&myri;
    *ritems=0;

    /* check input */
    if ((!size) || (!nitems)) {
        this->lasterror = EINVAL;
        return false;
    }

    /* check if this->mode allowes to read from the file */
    switch(this->type) {
        case MMSFT_FILE:
            if ((this->mode==MMSFM_WRITE)||(this->mode==MMSFM_APPEND)) {
                this->lasterror = EBADF;
                return false;
            }
            break;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            if (this->mode!=MMSFM_READ) {
                this->lasterror = EBADF;
                return false;
            }
            break;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif
        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }

    /* cache given? */
    if (this->usecache) {
        /* work with separate cache */
        if (this->cache) {
            /* calc available data */
            size_t availdata=this->cache_fsize-this->cache_fpos;
            if (availdata <= 0) {
                availdata=0;
                this->lasterror=EOF;
            }

            /* calc bytes to copy */
            *ritems = nitems * size;
            if (availdata < *ritems) *ritems=availdata;

            /* copy from cache to callers buffer */
            memcpy(ptr, &(this->cache[this->cache_fpos]), *ritems);

            /* increase fpos */
            this->cache_fpos+=*ritems;

            /* recalc number of items which are read */
            *ritems=*ritems/size;
            return true;
        }
        /* no cache available */
        this->lasterror = EBADF;
        return false;
    }

    /* normal access without separate cache */
    switch(this->type) {
        case MMSFT_FILE:
            if (this->file) {
#ifdef __L4_RE__
                *ritems = VfsWrapper::instance().fread(ptr, size, nitems, this->file);
#else
                *ritems = fread(ptr, size, nitems, this->file);
#endif
                if (*ritems < nitems) {
                    /* error, check if eof */
                    if (endOfFile()==EOF) return true;

                    /* a read error */
                    this->lasterror = EBADF;
                    return false;
                }
                return true;
            }
            this->lasterror = EBADF;
            return false;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
        	if (this->curl) {
                /* calc bytes to receive */
                *ritems = nitems * size;

                /* receive data, if not available */
                if (!fillCurlBuffer(*ritems)) {
                    this->lasterror = EBADF;
                    return false;
                }
                if (!this->buf_pos) {
                    this->lasterror = EBADF;
                    return false;
                }
                if (this->buf_pos < *ritems)
                    *ritems = this->buf_pos;

                /* copy from cached buffer to callers buffer */
                memcpy(ptr, this->buffer, *ritems);

                /* freeing unneeded memory in my buffer */
                freeCurlBuffer(*ritems);

                /* recalc number of items which are read */
                *ritems=*ritems/size;
                return true;
            }
            this->lasterror = EBADF;
            return false;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif

        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }
}


bool MMSFile::readBufferEx(void **ptr, size_t *ritems, size_t size, size_t nitems) {
    size_t  myri, myri2;
    size_t  myni;
    void    *newptr;
    bool    ret;

    /* clear error */
    this->lasterror = 0;

    /* init return ptr */
    *ptr=NULL;

    /* clear ritems */
    if (!ritems) ritems=&myri2;
    *ritems=0;

    /* check input */
    if ((!size) || (!nitems)) {
        this->lasterror = EINVAL;
        return false;
    }

    /* init other */
    if ((myni = 0x1000 / size) < 1) myni=1;
    myri=0;

    /* read file */
    do {
        if (endOfFile()==EOF) break;
        if ((myri > 0) && (myni > myri)) break;
        if (nitems == 0) break;
        if (nitems < myni) myni=nitems;
        nitems -= myni;
        *ritems = *ritems + myri;

        newptr=realloc(*ptr,*ritems*size+myni*size);
        if (!newptr) {
            free(*ptr);
            *ptr=NULL;
            this->lasterror = ENOMEM;
            return false;
        }
        *ptr=newptr;

    } while ((ret=readBuffer(&(((char*)*ptr)[*ritems*size]), &myri, size, myni)));

    /* check for error */
    if (ret) {
        if ((nitems == 0) || (endOfFile()==EOF)) {
            *ritems = *ritems + myri;
            return true;
        }
        else {
            free(*ptr);
            *ptr=NULL;
            this->lasterror = EBADF;
            return false;
        }
    }
    free(*ptr);
    *ptr=NULL;
    return false;
}


bool MMSFile::getString(char *ptr, size_t size) {
    size_t toget;

    /* clear error */
    this->lasterror = 0;

    /* check input */
    if (!size) {
        this->lasterror = EINVAL;
        return false;
    }

    /* check if this->mode allowes to read from the file */
    switch(this->type) {
        case MMSFT_FILE:
            if ((this->mode==MMSFM_WRITE)||(this->mode==MMSFM_APPEND)) {
                this->lasterror = EBADF;
                return false;
            }
            break;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            if (this->mode!=MMSFM_READ) {
                this->lasterror = EBADF;
                return false;
            }
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif
            break;

        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }

    /* cache given? */
    if (this->usecache) {
        /* work with separate cache */
        if (this->cache) {
            /* calc available data */
            size_t availdata=this->cache_fsize-this->cache_fpos;
            if (availdata <= 0) {
                availdata=0;
                this->lasterror=EOF;
            }

            /* one byte for zero termination */
            toget=size - 1;
            *ptr=0;

            /* calc bytes to copy */
            if (availdata < toget) toget=availdata;

            /* through the cache */
            for(unsigned int loop=this->cache_fpos; loop < this->cache_fpos + toget; loop++) {
                if (this->cache[loop] == '\n') {
                    toget=loop+1-this->cache_fpos;/* include newline */
                    break;
                }
            }

            /* copy from cache to callers buffer */
            memcpy(ptr, &(this->cache[this->cache_fpos]), toget);
            ptr[toget]=0;

            /* increase fpos */
            this->cache_fpos+=toget;

            return true;
        }
        /* no cache available */
        this->lasterror = EBADF;
        return false;
    }

    /* normal access without separate cache */
    switch(this->type) {
        case MMSFT_FILE:
            if (this->file) {
                *ptr=0;
#ifdef __L4_RE__
                ptr = VfsWrapper::instance().fgets(ptr, size, this->file);
#else
                ptr = fgets(ptr, size, this->file);
#endif
                if (!ptr) {
                    /* error, check if eof */
                    if (endOfFile()==EOF) return true;

                    /* a read error */
                    this->lasterror = EBADF;
                    return false;
                }
                return true;
            }
            this->lasterror = EBADF;
            return false;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            if (this->curl) {
                /* one byte for zero termination */
                toget=size - 1;
                *ptr=0;

                /* receive data, if not available */
                if (!fillCurlBuffer(toget)) {
                    this->lasterror = EBADF;
                    return false;
                }
                if (!this->buf_pos) {
                    this->lasterror = EBADF;
                    return false;
                }
                if (this->buf_pos < toget)
                    toget = this->buf_pos;

                /* through the buffer */
                for(unsigned int loop=0; loop < toget; loop++) {
                    if (this->buffer[loop] == '\n') {
                        toget=loop+1;/* include newline */
                        break;
                    }
                }

                /* copy from cached buffer to callers buffer */
                memcpy(ptr, this->buffer, toget);
                ptr[toget]=0;

                /* freeing unneeded memory in my buffer */
                freeCurlBuffer(toget);

                return true;
            }
            this->lasterror = EBADF;
            return false;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif

        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }
}


bool MMSFile::getStringEx(char **ptr, size_t size) {
    size_t  slen, mys;
    void    *newptr;
    bool    ret=false;

    /* clear error */
    this->lasterror = 0;

    /* init return ptr */
    *ptr=NULL;

    /* check input */
    if (!size) {
        this->lasterror = EINVAL;
        return false;
    }

    /* init other */
    mys=0x1000;

    /* read file */
    do {
        if (endOfFile()==EOF) break;

        slen=0;
        if (*ptr) {
            if (!**ptr) break;
            slen=strlen(*ptr);
            if ((*ptr)[slen-1] == '\n') {
                size=0;
                break;
            }
        }
        if (size == 0) break;

        if (size<mys) mys=size;
        size-=mys;

        newptr=realloc(*ptr, (!slen)?(slen+mys):(slen+mys+1));
        if (!newptr) {
            free(*ptr);
            *ptr=NULL;
            this->lasterror = ENOMEM;
            return false;
        }
        *ptr=(char*)newptr;

    } while ((ret=getString(&((*ptr)[slen]), (!slen)?mys:(mys+1))));

    /* check for error */
    if (ret) {
        if ((size == 0) || (endOfFile()==EOF)) {
            return true;
        }
        else {
            free(*ptr);
            *ptr=NULL;
            this->lasterror = EBADF;
            return false;
        }
    }
    free(*ptr);
    *ptr=NULL;
    return false;
}


bool MMSFile::getLine(char **ptr) {
    int slen;

    if (getStringEx(ptr))
        if (*ptr)
            if (**ptr) {
                slen=strlen(*ptr);
                if ((*ptr)[slen-1]=='\n')
                    (*ptr)[slen-1]=0;
                return true;
            }

    return false;
}

bool MMSFile::getLine(string &line) {
    int slen;
    char *ptr = NULL;
    if (getStringEx(&ptr))
        if (*ptr)
            if (ptr) {
                slen=strlen(ptr);
                if ((ptr)[slen-1]=='\n')
                    (ptr)[slen-1]=0;
                line = ptr;
				free(ptr);
                return true;
            }

    return false;
}

bool MMSFile::getChar(char *ptr) {
    char    retc;
    size_t  ritems;

    if (!ptr) ptr=&retc;

    if (readBuffer(ptr, &ritems, 1, 1))
        if (ritems==1)
            return true;

    return false;
}


bool MMSFile::writeBuffer(void *ptr, size_t *ritems, size_t size, size_t nitems) {
    size_t  myri;

    /* clear error */
    this->lasterror = 0;

    /* clear ritems */
    if (!ritems) ritems=&myri;
    *ritems=0;

    /* check input */
    if ((!size) || (!nitems)) {
        this->lasterror = EINVAL;
        return false;
    }

    /* check if this->mode allowes to write to the file */
    switch(this->type) {
        case MMSFT_FILE:
            if (this->mode==MMSFM_READ) {
                this->lasterror = EBADF;
                return false;
            }
            break;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            /* currently I cannot write to an url stream */
            /* so I will always fail */
            this->lasterror = EBADF;
            return false;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif

        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }

    /* cache given? */
    if (this->usecache) {
        /* work with separate cache is not supported in this version */
        this->lasterror = EBADF;
        return false;
    }

    /* normal access without separate cache */
    switch(this->type) {
        case MMSFT_FILE:
            if (this->file) {
#ifdef __L4_RE__
                *ritems = VfsWrapper::instance().fwrite(ptr, size, nitems, this->file);
#else
                *ritems = fwrite(ptr, size, nitems, this->file);
#endif
                if (*ritems < nitems) {
                    /* a write error */
                    this->lasterror = EBADF;
                    return false;
                }
                return true;
            }
            this->lasterror = EBADF;
            return false;

        case MMSFT_URL:
#ifdef __HAVE_CURL__
            if (this->curl) {
                /* currently I cannot write to an url stream */
                /* so I will always fail */
            }
            this->lasterror = EBADF;
            return false;
#else
    		throw MMSFileError(-1, "compile curl support!");
#endif

        default:
            /* unknown type */
            this->lasterror = EBADF;
            return false;
    }
}
