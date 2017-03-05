// lwlogger.cpp
//
// lwlogger(8) LiveWire routing daemon
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

#include <stdio.h>
#include <syslog.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QStringList>

#include "lwlogger.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  main_config=new Config();
  main_config->load();
  connect(main_config,
	 SIGNAL(watchdogStateChanged(ConfigEntry *,SyGpioEvent::Type,int,bool)),
	  this,
     SLOT(watchdogStateChangedData(ConfigEntry *,SyGpioEvent::Type,int,bool)));

  main_gpio_server=new SyGpioServer(new SyRouting(0,0),this);
  connect(main_gpio_server,SIGNAL(gpioReceived(SyGpioEvent *)),
	  this,SLOT(gpioReceivedData(SyGpioEvent *)));

  main_garbage_timer=new QTimer(this);
  main_garbage_timer->setSingleShot(true);
  connect(main_garbage_timer,SIGNAL(timeout()),this,SLOT(collectGarbageData()));
}


void MainObject::gpioReceivedData(SyGpioEvent *e)
{
  main_config->touchTimestamp(e);
  if(main_config->logActive(e)) {
    Log(main_config->logDir(e),main_config->logLine(e));
  }
  if(main_config->scriptActive(e)) {
    RunGpioScript(main_config->gpioAction(e),main_config->logFilename(e),e);
  }
}


void MainObject::scriptFinishedData()
{
  main_garbage_timer->start(1);
}


void MainObject::collectGarbageData()
{
  for(int i=main_script_list.size()-1;i>=0;i--) {
    if(main_script_list.at(i)->processState()==QProcess::NotRunning) {
      delete main_script_list.at(i);
      main_script_list.erase(main_script_list.begin()+i);
    }
  }
}


void MainObject::watchdogStateChangedData(ConfigEntry *e,SyGpioEvent::Type type,
					  int line,bool state)
{
  ConfigEntry::Role role=ConfigEntry::WatchdogSet;
  if(!state) {
    role=ConfigEntry::WatchdogReset;
  }
  if(!e->gpioString(type,role,line).isEmpty()) {
    Log(e->logDir(type,line),e->gpioString(type,role,line));
  }
  if(!e->gpioWatchdogAction(type,line).isEmpty()) {
    RunWatchdogScript(e->gpioWatchdogAction(type,line),e->logDir(type,line),
		      e->sourceNumber(),type,line,state);
  }
}


void MainObject::RunGpioScript(const QString &pgm,const QString &logname,
			       SyGpioEvent *e)
{
  /*
  printf("Run script: %s\n",(const char *)pgm.toUtf8());
  printf("   Logname: %s\n",(const char *)logname.toUtf8());
  printf("      GPIO: %s\n",(const char *)e->dump().toUtf8());
  */
  QStringList args;
  args.push_back(Config::gpioTypeText(e->type()));
  args.push_back(QString().sprintf("%d",e->sourceNumber()));
  args.push_back(QString().sprintf("%d",e->line()+1));
  args.push_back(Config::stateText(e->state()));
  args.push_back(Config::stateText(e->isPulse()));
  args.push_back(e->originAddress().toString());
  args.push_back(QString().sprintf("%d",0xFFFF&e->originPort()));

  main_script_list.push_back(new ScriptEvent(this));
  connect(main_script_list.back(),SIGNAL(finished()),
	  this,SLOT(scriptFinishedData()));
  main_script_list.back()->start(pgm,args,logname);
}


void MainObject::RunWatchdogScript(const QString &pgm,const QString &logname,
				   int srcnum,SyGpioEvent::Type type,int line,
				   bool state)
{
  QStringList args;
  args.push_back(Config::gpioTypeText(type));
  args.push_back(QString().sprintf("%d",srcnum));
  args.push_back(QString().sprintf("%d",line+1));
  args.push_back(Config::stateText(state));

  main_script_list.push_back(new ScriptEvent(this));
  connect(main_script_list.back(),SIGNAL(finished()),
	  this,SLOT(scriptFinishedData()));
  main_script_list.back()->start(pgm,args,logname);
}


void MainObject::Log(const QString &dirname,const QString &msg) const
{
  FILE *f=NULL;
  QDateTime now=QDateTime::currentDateTime();
  int tenth=lround((double)now.time().msec()/100.0);
  if(tenth==10) {
    now.setTime(now.time().addSecs(1));
    tenth=0;
  }
  QString filename=dirname+"/"+now.toString("yyyy-MM-dd")+".txt";
  
  if((f=fopen(filename.toUtf8(),"a"))==NULL) {
    syslog(LOG_WARNING,"unable to open log file \"%s\"",
	   (const char *)filename.toUtf8());
    return;
  }
  fprintf(f,"%s: %s\n",(const char *)(now.toString("hh:mm:ss")+
				      QString().sprintf(".%d",tenth)).toUtf8(),
	  (const char *)msg.toUtf8());
  fclose(f);
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();

  return a.exec();
}
