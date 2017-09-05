#include <QString>
#include <QRegExp>
#include "PinyinFormatter.h"
#include "HanyuPinyinOutputFormat.h"
#include "HanyuPinyinToneType.h"

QString PinyinFormatter::formatHanyuPinyin(QString pinyinStr, HanyuPinyinOutputFormat outputFormat)
{
    if((NULL==pinyinStr)){
        return pinyinStr;
    }

    if ((HanyuPinyinToneType::WITH_TONE_MARK == outputFormat.getToneType())&&((HanyuPinyinVCharType::WITH_V == outputFormat.getVCharType()) || (HanyuPinyinVCharType::WITH_U_AND_COLON == outputFormat.getVCharType())))
    {
       // throw new BadHanyuPinyinOutputFormatCombination("tone marks cannot be added to v or u:");
        return pinyinStr;
    }

    
    if (HanyuPinyinToneType::WITHOUT_TONE == outputFormat.getToneType())
    {
        pinyinStr = pinyinStr.replace(QRegExp("[1-5]"), "");
    }
	else if (HanyuPinyinToneType::WITH_TONE_MARK == outputFormat.getToneType())
    {
        pinyinStr = pinyinStr.replace("u:", "v");
        //pinyinStr = convertToneNumber2ToneMark(pinyinStr);
    }

    if (HanyuPinyinVCharType::WITH_V == outputFormat.getVCharType())
    {
        pinyinStr = pinyinStr.replace("u:", "v");
    }
	else if (HanyuPinyinVCharType::WITH_U_UNICODE == outputFormat.getVCharType())
    {
        pinyinStr = pinyinStr.replace("u:", "ü");
    }

    if (HanyuPinyinCaseType::UPPERCASE == outputFormat.getCaseType())
    {
        pinyinStr = pinyinStr.toUpper();
    }
    
    return pinyinStr;
}
