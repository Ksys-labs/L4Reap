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
#include "cmd.h"
#include "mmsinfo/mmsinfo.h"

static bool isdigit(const char *what) {
    if(atoi(what)>0)
        return true;

    return false;
}


void Cmd::executeEvent() {
	string heading = cmdline["heading"];

	int port;
	if(!cmdline["port"].empty())
		port = strToInt(cmdline["port"]);
	else
		port=10001;


	int rc;
	string err;
	map<string, string> parameter;

	if(!cmdline["param"].empty()) {
		parameter.insert(std::make_pair(cmdline["param"],cmdline["value"]));
	}

	MMSXMLClientInterface *ipcclient = new MMSXMLClientInterface("127.0.0.1",port);
	if(parameter.empty()) {
	if(!ipcclient->funcSendEvent(heading, NULL, 1, &rc, &err)) {
	    	cons.printError("Could not send event!");
	    	exit(1);
		} else {
	         cons.printText("Event sucessfully sent.");
	         exit(0);
		}
	} else if(!ipcclient->funcSendEvent(heading, &parameter, 1, &rc, &err)) {
    	cons.printError("Could not send event!");
    	exit(1);
	} else {
         cons.printText("Event sucessfully sent.");
         exit(0);
	}
}


void Cmd::executeExec() {
    MMSPluginService service(datasource);
    string import=cmdline["import"];
    MMSPluginData *data = NULL;

    if(import.empty()) {
    	cons.printError("No import plugin is given to execute. See cmd --help for further information.");
    	exit(1);
    }

    if(isdigit(import.c_str()))
    	data = service.getPluginByID(atoi(import.c_str()));
    else
    	data = service.getPluginByName(import);


    if(data!=NULL) {
        if(data->getType()->getName() != PT_IMPORT_PLUGIN) {
            char text[1024];
            sprintf(text, "Plugin with name or id %s is not an import plugin. ", import.c_str());
            cons.printError(text);
            exit(1);
        }
        MMSImportPluginHandler myhandler = MMSImportPluginHandler(*data, true);
         myhandler.invokeInitialize(NULL);

         char text[1024];
         sprintf(text,"executing import %s. ", data->getName().c_str());
         cons.linefeed();
         cons.printText(text);

         myhandler.invokeExecute(NULL);
         sprintf(text,"import %s sucessfully executed. ", data->getName().c_str());
         cons.printText(text);


    } else {
        char text[1024];
        sprintf(text, "Import plugin with name or id %s not found.", import.c_str());
        cons.printError(text);
        exit(1);
    }

}

void Cmd::executeList() {
	bool printed = false;
    MMSPluginService service(datasource);

	string type = this->cmdline["type"];
	string plugin = this->cmdline["plugin"];

	// just list the plugins
	if(type.empty()||type=="all") {

		vector<MMSPluginData *> result = service.getAllPlugins(true);
	    for(vector<MMSPluginData *>::iterator it = result.begin(); it != result.end(); it++) {
	    	if(plugin.empty()) {
	    		this->printPlugin(*it,true);
	    		printed=true;
	    	} else if(((*it)->getName()==plugin) || ((*it)->getId()==atoi(plugin.c_str()))) {
	    		this->printPlugin(*it,true);
	    		printed=true;
	    	}
	    }

	    MMSPluginData *dat = service.getPluginByID(-2);
	    if(dat) {
    		this->printPlugin(dat,true);
    		//printed=true;
	    }

	} else if(type=="imports") {

		vector<MMSPluginData *> imports = service.getImportPlugins(true);
        MMSImportSourceService *imsource = new MMSImportSourceService(datasource);
        MMSImportPropertyService *improp = new MMSImportPropertyService(datasource);
        for(vector<MMSPluginData *>::iterator it = imports.begin(); it != imports.end(); it++) {

   			printed=true;
        	this->printPlugin(*it,false);

        	MMSImportPropertyData *prop = improp->getImportPropertyByPlugin(*it);
            this->printImportproperty(prop);

            vector<MMSImportSourceData *> srcs = imsource->getImportSourcesByPlugin(*it);
            for(vector<MMSImportSourceData *>::iterator it2 = srcs.begin(); it2 != srcs.end(); it2++) {
                this->printImportsource(*it2);
            }

        }
	} else {
		char message[1024];
		sprintf(message, "Type @[1m%s@[0m is not valid. See cmd --help for further informations.",type.c_str());
		cons.printError(message);
	}


    if(printed == false) {
    	printf("\nno plugin found!\n");
    }
}

