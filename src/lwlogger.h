// lwlogger.h
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

#ifndef LWLOGGER_H
#define LWLOGGER_H

#include <QList>
#include <QObject>
#include <QTimer>

#include <sy/sygpio_server.h>

#include "config.h"
#include "scriptevent.h"

#define LWLOGGER_USAGE "[options]\n"

class MainObject : public QObject
{
 Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void gpioReceivedData(SyGpioEvent *e);
  void scriptFinishedData();
  void collectGarbageData();
  void watchdogStateChangedData(ConfigEntry *e,SyGpioEvent::Type type,
				int line,bool state);


 private:
  void RunGpioScript(const QString &pgm,const QString &logname,SyGpioEvent *e);
  void RunWatchdogScript(const QString &pgm,const QString &logname,
			 int srcnum,SyGpioEvent::Type type,int line,bool state);
  void Log(const QString &dirname,const QString &msg) const;
  QList<ScriptEvent *> main_script_list;
  QTimer *main_garbage_timer;
  SyGpioServer *main_gpio_server;
  Config *main_config;
};


#endif  // LWLOGGER_H
