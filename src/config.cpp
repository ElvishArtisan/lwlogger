// config.cpp
//
// Configuration for lwlogger(8)
//
//   (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <sy/syprofile.h>

#include "config.h"

ConfigEntry::ConfigEntry(int srcnum)
{
  entry_source_number=srcnum;
}


int ConfigEntry::sourceNumber() const
{
  return entry_source_number;
}


QString ConfigEntry::logDir(SyGpioEvent::Type type,int line) const
{
  return entry_log_dirs[type][line];
}


void ConfigEntry::setLogDir(SyGpioEvent::Type type,int line,
			    const QString &str)
{
  entry_log_dirs[type][line]=str;
}


QString ConfigEntry::gpioString(SyGpioEvent::Type type,ConfigEntry::Role role,
				int line)
{
  return entry_strings[type][role][line];
}


    void ConfigEntry::setGpioString(SyGpioEvent::Type type,
				    ConfigEntry::Role role,int line,
				    const QString &str)
{
  entry_strings[type][role][line]=str;
}




Config::Config(QObject *parent)
{
}


bool Config::logActive(SyGpioEvent *e)
{
  ConfigEntry *entry=NULL;

  if((entry=GetConfigEntry(e->sourceNumber()))==NULL) {
    return false;
  }
  return (!entry->logDir(e->type(),e->line()).isEmpty())&&
    (!entry->gpioString(e->type(),GetRole(e),e->line()).isEmpty());
}


QString Config::logDir(SyGpioEvent *e)
{
  return conf_entries.at(e->sourceNumber())->logDir(e->type(),e->line());
}


QString Config::logLine(SyGpioEvent *e)
{
  return conf_entries.at(e->sourceNumber())->
    gpioString(e->type(),GetRole(e),e->line());
}


bool Config::load()
{
  SyProfile *p=new SyProfile();
  p->setSource(CONFIG_FILE);
  QStringList sections=p->sectionNames();
  bool ok=false;

  for(int i=0;i<sections.size();i++) {
    unsigned srcnum=sections.at(i).toUInt(&ok);
    if(ok&&(srcnum>0)&&(srcnum<32768)) {
      conf_entries[srcnum]=new ConfigEntry(srcnum);
      ConfigEntry *e=conf_entries.at(srcnum);
      for(int j=0;j<2;j++) {
	SyGpioEvent::Type type=(SyGpioEvent::Type)j;
	for(int k=0;k<SWITCHYARD_GPIO_BUNDLE_SIZE;k++) {
	  e->setLogDir(type,k,p->stringValue(sections.at(i),QString().
					       sprintf("Gpi%dLogDir",k+1)));
	  e->setGpioString(type,ConfigEntry::Off,k,
	    p->stringValue(sections.at(i),QString().sprintf("Gpi%dOff",k+1)));
	  e->setGpioString(type,ConfigEntry::On,k,
	    p->stringValue(sections.at(i),QString().sprintf("Gpi%dOn",k+1)));
	  e->setGpioString(type,ConfigEntry::Pulse,k,
	    p->stringValue(sections.at(i),QString().sprintf("Gpi%dPulse",k+1)));
	}
      }
    }
  }
  return true;
}


ConfigEntry *Config::GetConfigEntry(unsigned srcnum) const
{
  try {
    return conf_entries.at(srcnum);
  }
  catch(...)
    {
    }
  return NULL;
}


ConfigEntry::Role Config::GetRole(SyGpioEvent *e) const
{
  if(!e->isPulse()) {
    if(e->state()) {
      return ConfigEntry::On;
    }
    return ConfigEntry::Off;
  }
  return ConfigEntry::Pulse;
}
