/*
 * Copyright (c) 2002-2006 by OpenMFG, LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * If you do not wish to be bound by the terms of the GNU General Public
 * License, DO NOT USE THIS SOFTWARE.  Please contact OpenMFG, LLC at
 * info@openmfg.com for details on how to purchase a commercial license.
 */

#include "xvariant.h"

#include <qvariant.h>
#include <qbitarray.h>
#include <qvaluelist.h>

static const QChar amper = '&';
static const QChar quote = '"';
static const QChar comma = ',';
static const QChar colon = ':';
static const QChar equal = '=';

QStringList decomposeList(const QString &);

QString encodeString(const QString &);
QString decodeString(const QString &);

QString bitArrayToString(const QBitArray &);
QBitArray stringToBitArray(const QString &);

QString listToString(const QValueList<QVariant> &);
QValueList<QVariant> stringToList(const QString &);

QString stringListToString(const QStringList &);
QStringList stringToStringList(const QString &);

QString mapToString(const QMap<QString, QVariant> &);
QMap<QString, QVariant> stringToMap(const QString &);

XVariant::XVariant() {}
XVariant::~XVariant() {}

QString XVariant::encode(const QVariant & v)
{
  QString str = QString::null;
  switch(v.type())
  {
    case QVariant::BitArray:
      return bitArrayToString(v.toBitArray());

    case QVariant::List:
      return listToString(v.toList());

    case QVariant::StringList:
      return stringListToString(v.toStringList());

    case QVariant::Map:
      return mapToString(v.toMap());

    // unsupported types
    case QVariant::Bitmap:
    case QVariant::Brush:
    case QVariant::ColorGroup:
    case QVariant::Cursor:
    case QVariant::IconSet:
    case QVariant::Image:
    case QVariant::Palette:
    case QVariant::Pen:
    case QVariant::Pixmap:
    case QVariant::Point:
    case QVariant::PointArray:
    case QVariant::Rect:
    case QVariant::Region:
    case QVariant::Size:
    case QVariant::SizePolicy:
      return QString::null;

    // handled by the fallthrough
    case QVariant::Invalid:
    case QVariant::String:
    case QVariant::Font:
    case QVariant::Color:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Bool:
    case QVariant::Double:
    case QVariant::CString:
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
    case QVariant::ByteArray:
    case QVariant::KeySequence:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    default:
      break;
  }
  return v.toString();
}

QVariant XVariant::decode(const QString & type, const QString & data)
{
  QVariant v(data);
  // handle a case where the type was not taken into consideration
  // in this case we will just treat the data a string
  if(type.isEmpty())
    return v;

  QVariant::Type t = QVariant::nameToType(type);
  switch(t)
  {
    case QVariant::ByteArray:
    case QVariant::CString:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Bool:
    case QVariant::Double:
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
    case QVariant::KeySequence:
    case QVariant::Font:
    case QVariant::Color:
    case QVariant::LongLong:
    case QVariant::ULongLong:
      v.cast(t);
      break;

    case QVariant::BitArray:
      v = QVariant(stringToBitArray(data));
      break;

    case QVariant::List:
      v = QVariant(stringToList(data));
      break;

    case QVariant::Map:
      v = QVariant(stringToMap(data));
      break;

    case QVariant::StringList:
      v = QVariant(stringToStringList(data));
      break;

    // unsupported types
    case QVariant::Bitmap:
    case QVariant::Brush:
    case QVariant::ColorGroup:
    case QVariant::Cursor:
    case QVariant::IconSet:
    case QVariant::Image:
    case QVariant::Palette:
    case QVariant::Pen:
    case QVariant::Pixmap:
    case QVariant::Point:
    case QVariant::PointArray:
    case QVariant::Rect:
    case QVariant::Region:
    case QVariant::Size:
    case QVariant::SizePolicy:
      v = QVariant();
      break;

    // already handled -- variant is already
    // in correct state for these types
    case QVariant::Invalid:
    case QVariant::String:
    default:
      break;
  }

  return v;
}

