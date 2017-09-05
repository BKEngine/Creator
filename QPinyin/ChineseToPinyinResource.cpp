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

//#include <QDebug>
#include "ChineseToPinyinResource.h"
#include "ResourceHelper.h"



ChineseToPinyinResource* ChineseToPinyinResource::instance=NULL;

/**
* Private constructor as part of the singleton pattern.
*/
ChineseToPinyinResource::ChineseToPinyinResource()
{
    //private constructor
    this->initializeResource();
}

ChineseToPinyinResource::~ChineseToPinyinResource()
{
    if(NULL!=ChineseToPinyinResource::instance){
        delete ChineseToPinyinResource::instance;
    }
}

void ChineseToPinyinResource::initializeResource()
{
    this->hanyuPinyinHash=ResourceHelper::loadProperties(TEXT_FILE_RESOURCE_PATH);
}


ChineseToPinyinResource* ChineseToPinyinResource::getInstance()
{
    if(NULL==ChineseToPinyinResource::instance){
        ChineseToPinyinResource::instance=new ChineseToPinyinResource();
    }

    return ChineseToPinyinResource::instance;
}

void ChineseToPinyinResource::getHanyuPinyinStringArray(QChar ch, QList<QString> *pinyinList)
{
    if(NULL==pinyinList){
        return;
    }
    QString pinyinRecord=this->getHanyuPinyinRecordFromChar(ch);
    ////qDebug()<<pinyinRecord;
    if(NULL==pinyinRecord){
        return;
    }

    int indexOfLeftBracket=pinyinRecord.indexOf(LEFT_BRACKET);
    int indexOfRightBracket = pinyinRecord.indexOf(RIGHT_BRACKET);
    QString stripedString=pinyinRecord.mid(indexOfLeftBracket+LEFT_BRACKET.length(),indexOfRightBracket-(indexOfLeftBracket+LEFT_BRACKET.length()));
   // //qDebug()<<stripedString;
    QStringList pinyinStringList=stripedString.split(COMMA);

    for(int i=0; i<pinyinStringList.length();i++){
        pinyinList->insert(i,pinyinStringList.at(i));
       // //qDebug()<<pinyinList->at(i);
    }

   // //qDebug()<<"yes";
    return;
}

QString ChineseToPinyinResource::getHanyuPinyinRecordFromChar(QChar ch)
{
    // convert Chinese character to code point (integer)
    // please refer to http://www.unicode.org/glossary/#code_point
    // Another reference: http://en.wikipedia.org/wiki/Unicode

    //qDebug()<<ch.unicode();
    QString codePointHexStr;
    QString codePointHexUpperStr=codePointHexStr.setNum(ch.unicode(),16).toUpper();
    QString foundRecord=this->hanyuPinyinHash->value(codePointHexUpperStr,NULL);
    //qDebug()<<codePointHexUpperStr;
    //qDebug()<<foundRecord;

    return (this->isValidRecord(foundRecord))?(foundRecord):(NULL);
}

/**
 * @brief ChineseToPinyinResource::isValidRecord
 * @param record    given record string of Hanyu Pinyin
 * @return      return true if record is not NULL and record is not "none0" and
 *         record is not mal-formatted, else return false
 */
bool ChineseToPinyinResource::isValidRecord(QString record)
{
    bool validRecord=false;
    const QString noneStr = "(none0)";
    do{
        if(NULL==record){
            break;
        }

        if(QString::compare(record,noneStr)==0){
            break;
        }

        if(record.startsWith(LEFT_BRACKET)&&record.endsWith(RIGHT_BRACKET)){
            validRecord=true;
            break;
        }

    }while(false);

    return validRecord;
}

void ChineseToPinyinResource::freeResource()
{

}


