#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import numpy as np
from sklearn.model_selection import train_test_split
import csv

# 注意，使用时，默认最后一列是类别标签
# 并且假定第0行开始就是数据集
def read_data_for_split(test_data='abalone.data',n=0,label=1):
    '''
    加载数据的功能
    n:特征数据起始位
    label：是否是监督样本数据
    '''
    csv_reader=csv.reader(open(test_data))
    data_list=[]
    for one_line in csv_reader:
        data_list.append(one_line)
    x_list=[]
    y_list=[]
    for one_line in data_list[n:]:
        if label==1:#有监督数据
            y_list.append(int(one_line[-1]))   #标志位
            one_list=[o for o in one_line[n:-1]]
            x_list.append(one_list)
        else:#无监督数据
            one_list=[float(o) for o in one_line[n:]]
            x_list.append(one_list)

    return x_list, y_list

def split_data(data_list, y_list, ratio=0.30):
    '''
    按照指定的比例，划分样本数据集
    ratio: 测试数据的比率
    '''
    X_train, X_test, y_train, y_test = train_test_split(data_list, y_list, test_size=ratio, random_state=50)
    print '--------------------------------split_data shape-----------------------------------'
    print len(X_train), len(y_train)
    print len(X_test), len(y_test)
    return X_train, X_test, y_train, y_test


if __name__ == '__main__':
    x_list, y_list=read_data_for_split(test_data='abalone.data',n=0,label=1)
    X_train,X_test,y_train,y_test=split_data(x_list,y_list)
    print"X_train=",X_train
    print"*****************************************************************"
    print"X_test=",X_test
    print"*****************************************************************"
    print"y_train=",y_train
    print"*****************************************************************"
    print"y_test=",y_test

