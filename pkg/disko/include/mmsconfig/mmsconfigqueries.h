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

#ifndef MMSCONFIGQUERIES_H_
#define MMSCONFIGQUERIES_H_

/****************************************************************************************/
/*                                                                                      */
/* All queries used in the PluginDAO class                                              */
/*                                                                                      */
/****************************************************************************************/
#define PLUGINDAO_SAVE(ID, Name, Title, Description, Filename, Path, Active, Icon, Icon_s, Icon_small, Icon_small_s, CategoryID, Orderpos, Version) \
        "insert into Plugins(PluginTypeID,PluginName,PluginTitle,PluginDescription,Filename,PluginPath,Active,Icon,SelectedIcon,SmallIcon,SmallSelectedIcon,CategoryID,Orderpos,Version) values('" \
        + ID + "','"          \
        + Name + "','"        \
        + Title + "','"       \
        + Description + "','" \
        + Filename + "','"    \
        + Path + "','"        \
        + Active + "','"      \
        + Icon + "','"        \
        + Icon_s + "','"      \
        + Icon_small + "','"  \
        + Icon_small_s + "','"  \
        + CategoryID + "','"  \
        + Orderpos + "','" \
        + Version + "')"

#define PLUGINDAO_UPDATE(Filename, Active, Description, CategoryID, Orderpos, ID, Version) \
        "update Plugins set Filename='" + Filename + "',"   \
        + "Active='" + Active + "',"                        \
        + "PluginDescription='" + Description + "', " +      \
        + "CategoryID='" + CategoryID + "', " +      \
        + "Orderpos='" + Orderpos + "', " +      \
        + "Version='" + Version + "' " + \
        "where ID = '" + ID + "'"

#define PLUGINDAO_FIND_ALL_ACTIVE_PLUGINS \
		"select Plug.*, Cat.CategoryName, PlugType.PluginTypeName from Plugins Plug left join Category Cat ON Cat.ID = Plug.CategoryID left join PluginTypes PlugType ON PlugType.ID = Plug.PluginTypeID where Plug.Active = 'Y' and Plug.ID != -2"

#define PLUGINDAO_FIND_ALL_PLUGINS	 \
	    "select Plug.*, Cat.CategoryName, PlugType.PluginTypeName from Plugins Plug left join Category Cat ON Cat.ID = Plug.CategoryID left join PluginTypes PlugType ON PlugType.ID = Plug.PluginTypeID where Plug.ID != -2"

#define PLUGINDAO_F_PLUGIN_BY_NAME(Name) \
	    "select Plug.*, Cat.CategoryName, PlugType.PluginTypeName from Plugins Plug left join Category Cat ON Cat.ID = Plug.CategoryID left join PluginTypes PlugType ON PlugType.ID = Plug.PluginTypeID where Plug.PluginName = '" + Name + "';"

#define PLUGINDAO_F_PLUGIN_BY_ID(ID) \
    	"select Plug.*, Cat.CategoryName, PlugType.PluginTypeName from Plugins Plug left join Category Cat ON Cat.ID = Plug.CategoryID left join PluginTypes PlugType ON PlugType.ID = Plug.PluginTypeID where Plug.ID = " + ID

#define PLUGINDAO_F_ACTIVE_PLUGINS_BY_CATEGORY(CATEGORY) \
	    "select Plug.*,Cat.CategoryName,Types.PluginTypename from Plugins Plug left join Category Cat ON Cat.CategoryName ='" + CATEGORY + "' left join PluginTypes Types ON Types.ID = Plug.PluginTypeID WHERE Plug.CategoryID = Cat.ID and Plug.Active = 'Y'"

#define PLUGINDAO_F_ALL_PLUGINS_BY_CATEGORY(CATEGORY) \
	    "select Plug.*,Cat.CategoryName,Types.PluginTypename from Plugins Plug left join Category Cat ON Cat.CategoryName ='" + CATEGORY + "' left join PluginTypes Types ON Types.ID = Plug.PluginTypeID WHERE Plug.CategoryID = Cat.ID"

#define PLUGINDAO_F_ACTIVE_PLUGINS_BY_TYPE(TYPE) \
    	"select Plug.*,Cat.CategoryName,Types.PluginTypeName from Plugins Plug left join Category Cat ON Cat.ID  = Plug.CategoryID left join PluginTypes Types ON Types.ID = Plug.PluginTypeID where Types.PluginTypeName = '" + TYPE + "' and Plug.Active = 'Y'"

#define PLUGINDAO_F_ALL_PLUGINS_BY_TYPE(TYPE) \
    	"select Plug.*,Cat.CategoryName,Types.PluginTypeName from Plugins Plug left join Category Cat ON Cat.ID  = Plug.CategoryID left join PluginTypes Types ON Types.ID = Plug.PluginTypeID where Types.PluginTypeName = '" + TYPE + "'"

/****************************************************************************************/
/*                                                                                      */
/* All queries used in the PluginPropertiesDAO class                                    */
/*                                                                                      */
/****************************************************************************************/
#define PLUGINPROPERTYDAO_FIND_ALL_PLUGIN_PROPERTIES_BY_PLUGIN(ID) \
		"select * from PluginProperties where PluginID = " + ID

#endif /*MMSCONFIGQUERIES_H_*/
