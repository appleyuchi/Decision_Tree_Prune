# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-11-02 14:35:38
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-11-02 14:48:19

# Reference:
# https://www.jb51.net/article/144122.htm

def getNumofCommonSubstr(str1, str2):
    lstr1 = len(str1)
    lstr2 = len(str2)
    record = [[0 for i in range(lstr2+1)] for j in range(lstr1+1)] # 多一位
    maxNum = 0   # 最长匹配长度
    p = 0    # 匹配的起始位
    for i in range(lstr1):
     for j in range(lstr2):
      if str1[i] == str2[j]:
       # 相同则累加
       record[i+1][j+1] = record[i][j] + 1
       if record[i+1][j+1] > maxNum:
        # 获取最大匹配长度
        maxNum = record[i+1][j+1]
        # 记录最大匹配长度的终止位置
        p = i + 1
    return str1[p-maxNum:p], maxNum

if __name__ == '__main__':
    str1=">=228"
    str2="<2398492343228"
    res = getNumofCommonSubstr(str1, str2)
    print res[0]#str类型