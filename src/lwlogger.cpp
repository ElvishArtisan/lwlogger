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

#include "lwlogger.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  main_config=new Config();
  main_config->load();

  main_gpio_server=new SyGpioServer(new SyRouting(0,0),this);
  connect(main_gpio_server,SIGNAL(gpioReceived(SyGpioEvent *)),
	  this,SLOT(gpioReceivedData(SyGpioEvent *)));
}


void MainObject::gpioReceivedData(SyGpioEvent *e)
{
  if(main_config->logActive(e)) {
    Log(main_config->logDir(e),main_config->logLine(e));
  }
}


void MainObject::Log(const QString &dirname,const QString &msg) const
{
  FILE *f=NULL;
  QDateTime now=QDateTime::currentDateTime();
  QString filename=dirname+"/"+now.toString("yyyy-MM-dd")+".txt";

  if((f=fopen(filename.toUtf8(),"a"))==NULL) {
    syslog(LOG_WARNING,"unable to open log file \"%s\"",
	   (const char *)filename.toUtf8());
    return;
  }
  fprintf(f,"%s: %s\n",(const char *)now.toString("hh:mm:ss").toUtf8(),
	  (const char *)msg.toUtf8());
  fclose(f);
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();

  return a.exec();
}