void Cmd::executeUpdate() {
	string type;

	type = this->cmdline[MMSCMD_SOURCE];
	if(type.empty()) {
		type = cmdline[MMSCMD_PARAM];
		if(type.empty()) {
			cons.printError("Could not determine what to update. See cmd --help for further information.");
		} else
			this->updateParameter();
	} else
		this->updateSource();
}

void Cmd::executeAct() {
    Console cons(true);
    string pluginname;

    MMSPluginService service(datasource);

    pluginname = cmdline["plugin"];
    if(pluginname.empty()) {
    	cons.printError("no plugin given to activate");
    	exit(1);
    }

    if(isdigit(pluginname.c_str())) {
        //we have a plugin id;
        MMSPluginData *plugin = service.getPluginByID(atoi(pluginname.c_str()));
        if(plugin != NULL) {
            if(plugin->getActive()==true) {
                string message = "plugin with id @[1m" + pluginname + "@[0m is already active.";
                cons.printText((char *)message.c_str());
                exit(0);
            }

            plugin->setActive(true);
            service.setPlugin(plugin);
            string message = "plugin with id @[1m" + pluginname + "@[0m sucessfully set to @[1mactive@[0m.";
            cons.printText((char *)message.c_str());

        } else {
            string message = "no plugin with id @[1m" + pluginname + "@[0m found.";
            cons.printText((char *)message.c_str());

        }

    } else {
        //it looks like a name
        MMSPluginData *plugin = service.getPluginByName(pluginname);
        if(plugin != NULL) {
            if(plugin->getActive()==true) {
                string message = "plugin " + pluginname + " is already active.";
                cons.printText((char *)message.c_str());
                exit(0);
            }

            plugin->setActive(true);
            service.setPlugin(plugin);
            string message = "plugin @[1m" + pluginname + "@[0m sucessfully set to @[1mactive@[0m.";
            cons.printText((char *)message.c_str());
        }else {
            string message = "no plugin with name @[1m" + pluginname + "@[0m found.";
            cons.printText((char *)message.c_str());
        }
    }
}

void Cmd::executeDeact() {
    string pluginname;

    MMSPluginService service(datasource);

    pluginname = cmdline["plugin"];
    if(pluginname.empty()) {
    	cons.printError("no plugin given to activate");
    	exit(1);
    }

    if(isdigit(pluginname.c_str())) {
        //we have a plugin id;
        MMSPluginData *plugin = service.getPluginByID(atoi(pluginname.c_str()));
        if(plugin != NULL) {
            if(plugin->getActive()==false) {
                string message = "plugin with id @[1m" + pluginname + "@[0m is already inactive.";
                cons.printText((char *)message.c_str());
                exit(0);
            }

            plugin->setActive(false);
            service.setPlugin(plugin);
            string message = "plugin with id @[1m" + pluginname + "@[0m sucessfully set to @[1minactive@[0m.";
            cons.printText((char *)message.c_str());

        } else {
            string message = "no plugin with id @[1m" + pluginname + "@[0m found.";
            cons.printText((char *)message.c_str());
        }
    } else {
        //it looks like a name
        MMSPluginData *plugin = service.getPluginByName(pluginname);
        if(plugin != NULL) {
            if(plugin->getActive()==false) {
                string message = "plugin @[1m" + pluginname + "@[0m is already inactive.";
                cons.printText((char *)message.c_str());
                exit(0);
            }

            plugin->setActive(false);
            service.setPlugin(plugin);
            string message = "plugin @[1m" + pluginname + "@[0m sucessfully set to @[1inmactive@[0m.";
            cons.printText((char *)message.c_str());
        } else {
            string message = "no plugin with name @[1m" + pluginname + "@[0m found.";
            cons.printText((char *)message.c_str());
        }
    }
}

void Cmd::updateSource() {
    MMSImportSourceService *sserv;
    MMSImportSourceData *data = NULL;
    int id;
    string source = cmdline[MMSCMD_SOURCE];
    string value = cmdline[MMSCMD_VALUE];

    if(source.empty()) {
    	cons.printError("No source name or id given. See cmd --help for further information.");
    	exit(1);
    }
    if(value.empty()) {
    	cons.printError("No new value given. See cmd --help for further information.");
    	exit(1);
    }

    sserv = new MMSImportSourceService(datasource);
    id = strToInt(source);
    if(id>0) {
        data = sserv->getImportSourcesByID(id);
    } else {
        data = sserv->getImportSourcesByName(source);
    }

    if(data!=NULL) {
        data->setSource(value);
        vector<MMSImportSourceData *> datalist;
        datalist.push_back(data);
        sserv->setImportSource(datalist);
        cons.printText("source sucessfully changed.");
    } else {
    	char message[1024];
    	sprintf(message,"No import source with id or name @[1m%s@[0m found.", source.c_str());
        cons.printText(message);
    }

}

