# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-10-09 17:57:19
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-10-09 18:05:16

import numpy as np 
import pandas as pd 
import seaborn as sns 
titanic = sns.load_dataset('titanic') 
titanic.head() 

# print titanic.head() 
# print"-----------------------------------"
# print titanic.groupby('sex')[['survived']].mean() 
# print"-----------------------------------"
print titanic.pivot_table('survived', index='sex', columns='pclass')
#透视表的意思是:同时具备两种属性与类别标签之间的关系的统计值 


# 参考链接:
# https://www.douban.com/group/topic/97652045/




