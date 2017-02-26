// scriptevent.cpp
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

#include <stdio.h>

#include <QDateTime>

#include "scriptevent.h"

ScriptEvent::ScriptEvent(QObject *parent)
  : QObject(parent)
{
  script_process=new QProcess(this);
  connect(script_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(processFinishedData(int,QProcess::ExitStatus)));
  connect(script_process,SIGNAL(error(QProcess::ProcessError)),
	  this,SLOT(processErrorData(QProcess::ProcessError)));
}


ScriptEvent::~ScriptEvent()
{
  delete script_process;
}


QProcess::ProcessState ScriptEvent::processState() const
{
  return script_process->state();
}


void ScriptEvent::start(const QString &pgm,const QStringList &args,
			const QString &logfile)
{
  script_program=pgm;
  script_logfile=logfile;
  script_process->start(pgm,args);
}


void ScriptEvent::processFinishedData(int exit_code,QProcess::ExitStatus status)
{
  Log(script_process->readAllStandardOutput());

  if(status!=QProcess::NormalExit) {
    Log("script \""+script_program+"\" crashed");
  }
  else {
    if(exit_code!=0) {
      Log("script \""+script_program+"\" returned exit code "+
	  QString().sprintf("%d",exit_code));
    }
  }
  emit finished();
}


void ScriptEvent::processErrorData(QProcess::ProcessError err)
{
  Log("script \""+script_program+"\" generated process error "+
      QString().sprintf("%d",err));
}


void ScriptEvent::Log(const QString &msg) const
{
  if(!msg.trimmed().isEmpty()) {
    FILE *f=NULL;

    if((f=fopen(script_logfile.toUtf8(),"a"))!=NULL) {
      fprintf(f,"%s: ",
	      (const char *)QTime::currentTime().toString("hh:mm:ss").toUtf8());
      fprintf(f,"%s",(const char *)msg.toUtf8());
      if(msg.right(1)!="\n") {
	fprintf(f,"\n");
      }
      fclose(f);
    }
  }
}