void Cmd::updateParameter() {
    MMSPluginData *plugindata;
    MMSPropertyData *parameterdata;
    MMSPluginParameterParser paramsparser;
    Console cons(true);
    string plugin;
    string parameter;
    string value;


    plugin=cmdline[MMSCMD_PLUGIN];
    parameter=cmdline[MMSCMD_PARAM];
    value=cmdline[MMSCMD_VALUE];

    if(plugin.empty()) {
    	/*cons.printError("No plugin given to update. See cmd --help for further information.");
    	exit(1);*/
    	plugin="-2";
    }
    if(parameter.empty()) {
    	cons.printError("No parameter given to update. See cmd --help for further information.");
    	exit(1);
    }
    if(value.empty()) {
    	cons.printError("No parameter value given to update. See cmd --help for further information.");
    	exit(1);
    }

    MMSPluginService service(datasource);

    if(isdigit(plugin.c_str())||plugin=="-2")
        plugindata = service.getPluginByID(atoi(plugin.c_str()));
    else
        plugindata = service.getPluginByName(plugin);

    if(plugindata == NULL) {
        string mess = "A plugin with name or id @[1m" + plugin + "@[0m was not found!";
        cons.printText((char *)mess.c_str());
        exit(1);
    }

    //paramsparser.createProperty(plugindata,parameter);

    parameterdata = plugindata->getProperty(parameter);

    parameterdata->setValue(value);

    service.setPlugin(plugindata);

    cons.printText("Parameter sucessfully updated.");

}


MMSCMD_COMMANDS Cmd::getCommand() {
	string command = this->cmdline["cmd"];

	if(strcmp(command.c_str(),MMSCMD_LIST)==0)
		return LIST;
	if(strcmp(command.c_str(),MMSCMD_UPDATE)==0)
		return UPDATE;
	if(strcmp(command.c_str(),MMSCMD_ACT)==0)
		return ACT;
    if(strcmp(command.c_str(),MMSCMD_DEACT)==0)
        return DEACT;
    if(strcmp(command.c_str(),MMSCMD_EXEC)==0)
        return EXEC;
    if(strcmp(command.c_str(),MMSCMD_EVENT)==0)
        return EVENT;
    if(strcmp(command.c_str(),MMSCMD_VERSION)==0)
        return VERSION;

	return UNKNOWN;
}

void Cmd::printPlugin(MMSPluginData *plugin, bool full) {

	if(full==true) {
		if(plugin->getId()!=-2) {
			printf("\n%c[1m%s%c[0m (id=%d)\n",27,plugin->getName().c_str(),27,plugin->getId());
			printf("\tdesc:   %s\n",plugin->getDescription().c_str());
			printf("\tactive: %s\n",(plugin->getActive()==true) ? ("yes") : ("no"));
			printf("\ttype:   %s\n\n",plugin->getType()->getName().c_str());
		} else {
			if(!plugin->getProperties().empty()) {
				printf("\n%c[1mSYSTEM PROPERTIES%c[0m\n",27,27);
			}
		}
        if(!plugin->getProperties().empty())
            printf("\tparameters (parameter|value): \n");

        for(unsigned int i = 0;i < plugin->getProperties().size();i++) {
            printf("\t  %s | %s\n",(plugin->getProperties().at(i)->getParameter()).c_str(),(plugin->getProperties().at(i)->getValue()).c_str());
        }


	}
    else {
        printf("%c[1m%s%c[0m (id=%d)\n",27,plugin->getName().c_str(),27,plugin->getId());
    }

}

void Cmd::printImportproperty(MMSImportPropertyData *prop) {

    printf("\n\tstartup:  %s\n",(prop->getOnStartUp()==true) ? ("yes") : ("no"));
    printf("\ttime:     %d seconds from startup\n",prop->getTime());
    printf("\tinterval: every %d seconds\n\n",prop->getInterval());


}

void Cmd::printImportsource(MMSImportSourceData *src) {

	printf("\n\tname(id): %s(%d)\n", src->getName().c_str(),src->getId());
    printf("\tsource:   %s\n\n",src->getSource().c_str());

}

Cmd::Cmd(CmdLine &cmdline) {
	this->cmdline = cmdline;
	cons.setRichformat(true);
}

Cmd::~Cmd() {

}

