#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import pandas as pd



def unKnown_average(column):
    sums=0.0
    for item in column:
        try:
            sums+=float(item)
        except:
            pass
    print"? average=",sums/len(column)


if __name__ == '__main__':
    data=pd.read_csv("crx.data",sep=',',header=None)
    zero_col_count = dict(data[0].value_counts())
    print"\n"
    print "Column A=",zero_col_count

    zero_col_count = dict(data[3].value_counts())
    print"\n"
    print "Column D=",zero_col_count

    zero_col_count = dict(data[4].value_counts())
    print"\n"
    print "Column E=",zero_col_count

    zero_col_count = dict(data[5].value_counts())
    print"\n"
    print "Column F=",zero_col_count

    zero_col_count = dict(data[6].value_counts())
    print"\n"
    print "Column G=",zero_col_count

    zero_col_count = dict(data[8].value_counts())
    print"\n"
    print "Column I=",zero_col_count

    zero_col_count = dict(data[9].value_counts())
    print"\n"
    print "Column J=",zero_col_count



    unKnown_average(data[1])#column B
    unKnown_average(data[2])#column C

    unKnown_average(data[13])#column N



