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

#ifndef MMSMAIL_H_
#define MMSMAIL_H_
#ifdef __HAVE_VMIME__
#include "mmstools/base.h"
#include "mmstools/mmserror.h"
#include <vmime/vmime.hpp>
#include <vmime/platforms/posix/posixHandler.hpp>
#include <vector>

/**
 *  A smtp mail transfer class.
 *
 *  This class provides functions to to send e-mails via a smtp server.
 *  Secure connections are note supported.
 *
 *  @author Patrick Helterhoff
 */
class MMSMail {
private:
	std::string							subject;
	std::string							returnAddress;
	std::vector<std::string>			recipients;
	std::string							mailBody;
	std::string						  	host;
	vmime::ref<vmime::net::transport> 	transportService;

public:
	/**
	 * Constructor of class MMSMail.
	 *
	 * @param _subject		 [in] the mail subject line
	 * @param _returnAddress [in] the return address
	 * @param _host			 [in] the smtp server name
	 */
	MMSMail(const std::string _subject, const std::string _returnAddress, const std::string _host);

	/** Destructor of class MMSMail. */
	~MMSMail();

    /**
     * Returns the mail subject line.
     */
	std::string getSubject() const;

	/**
	 * Sets the mail subject line.
	 *
	 * @param subject [in] the mail subject
	 */
    void setSubject(const std::string subject);

    /**
     * Returns the mails return addresss.
     */
    std::string getReturnAddress() const;
    /**
     * Sets the mails return address.
     *
     * @param returnAddress [in] the mails return address
     */
    void setReturnAddress(std::string returnAddress);

    /**
     * Returns the recipient at the given index.
     *
     * @param index [in] index of the recipient (first element = 0)
     */
    std::string getRecipient(int index) const;

    /**
     * Returns the current number of recipients.
     */
    const int getRecipientCount() const;

    /**
     * Adds a recipient to the current mail.
     */
    void addRecipient(std::string recipient);

    /**
     * Removes a recipient at the given index.
     *
     * @param index [in] index of the recipient (first element = 0)
     */
    void removeRecipient(int index);

    /**
     * Returns the mails message text.
     */
    std::string getMailBody() const;

    /**
     * Sets the mails message text.
     *
     * @param mailBody [in] Message text of the mail
     */
    void setMailBody(std::string mailBody);

    /**
     * Returns the current mail servers address.
     */
    std::string getHost() const;

    /**
     * Sets the mail servers address.
     *
     * @param host [in] the mail servers address
     */
    void setHost(std::string host);

    /**
     * Use this to set user and password for the smtp server connection, if necessary.
     *
     * @param   user    	[in] the ftp user
     * @param   password   	[in] the password
     */
    void setAuthData(const std::string &user, const std::string &password);

    /**
     * Sends the mail to the specified recipients.
     *
     * @throws MMSError if errors occure during the mail send.
     */
    void send();
};

#endif
#endif /* MMSMAIL_H_ */
