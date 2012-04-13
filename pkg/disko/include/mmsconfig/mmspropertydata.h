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

#ifndef MMSPROPERTYDATA_H_
#define MMSPROPERTYDATA_H_

using namespace std;

#include <string>
#include <vector>

#define MMSPROPERTYTYPE_STRING  "string"
#define MMSPROPERTYTYPE_INTEGER "integer"

class MMSPropertyData {

    friend class MMSPluginPropertyDAO;

    private:
        int     id;
        string  parameter;
        string  value;
        string  type;
        int     max;
        int 	min;
        vector <string> vallist;
        char	separator;
        bool    issetindb;

        void    setID(int id);

    public:
        MMSPropertyData();
        int     getID();
        string  getParameter();
        void    setParameter(string parameter);
        string  getValue();
        void    setValue(string value);
        string  getType();
        void    setType(string type);
        int     getMax();
        void    setMax(int max);
        int     getMin();
        void    setMin(int min);
        vector <string> getVallist();
        void    setVallist(vector <string> vallist);
        char    getSeparator();
        void    setSeparator(char separator);
        bool    isSetInDb();
        void    setisSetinDb(bool issetindb);
};
#endif /*MMSPROPERTYDATA_H_*/
