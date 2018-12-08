#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import pandas as pd
data=pd.read_csv("crx.data",sep=',',header=None)


def unKnown_average(column):
    sums=0.0
    for item in column:
        try:
            sums+=float(item)
        except:
            pass
    print"? average=",sums/len(column)


if __name__ == '__main__':
    zero_col_count = dict(data[0].value_counts())#统计第0列元素的值的个数
    print zero_col_count
    # print max(data[3])#column D
    # print max(data[4])#column E
    # print max(data[5])#column F

