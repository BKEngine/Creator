#ifndef PINYINFORMATTER_H
#define PINYINFORMATTER_H
#include <QString>
#include "HanyuPinyinOutputFormat.h"

class PinyinFormatter{
public:
    static QString formatHanyuPinyin(QString pinyinStr,HanyuPinyinOutputFormat outputFormat);
private:

};
#endif // PINYINFORMATTER_H
