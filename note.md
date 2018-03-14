1. getDongleInfo
获取dongleId和dongle 时间

通过API 的read函数读取当前时间和ｄｏｎｇｌｅ时间比较
 通过API 的read函数读取dongleid和ｄｏｎｇｌｅkey比较

时间在有效时间范围内且ｉｄ相同，则表示正确．

根据正确或错误状态返回不同值，


id不匹配返回 idnotmatch
过期返回 outofdate
正常返回 完整字符串   与入的完整数据
0x0008 20190101 0x00f3 aaabbbXXXXXXXbb
