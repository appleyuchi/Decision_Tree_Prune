#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding("utf-8")
import numpy as np
import pandas as pd
from pandas import Series, DataFrame
ser = Series(np.arange(3.))
data = DataFrame(np.arange(16).reshape(4,4),index=list('abcd'),columns=list('wxyz'))
print"data=",data
print"type(data)=",type(data)
print"---------------------------"
print data['w']  #选择表格中的'w'列，使用类字典属性,返回的是Series类型
#注意,这里的data['w']中的w都是列名,如果写一个不存在的名字,那么就会有KeyError的错误
print"---------------------------"
# print data.w    #选择表格中的'w'列，使用点属性,返回的是Series类型
# print data[['w']]  #选择表格中的'w'列，返回的是DataFrame类型
# print data[['w','z']]  #选择表格中的'w'、'z'列
# print data[0:2]  #返回第1行到第2行的所有行，前闭后开，包括前不包括后

# https://blog.csdn.net/kingov/article/details/79513322