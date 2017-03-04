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
  QDateTime now(QDateTime::currentDateTime());
  for(int i=0;i<2;i++) {
    for(int j=0;j<SWITCHYARD_GPIO_BUNDLE_SIZE;j++) {
      entry_watchdog_states[i][j]=false;
      entry_timestamps[i][j]=now;
    }
  }
}


ConfigEntry *Config::configEntry(unsigned srcnum) const
{
  try {
    return conf_entries.at(srcnum);
  }
  catch(...)
    {
    }
  return NULL;
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


QString ConfigEntry::gpioAction(SyGpioEvent::Type type,int line) const
{
  return entry_actions[type][line];
}


void ConfigEntry::setGpioAction(SyGpioEvent::Type type,int line,
				const QString &filename)
{
  entry_actions[type][line]=filename;
}


int ConfigEntry::gpioWatchdogTimeout(SyGpioEvent::Type type,int line) const
{
  return entry_watchdog_timeouts[type][line];
}


void ConfigEntry::setGpioWatchdogTimeout(SyGpioEvent::Type type,int line,
					 int msec)
{
  entry_watchdog_timeouts[type][line]=msec;
}


QString ConfigEntry::gpioWatchdogAction(SyGpioEvent::Type type,int line) const
{
  return entry_watchdog_actions[type][line];
}


void ConfigEntry::setGpioWatchdogAction(SyGpioEvent::Type type,int line,
					const QString &filename)
{
  entry_watchdog_actions[type][line]=filename;
}


bool ConfigEntry::gpioWatchdogState(SyGpioEvent::Type type,int line) const
{
  return entry_watchdog_states[type][line];
}


void ConfigEntry::setGpioWatchdogState(SyGpioEvent::Type type,int line,
				       bool state)
{
  entry_watchdog_states[type][line]=state;
}


QDateTime ConfigEntry::timestamp(SyGpioEvent::Type type,int line)
{
  return entry_timestamps[type][line];
}


void ConfigEntry::touchTimestamp(SyGpioEvent::Type type,int line)
{
  entry_timestamps[type][line].setDate(QDate::currentDate());
  entry_timestamps[type][line].setTime(QTime::currentTime());
}




Config::Config(QObject *parent)
  : QObject(parent)
{
  conf_watchdog_timer=new QTimer(this);
  connect(conf_watchdog_timer,SIGNAL(timeout()),
	  this,SLOT(checkWatchdogsData()));
}


bool Config::logActive(SyGpioEvent *e)
{
  ConfigEntry *entry=NULL;

  if((entry=configEntry(e->sourceNumber()))==NULL) {
    return false;
  }
  return (!entry->logDir(e->type(),e->line()).isEmpty())&&
    (!entry->gpioString(e->type(),GetRole(e),e->line()).isEmpty());
}


QString Config::logDir(SyGpioEvent *e)
{
  return conf_entries.at(e->sourceNumber())->logDir(e->type(),e->line());
}


QString Config::logFilename(SyGpioEvent *e)
{
  return logDir(e)+"/"+QDate::currentDate().toString("yyyy-MM-dd")+".txt";
}


bool Config::scriptActive(SyGpioEvent *e)
{
  ConfigEntry *entry=NULL;

  if((entry=configEntry(e->sourceNumber()))==NULL) {
    return false;
  }
  return !entry->gpioAction(e->type(),e->line()).isEmpty();
}


QString Config::gpioAction(SyGpioEvent *e)
{
  return conf_entries.at(e->sourceNumber())->gpioAction(e->type(),e->line());
}


void Config::touchTimestamp(SyGpioEvent *e)
{
  try {
    conf_entries.at(e->sourceNumber())->touchTimestamp(e->type(),e->line());
  }
  catch(...)
    {
    }
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
	  e->setLogDir(type,k,p->stringValue(sections.at(i),TypeString(type)+
					    QString().sprintf("%dLogDir",k+1)));
	  e->setGpioString(type,ConfigEntry::Off,k,
			   p->stringValue(sections.at(i),TypeString(type)+
					  QString().sprintf("%dOff",k+1)));
	  e->setGpioString(type,ConfigEntry::On,k,
			   p->stringValue(sections.at(i),TypeString(type)+
					  QString().sprintf("%dOn",k+1)));
	  e->setGpioString(type,ConfigEntry::Pulse,k,
			   p->stringValue(sections.at(i),TypeString(type)+
					  QString().sprintf("%dPulse",k+1)));
	  e->setGpioString(type,ConfigEntry::WatchdogSet,k,
			   p->stringValue(sections.at(i),TypeString(type)+
				     QString().sprintf("%dWatchdogSet",k+1)));
	  e->setGpioString(type,ConfigEntry::WatchdogReset,k,
			   p->stringValue(sections.at(i),TypeString(type)+
				     QString().sprintf("%dWatchdogReset",k+1)));
	  e->setGpioAction(type,k,
			   p->stringValue(sections.at(i),TypeString(type)+
					  QString().sprintf("%dAction",k+1)));
	  e->setGpioWatchdogTimeout(type,k,
		   p->intValue(sections.at(i),TypeString(type)+
			       QString().sprintf("%dWatchdogTimeout",k+1)));
	  e->setGpioWatchdogAction(type,k,
       		   p->stringValue(sections.at(i),TypeString(type)+
				  QString().sprintf("%dWatchdogAction",k+1)));
	}
      }
    }
  }
  conf_watchdog_timer->start(1000);
  return true;
}


QString Config::gpioTypeText(SyGpioEvent::Type type)
{
  if(type==SyGpioEvent::TypeGpi) {
    return QString("gpi");
  }
  return QString("gpo");
}


QString Config::stateText(bool state)
{
  if(state) {
    return QString("true");
  }
  return QString("false");
}


void Config::checkWatchdogsData()
{
  QDateTime now=QDateTime::currentDateTime();

  for(std::map<unsigned,ConfigEntry *>::const_iterator it=conf_entries.begin();
      it!=conf_entries.end();it++) {
    ConfigEntry *ce=it->second;
    for(int i=0;i<2;i++) {
      SyGpioEvent::Type type=(SyGpioEvent::Type)i;
      for(int j=0;j<SWITCHYARD_GPIO_BUNDLE_SIZE;j++) {
	if(ce->gpioWatchdogTimeout(type,j)>0) {
	  if((!ce->gpioWatchdogState(type,j))&&
	     (ce->timestamp(type,j).addSecs(ce->gpioWatchdogTimeout(type,j))<
	      now)) {
	    ce->setGpioWatchdogState(type,j,true);
	    emit watchdogStateChanged(ce,type,j,true);
	  }
	  if(ce->gpioWatchdogState(type,j)&&
	     (ce->timestamp(type,j).addSecs(ce->gpioWatchdogTimeout(type,j))>
	      now)) {
	    ce->setGpioWatchdogState(type,j,false);
	    emit watchdogStateChanged(ce,type,j,false);
	  }
	}
      }
    }
  }
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


QString Config::TypeString(SyGpioEvent::Type type) const
{
  if(type==SyGpioEvent::TypeGpi) {
    return "Gpi";
  }
  return "Gpo";
}
