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

#ifndef HANYUPINYINOUTPUTFORMAT_H
#define HANYUPINYINOUTPUTFORMAT_H
#include "HanyuPinyinVCharType.h"
#include "HanyuPinyinCaseType.h"
#include "HanyuPinyinToneType.h"
#include "pinyin4cpp_global.h"

class PINYIN4CPPSHARED_EXPORT HanyuPinyinOutputFormat{
public:
    HanyuPinyinOutputFormat();
    void restoreDefault();


    HanyuPinyinVCharType getVCharType() const;
    void setVCharType(HanyuPinyinVCharType vCharType);

    HanyuPinyinCaseType getCaseType() const;
    void setCaseType(HanyuPinyinCaseType caseType);

    HanyuPinyinToneType getToneType() const;
    void setToneType(HanyuPinyinToneType toneType);

private:
    HanyuPinyinVCharType vCharType;
    HanyuPinyinCaseType caseType;
    HanyuPinyinToneType toneType;

};

#endif // HANYUPINYINOUTPUTFORMAT_H
