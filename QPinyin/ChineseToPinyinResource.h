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

#ifndef CHINESETOPINYINRESOURCE_H
#define CHINESETOPINYINRESOURCE_H
#include <QChar>
#include <QString> //NULL
#include <QStringList>
#include <QHashIterator>

static QString TEXT_FILE_RESOURCE_PATH = ":/pinyin4cpp/pinyin.txt.compress";
static QString LEFT_BRACKET="(";
static QString RIGHT_BRACKET=")";
static QString COMMA=",";

class ChineseToPinyinResource{
public:
    ~ChineseToPinyinResource();
    static ChineseToPinyinResource* getInstance();
    void getHanyuPinyinStringArray(QChar ch, QList<QString> *pinyinList);
    void freeResource();
private:
    static ChineseToPinyinResource *instance;
    QHash<QString, QString> *hanyuPinyinHash;
    ChineseToPinyinResource();
    void initializeResource();
    QString getHanyuPinyinRecordFromChar(QChar ch);
    bool isValidRecord(QString record);
};

#endif // CHINESETOPINYINRESOURCE_H