void Cmd::prepareDb() {
    MMSRcParser         	rcparser;
    MMSConfigDataGlobal		*rcGlobal;
    MMSConfigDataDB     	*rcConfigDB, *rcDataDB;
    MMSConfigDataGraphics	*rcGraphics;
    MMSConfigDataLanguage   *rcLanguage;

    //get datasource
    try {

    	rcparser.parseFile(cmdline["config"]);
	    rcparser.getMMSRc(&rcGlobal, &rcConfigDB, &rcDataDB, &rcGraphics,&rcLanguage);

    } catch (MMSRcParserError &ex) {
    	char error[1024];
    	sprintf(error,"cannot read configuration file @[1m%s@[0m.",configfile.c_str());
        cons.printError(error);
        exit(1);
    }

    rcGlobal->stdout = true;
    MMSConfigData config = MMSConfigData(*rcGlobal, *rcConfigDB, *rcDataDB, *rcGraphics,*rcLanguage);

    //printworkingdata(&config);

    try {
        /* get the active osd and central plugins */
        datasource = new DataSource(config.getConfigDBDBMS(),
                                           config.getConfigDBDatabase(),
                                           config.getConfigDBAddress(),
                                           config.getConfigDBPort(),
                                           config.getConfigDBUser(),
                                           config.getConfigDBPassword());
    } catch(MMSError &ex) {
    	cons.printText("cannot get data source.");
    }

}

void Cmd::executeVersion() {
	bool havemedia = false;
	bool havebackend = false;
	bool havedb = false;
	printf("Disko the embedded GUI framework\n");
	printf(" version:       %s\n", DISKO_VERSION_STR);
	printf(" prefix:        %s\n", getPrefix());
	printf(" media support: ");
#ifdef __HAVE_XINE__
	printf("xine");
	havemedia = true;
#endif
#ifdef __HAVE_GSTREAMER__
	if(havemedia) {
		printf(", gstreamer");
	} else {
		printf("gstreamer");
		havemedia=true;
	}
#endif
#ifdef __HAVE_MXIER__
	if(havemedia) {
		printf(", alsa mixing");
	} else {
		printf("alsa mixing");
		havemedia=true;
	}
#endif
	if(!havemedia) {
		printf("none\n");
	} else {
		printf("\n");
	}
	printf(" gfx support:   ");
#ifdef __HAVE_FBDEV__
	printf("fbdev");
	havebackend = true;
#endif
#ifdef __HAVE_DIRECTFB__
	if(havebackend) {
		printf(", directfb");
	} else {
		printf("directfb");
		havebackend = true;
	}
#endif
#ifdef __HAVE_XLIB__
	if(havebackend) {
		printf(", x11");
	} else {
		printf("x11");
		havebackend = true;
	}
#endif
	if(!havebackend) {
		printf("none\n");
	} else {
		printf("\n");
	}
	printf(" db support:    ");

#ifdef __ENABLE_SQLITE__
	printf("sqlite");
	havedb = true;
#endif
#ifdef __ENABLE_MYSQL__
	if(havedb) {
		printf(", mysql");
	} else {
		printf("mysql");
		havedb = true;
	}
#endif
#ifdef __ENABLE_FREETDS__
	if(havedb) {
		printf(", freetds");
	} else {
		printf("freetds");
		havedb = true;
	}
#endif
	if(!havedb) {
		printf("none\n");
	} else {
		printf("\n");
	}
#ifdef __HAVE_CURL__
	printf(" curl support:  no\n");
#else
	printf(" curl support:  yes\n");
#endif
#ifdef __HAVE_DL__
	printf(" dynamic libs:  yes\n");
#else
	printf(" dynamic libs:  no\n");
#endif
#ifdef __HAVE_MMSCRYPT__
	printf(" ssl support:   yes\n");
#else
	printf(" ssl support:   no\n");
#endif
	printf("\n");
	exit(0);
}

void Cmd::handleRequest() {

    switch(getCommand()) {
		case LIST :
			prepareDb();
			executeList();
			break;
		case UPDATE :
			prepareDb();
			executeUpdate();
			break;
		case ACT :
			prepareDb();
			executeAct();
			break;
        case DEACT :
			prepareDb();
            executeDeact();
            break;
        case EXEC :
			prepareDb();
            executeExec();
            break;
        case EVENT :
			prepareDb();
            executeEvent();
            break;
        case VERSION :
        	executeVersion();
		default:
			cons.printError("no command found");
			break;
	}

}

void Cmd::printWorkingdata(MMSConfigData *config) {
	printf("\nPrefix:    %s\n",config->getPrefix().c_str());
	printf("Data DB:   %s\n",config->getDataDBDatabase().c_str());
	printf("Config DB: %s\n\n",config->getConfigDBDatabase().c_str());
}
