// scriptevent.h
//
// Execute a script
//
//   (C) Copyright 2017 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef SCRIPTEVENT_H
#define SCRIPTEVENT_H

#include <QObject>
#include <QProcess>

class ScriptEvent : public QObject
{
  Q_OBJECT;
 public:
  ScriptEvent(QObject *parent=0);
  ~ScriptEvent();
  QProcess::ProcessState processState() const;

 signals:
  void finished();

 public slots:
  void start(const QString &pgm,const QStringList &args,const QString &logfile);

 private slots:
  void processFinishedData(int exit_code,QProcess::ExitStatus status);
  void processErrorData(QProcess::ProcessError err);

 private:
  void Log(const QString &msg) const;
  QProcess *script_process;
  QString script_program;
  QString script_logfile;
};


#endif  // SCRIPTEVENT_H
