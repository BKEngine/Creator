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

#ifndef HANYUPINYINVCHARTYPE_H
#define HANYUPINYINVCHARTYPE_H
#include <QString>
#include "pinyin4cpp_global.h"

enum class HanyuPinyinVCharType{
    /**
     * The option indicates that the output of 'ü' is "u:"
     */
    WITH_U_AND_COLON,
    /**
     * The option indicates that the output of 'ü' is "v"
     */
    WITH_V,
    /**
     * The option indicates that the output of 'ü' is "ü" in Unicode form
     */
    WITH_U_UNICODE,
};

#endif // HANYUPINYINVCHARTYPE_H
