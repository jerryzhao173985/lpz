/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Dominic Schneider (original author)                                  *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/

#include "inifile.h"
#include <iostream>
using namespace std;
#include <QRegularExpression>


IniFile::IniFile(){
  // Qt5: No more setAutoDelete, we'll manage memory manually
  bloaded=false;
}

IniFile::IniFile(const QString& _filename){
  // Qt5: No more setAutoDelete, we'll manage memory manually
  bloaded=false;
  setFilename(_filename);
}


IniFile::~IniFile(){
  // Qt5: Manually delete all sections
  qDeleteAll(sections);
  sections.clear();
}

void IniFile::setFilename(const QString& _filename){
  filename=_filename;
}

QString IniFile::getFilename() const{
  return filename;
}


bool IniFile::Load(){
  sections.clear();
  char buffer[1024];
  QString line;
  IniSection* momsection;
  IniVar* momvar;
  bool beforeFirstSection=true;
  int LineType;
  QString str1,str2,str3;

  file.setFileName(filename);
  if (!file.exists()){
#ifdef DEBUG
    cerr << "\nFile doesn't exist: " << filename.toLatin1().data();
#else

#endif
    return false;
  }
  if (!file.open( QIODevice::ReadOnly)) {
#ifdef DEBUG
    cerr << "\nCannot read File: " << filename.toLatin1().data();
#else

#endif
    return false;
  }
  // Zeile f�r Zeile auslesen und zuordnen
  while( (file.readLine(buffer,1024))>0 ){
    line=QString(buffer);
    str1="";
    str2="";
    str3="";
    LineType=getLineType(line,str1,str2,str3);
    switch (LineType){
      case EMPTY:
      break;
      case COMMENT:
        if (beforeFirstSection){
          comment.append(str1);
        }else{
          momsection->addComment(str1);
        }
      break;
      case SECTION:
        beforeFirstSection=false;
        momsection=new IniSection(str1);
        sections.append(momsection);
      break;
      case VAR:
        if (beforeFirstSection){ // wird als Kommentar gewertet
          comment.append(str1);
        }else{                    // Normale Variable
          if (momsection) {  // Ensure we have a valid section
            momvar=new IniVar(str1,str2,str3);
            momsection->vars.append(momvar);
          }
        }
      break;
    }
  }

  file.close();

  bloaded=true;
  return true;
}


int IniFile::getLineType( QString _line, QString &str1, QString &str2, QString &str3){
  QRegularExpression regexp;
  int start,len;

  if (_line.isEmpty()|| _line=="\n") return EMPTY;

  if (_line.indexOf(QRegularExpression("^[;#]"),0)!=-1){
    str1=_line;
    return COMMENT;
  }

  regexp.setPattern("^\\[.+\\]");
  QRegularExpressionMatch match = regexp.match(_line);
  if (match.hasMatch()){
    start = match.capturedStart();
    len = match.capturedLength();
    str1=_line.mid(start+1,len-2);
    return SECTION;
  }

  regexp.setPattern(".+=.+");
  match = regexp.match(_line);
  if (match.hasMatch()){
    regexp.setPattern(".+=");
    match = regexp.match(_line);
    if (match.hasMatch()) {
      len = match.capturedLength();
      str1=_line.left(len-1);
      int start2=len;

      regexp.setPattern(".+[;#]");
      match = regexp.match(_line.mid(start2));
      if (match.hasMatch()){ // is there a comment at the end of the line?
        len = match.capturedLength();
        str2=_line.mid(start2,len-1);
        str3=_line.mid(start2+len-1);
      }else{  // no comment
        str2=_line.mid(start2, _line.length()-start2-1);
      }
    }
    return VAR;
  }

  // kann sich nur um eine alleinstehende Variable handeln
  str1=_line;
  
  return VAR;
}


bool IniFile::Save(){
  if (filename.isEmpty()) return false;
  file.setFileName(filename);
  if (! file.open(QIODevice::WriteOnly)){
#ifdef DEBUG
    cerr << "\nCannot write File: " << filename.toLatin1().constData();
#else

#endif
    return false;
  }

  IniSection* section;
  IniVar* var;
  QString line;

  // Durchgehen und alles reinschreiben
  file.write(comment.toLatin1());
  foreach(section, sections){
    line="\n[";
    line.append(section->getName());
    line.append("]\n");
    file.write(line.toLatin1());
    line=section->getComment();
    if (!line.isEmpty()) {
      line += "\n";
      file.write(line.toLatin1());
    }
    foreach(var, section->vars){
      line=var->getName();
      if (! var->getValue().isEmpty()){
        line.append("=");
        line.append(var->getValue());
        line.append(var->getComment());
//      }else{
        line.append("\n");
      }
      file.write(line.toLatin1());
    }
  }

  file.close();

  return true;
}

