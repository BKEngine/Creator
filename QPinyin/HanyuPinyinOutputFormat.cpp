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

#include "HanyuPinyinOutputFormat.h"

HanyuPinyinOutputFormat::HanyuPinyinOutputFormat()//:vCharType(),caseType(),toneType()
{
    this->restoreDefault();
}

void HanyuPinyinOutputFormat::restoreDefault()
{

    this->setVCharType(HanyuPinyinVCharType::WITH_V);

    this->setCaseType(HanyuPinyinCaseType::LOWERCASE);

    this->setToneType(HanyuPinyinToneType::WITHOUT_TONE);
}

HanyuPinyinVCharType HanyuPinyinOutputFormat::getVCharType() const
{
    return this->vCharType;
}

void HanyuPinyinOutputFormat::setVCharType(HanyuPinyinVCharType vCharType)
{
    this->vCharType = vCharType;
}
HanyuPinyinCaseType HanyuPinyinOutputFormat::getCaseType() const
{
    return this->caseType;
}

void HanyuPinyinOutputFormat::setCaseType(HanyuPinyinCaseType caseType)
{
    this->caseType = caseType;
}

HanyuPinyinToneType HanyuPinyinOutputFormat::getToneType() const
{
    return this->toneType;
}

void HanyuPinyinOutputFormat::setToneType(HanyuPinyinToneType toneType)
{
    this->toneType = toneType;
}




