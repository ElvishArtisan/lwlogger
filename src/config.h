// config.h
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

#ifndef CONFIG_H
#define CONFIG_H

#include <map>

#include <QObject>
#include <QTimer>
#include <QString>

#include <sy/sygpio_server.h>

#define CONFIG_FILE "/etc/lwlogger.conf"

class ConfigEntry
{
 public:
  enum Role {Off=0,On=1,Pulse=2,WatchdogSet=3,WatchdogReset=4,LastRole=5};
  ConfigEntry(int srcnum);
  int sourceNumber() const;
  QString logDir(SyGpioEvent::Type type,int line) const;
  void setLogDir(SyGpioEvent::Type type,int line,const QString &str);
  QString gpioString(SyGpioEvent::Type type,Role role,int line);
  void setGpioString(SyGpioEvent::Type type,Role role,int line,
		     const QString &str);
  QString gpioAction(SyGpioEvent::Type type,int line) const;
  void setGpioAction(SyGpioEvent::Type type,int line,const QString &filename);
  int gpioWatchdogTimeout(SyGpioEvent::Type type,int line) const;
  void setGpioWatchdogTimeout(SyGpioEvent::Type type,int line,int msec);
  QString gpioWatchdogAction(SyGpioEvent::Type type,int line) const;
  void setGpioWatchdogAction(SyGpioEvent::Type type,int line,
			     const QString &filename);
  bool gpioWatchdogState(SyGpioEvent::Type type,int line) const;
  void setGpioWatchdogState(SyGpioEvent::Type type,int line,bool state);
  QDateTime timestamp(SyGpioEvent::Type type,int line);
  void touchTimestamp(SyGpioEvent::Type type,int line);

 private:
  int entry_source_number;
  QString entry_log_dirs[2][SWITCHYARD_GPIO_BUNDLE_SIZE];
  QString entry_strings[2][ConfigEntry::LastRole][SWITCHYARD_GPIO_BUNDLE_SIZE];
  QString entry_actions[2][SWITCHYARD_GPIO_BUNDLE_SIZE];
  int entry_watchdog_timeouts[2][SWITCHYARD_GPIO_BUNDLE_SIZE];
  QString entry_watchdog_actions[2][SWITCHYARD_GPIO_BUNDLE_SIZE];
  bool entry_watchdog_states[2][SWITCHYARD_GPIO_BUNDLE_SIZE];
  QDateTime entry_timestamps[2][SWITCHYARD_GPIO_BUNDLE_SIZE];
};




class Config : public QObject
{
  Q_OBJECT;
 public:
  Config(QObject *parent=0);
  ConfigEntry *configEntry(unsigned srcnum) const;
  bool logActive(SyGpioEvent *e);
  QString logDir(SyGpioEvent *e);
  QString logFilename(SyGpioEvent *e);
  QString logLine(SyGpioEvent *e);
  bool scriptActive(SyGpioEvent *e);
  QString gpioAction(SyGpioEvent *e);
  void touchTimestamp(SyGpioEvent *e);
  bool load();
  static QString gpioTypeText(SyGpioEvent::Type type);
  static QString stateText(bool state);

 signals:
  void watchdogStateChanged(ConfigEntry *e,SyGpioEvent::Type type,int line,
			    bool state);

 private slots:
  void checkWatchdogsData();

 private:
  QString TypeString(SyGpioEvent::Type) const;
  QTimer *conf_watchdog_timer;
  ConfigEntry::Role GetRole(SyGpioEvent *e) const;
  std::map<unsigned,ConfigEntry *> conf_entries;
};


#endif  // LWLOGGER_H
