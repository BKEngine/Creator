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

#ifndef PINYINHELPER_H
#define PINYINHELPER_H
#include <QChar>
#include <QList>
#include "HanyuPinyinOutputFormat.h"
#include "pinyin4cpp_global.h"

class PINYIN4CPPSHARED_EXPORT PinyinHelper{
public:
    static void toHanyuPinyinStringArray(QChar ch, HanyuPinyinOutputFormat outputFormat, QList<QString> *pinyinList);
private:
    PinyinHelper();
    static void getFormattedHanyuPinyinStringArray(QChar ch, HanyuPinyinOutputFormat outputFormat, QList<QString> *pinyinList);
    static void getUnformattedHanyuPinyinStringArray(QChar ch, QList<QString> *pinyinList);

};

#endif // PINYINHELPER_H
