/*
 * Copyright 2015 handsomezhou & Li Min
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QTextStream>
#include <QStringList>
#include <QByteArray>
//#include <QDebug>
#include "ResourceHelper.h"

QHash<QString, QString> *ResourceHelper::loadProperties(QString resourcePath)
{
    QHash<QString, QString> *hash=new  QHash<QString, QString>();
    if((NULL==resourcePath)||(resourcePath.trimmed().isEmpty())){ /* nothing but whitespace */;
        return hash;
    }

    QFile *pFile=new QFile(resourcePath);
    //qDebug(" File:%s, Line:%d, Function:%s,resourcePath[%s]", __FILE__, __LINE__ , __FUNCTION__,qPrintable(resourcePath));
    if(!pFile->open(QIODevice::ReadOnly)){
        //qDebug(" File:%s, Line:%d, Function:%s, open file failed", __FILE__, __LINE__ , __FUNCTION__);
        return hash;
    }
    QByteArray qba = pFile->readAll();
    qba = qUncompress(qba);
    QString s = QString::fromLatin1(qba);
    QTextStream in(&s);
    while (!in.atEnd()) {
        QString line = in.readLine();

        QStringList stringList=line.split(QChar::Space);
        hash->insert( stringList.at(0), stringList.at(1));

        // //qDebug(" line:%s key[%s]-value[%s]",qPrintable(line),qPrintable(stringList.at(0)),qPrintable(stringList.at(1)));
    }

    //qDebug(" File:%s, Line:%d, Function:%s  hash->size()[%d]", __FILE__, __LINE__ , __FUNCTION__, hash->size());


    return hash;
}