QString bitArrayToString(const QBitArray & ba)
{
  QString str;
  for(uint pos = 0; pos < ba.size(); pos++)
  {
    if(ba.at(pos) == 1)
      str.append("1");
    else
      str.append("0");
  }
  return str;
}

QBitArray stringToBitArray(const QString & str)
{
  uint size = str.length();

  QBitArray ba(size);
  ba.fill(0);

  for(uint pos = 0; pos < size; pos++)
  {
    if(str.at(pos) == '1')
      ba.setBit(pos);
  }

  return ba;
}


QString listToString(const QValueList<QVariant> & list)
{
  QString str;
  for(QValueList<QVariant>::ConstIterator it = list.begin(); it != list.end(); ++it)
  {
    if(!str.isEmpty())
      str.append(comma);

    str.append(quote);
    str.append(encodeString(QVariant::typeToName((*it).type())));
    str.append(quote);
    str.append(colon);
    str.append(quote);
    str.append(encodeString(XVariant::encode(*it)));
    str.append(quote);
  }
  return str;
}

QValueList<QVariant> stringToList(const QString & str)
{
  QStringList tList = decomposeList(str);
  QValueList<QVariant> list;

  QString s;
  QString type;
  QString value;

  int pos;

  for(QStringList::ConstIterator it = tList.begin(); it != tList.end(); ++it)
  {
    s = *it;

    pos = s.find(colon);
    type = s.left(pos).stripWhiteSpace();
    value = s.mid(pos+1).stripWhiteSpace();
    
    if(quote == type.constref(0) && quote == type.constref(type.length() - 1))
      type = decodeString(type.mid(1, type.length() - 2));

    if(quote == value.constref(0) && quote == value.constref(value.length() - 1))
      value = decodeString(value.mid(1, value.length() - 2));

    list.append(XVariant::decode(type, value));
  }

  return list;
}

QString stringListToString(const QStringList & list)
{
  QString str;
  for(QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
  {
    if(!str.isEmpty())
      str.append(comma);

    str.append(quote);
    str.append(encodeString(*it));
    str.append(quote);
  }
  return str;
}

QStringList stringToStringList(const QString & str)
{
  QStringList tList = decomposeList(str);
  QStringList list;

  QString s;

  for(QStringList::ConstIterator it = tList.begin(); it != tList.end(); ++it)
  {
    s = *it;
    if(quote == s.constref(0) && quote == s.constref(s.length() - 1))
      s = decodeString(s.mid(1, s.length() - 2));
    list.append(s);
  }

  return list;
}

QString mapToString(const QMap<QString, QVariant> &)
{
  // TODO

  return QString::null;
}

QMap<QString, QVariant> stringToMap(const QString &)
{
  QMap<QString, QVariant> map;

  // TODO

  return map;
}


QStringList decomposeList(const QString & str)
{
  bool in_string = false;
  int start = 0;
  int pos = 0;

  QStringList list;
  QString s;
  QChar c;
  
  for(pos = 0; pos < (int)str.length(); pos++)
  {
    c = str[pos];
    if(in_string)
    {
      if(quote == c)
        in_string = false;
    }
    else
    {
      if(quote == c)
        in_string = true;
      else if(comma == c)
      {
        s = str.mid(start, pos - start);
        list.append(s.stripWhiteSpace());
        start = pos + 1;
      }
    }
  }
  if(start < (int)str.length())
  {
    s = str.mid(start, (int)str.length() - start);
    list.append(s.stripWhiteSpace());
  }

  return list;
}

QString encodeString(const QString & data)
{
  QString str = data;

  str = str.replace(amper, "&amp;");
  str = str.replace(quote, "&quot;");

  return str;
}

QString decodeString(const QString & data)
{
  QString str = data;

  str = str.replace("&quot;", quote);
  str = str.replace("&amp;",  amper);

  return str;
}