void IniFile::Clear(){
  // Qt5: Manually delete all sections before clearing
  qDeleteAll(sections);
  sections.clear();
  filename="";
  bloaded=false;
  comment="";
}



bool IniFile::getSection(IniSection& _section,QString _name,bool _next){
  static QString lastname;
  static int lastindex = 0;
  
  if (_next==false || (_next==true && _name!=lastname) ) {
    lastname ="";
    lastindex = 0;
  }else{
    ++lastindex;
  }

  lastname=_name;

  for(int i = lastindex; i < sections.size(); ++i){
      IniSection* sec = sections[i];
      if (sec->getName()==_name){  // gefunden
        sec->copy(_section);
        lastindex = i;
        return true;
      }
  }
  return false;
}


IniSection *IniFile::addSection(QString name)
{   IniSection* sec = new IniSection(name);
    sections.append(sec);
    return sec;
}


void IniFile::delSection(IniSection* _section)
{   sections.removeAll(_section);
    delete _section;
    // _section = nullptr; // Parameter assignment has no effect
}


QString IniFile::getValueDef(QString _section, QString _var, QString _default){
  IniSection sec;
  if(getSection(sec,_section,false)){
    IniVar var;
    if(sec.getVar(var,_var)){
      return var.getValue();
    } else 
      return _default;
  } else 
    return _default;

}

void IniFile::setComment(const QString& _comment){
  comment=_comment;
}

void IniFile::addComment(const QString& _comment){
  comment.append(_comment);
}

QString IniFile::getComment() const{
  return comment;
}


//////////////////////////////////////////////////////////////////////////////////////////(
//SECTION

IniSection::IniSection(){
  // Qt5: No more setAutoDelete, we'll manage memory manually
}

IniSection::IniSection(const QString& _name){
  // Qt5: No more setAutoDelete, we'll manage memory manually
  setName(_name);
}

IniSection::~IniSection(){
  // Qt5: Manually delete all vars
  qDeleteAll(vars);
  vars.clear();
}

void IniSection::setName(const QString& _name){
  name=_name;
}

QString IniSection::getName() const{
  return name;
}

void IniSection::setComment(const QString& _comment){
  comment=_comment;
}

void IniSection::addComment(const QString& _addcomment){
  comment.append(_addcomment);
}

QString IniSection::getComment() const{
  return comment;
}

bool IniSection::operator== (IniSection& _section){
  return name==_section.getName();
}


void IniSection::copy (IniSection& _section){
  _section.setName(name);
  _section.setComment(comment);
  
  // Deep copy: clear existing vars and create new IniVar objects
  qDeleteAll(_section.vars);
  _section.vars.clear();
  
  foreach(IniVar* var, vars) {
    if (var) {  // Check for null pointer
      IniVar* newVar = new IniVar();
      var->copy(*newVar);
      _section.vars.append(newVar);
    }
  }
}


bool IniSection::getVar( IniVar& _var, QString _name){
  foreach(IniVar* tempvar, vars){
    if (tempvar && tempvar->getName()==_name){  // Add null check
      tempvar->copy(_var);
      return true;
    }
  }
  return false;
}


void IniSection::delVar(IniVar* _var)
{    vars.removeAll(_var);
     delete _var;
     // _var = nullptr; // Parameter assignment has no effect
}


void IniSection::addValue(QString name, QString value,QString comment)
{   IniVar* tmpvar = new IniVar(name, value, comment);
    vars.append(tmpvar);
}


/////////////////////////////////////////////////////////////////////////////////////////
// VAR
IniVar::IniVar(){
}

IniVar::IniVar( QString _name, QString _value, QString _comment){
  setName(_name);
  setValue(_value);
  setComment(_comment);
}

IniVar::~IniVar(){
}

void IniVar::setName(const QString& _name){
  name=_name;
}

QString IniVar::getName() const{
  return name;
}

void IniVar::setValue(const QString& _value){
  value=_value;
}

QString IniVar::getValue() const{
  return value;
}

void IniVar::setComment(const QString& _comment){
  comment=_comment;
}

QString IniVar::getComment() const{
  return comment;
}

bool IniVar::operator== (IniVar& _var){
  return name==_var.getName();
}

void IniVar::copy (IniVar& _var){
  _var.setName(name);
  _var.setComment(comment);
  _var.setValue(value);
}
