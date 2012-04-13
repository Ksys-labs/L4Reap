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

#include "mmstools/mmsmail.h"
#ifdef __HAVE_VMIME__
MMSMail::MMSMail(const std::string _subject, const std::string _returnAddress, const std::string _host) :
	subject(_subject), returnAddress(_returnAddress) {
	vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();
	setHost(_host);
}

MMSMail::~MMSMail() {

}

std::string MMSMail::getSubject() const {
    return this->subject;
}

void MMSMail::setSubject(const std::string subject) {
    this->subject = subject;
}

std::string MMSMail::getReturnAddress() const {
    return this->returnAddress;
}

void MMSMail::setReturnAddress(std::string returnAddress) {
    this->returnAddress = returnAddress;
}

std::string MMSMail::getRecipient(int index) const {
	if (!this->recipients.empty() && index < this->recipients.size())
		return this->recipients.at(index);
	else
		return NULL;
}

const int MMSMail::getRecipientCount() const {
	return this->recipients.size();
}

void MMSMail::addRecipient(std::string recipient) {
	this->recipients.push_back(recipient);
}

void MMSMail::removeRecipient(int index) {
	this->recipients.erase(this->recipients.begin() + index);
}

std::string MMSMail::getMailBody() const {
    return mailBody;
}

void MMSMail::setMailBody(std::string mailBody) {
    this->mailBody = mailBody;
}

std::string MMSMail::getHost() const {
	return this->host;
}

void MMSMail::setHost(std::string host) {
	this->host = host;

	std::string hoststring = "smtp://" + host;
	vmime::utility::url url(hoststring);
	vmime::ref <vmime::net::session> sess = vmime::create <vmime::net::session>();
	this->transportService = sess->getTransport(url);
}

void MMSMail::setAuthData(const std::string &user, const std::string &password) {
	this->transportService->setProperty("options.need-authentication", true);
	this->transportService->setProperty("auth.username", user);
	this->transportService->setProperty("auth.password", password);
}

void MMSMail::send() {
	try {
		vmime::messageBuilder mb;
		mb.setSubject(vmime::text(this->subject));
		mb.setExpeditor(vmime::mailbox(this->returnAddress));

		for (std::vector<std::string>::iterator i = this->recipients.begin(); i != this->recipients.end(); i++) {
			mb.getRecipients().appendAddress(vmime::create<vmime::mailbox>(*i));
		}

		mb.getTextPart()->setCharset(vmime::charsets::ISO8859_15);
		mb.getTextPart()->setText(vmime::create<vmime::stringContentHandler>(this->mailBody));
		vmime::ref<vmime::message> msg = mb.construct();

		this->transportService->connect();
		this->transportService->send(msg);
		this->transportService->disconnect();
	}
	catch (vmime::exception& e)
	{
		throw MMSError(0, e.what());
	}
	catch (std::exception& e)
	{
		throw MMSError(0, e.what());
	}
}
#endif
