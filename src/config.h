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

#include <QString>

#include <sy/sygpio_server.h>

#define CONFIG_FILE "/etc/lwlogger.conf"

class ConfigEntry
{
 public:
  enum Role {Off=0,On=1,Pulse=2,LastRole=3};
  ConfigEntry(int srcnum);
  int sourceNumber() const;
  QString logDir(SyGpioEvent::Type type,int line) const;
  void setLogDir(SyGpioEvent::Type type,int line,const QString &str);
  QString gpioString(SyGpioEvent::Type type,Role role,int line);
  void setGpioString(SyGpioEvent::Type type,Role role,int line,
		     const QString &str);

 private:
  int entry_source_number;
  QString entry_log_dirs[2][SWITCHYARD_GPIO_BUNDLE_SIZE];
  QString entry_strings[2][ConfigEntry::LastRole][SWITCHYARD_GPIO_BUNDLE_SIZE];
};




class Config
{
 public:
  Config(QObject *parent=0);
  bool logActive(SyGpioEvent *e);
  QString logDir(SyGpioEvent *e);
  QString logLine(SyGpioEvent *e);
  bool load();

 private:
  ConfigEntry *GetConfigEntry(unsigned srcnum) const;
  ConfigEntry::Role GetRole(SyGpioEvent *e) const;
  std::map<unsigned,ConfigEntry *> conf_entries;
};


#endif  // LWLOGGER_H
