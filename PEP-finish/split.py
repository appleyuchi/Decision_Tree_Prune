#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')


import csv
from collections import Counter
import pandas as pd 
path='./abalone.data'
def read_data(path):
    source_data=pd.read_csv(path,header=None)#第一行不是标题
    lists=source_data.values.tolist()
    return lists


name_path='./abalone.names'
def get_feature(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list



signals=["<=",">","="]


def datasets_discrete_filter(number,datasets,branch):#获取符合树枝上离散特征要求的数据
    value=branch.split("=")[1]
    subdata=[]
    for item in datasets:
        if item[number]==value:
            subdata.append(item)
    return subdata



def datasets_continuous_filter(number,datasets,branch):#获取符合树枝上连续特征要求的数据
    # print "number=",number
    we_need_small="unKnown"
    value=0.0
    if "<=" in branch:
        value=float(branch.split("<=")[1])

        we_need_small="yes"
    if ">" in branch:
        value=float(branch.split(">")[1])
        we_need_small="no"

    subdata=[]
    if we_need_small=="yes":
        for item in datasets:

            if item[number]<=value:
                subdata.append(item)
    if we_need_small=="no":
        for item in datasets:
            if item[number]>value:
                subdata.append(item)
    return subdata


def splitdatasets(best_feature,fea_list,branch,datasets):
    #判断当前树枝是离散还是连续特征,如果是"="，就是离散特征，如果是不等号，就是连续特征。
    #因为"<="中也含有“＝”，为了防止误判，必须先判断"<="，ｂｒａｎｃｈ中没有"<="的情况下再判断"="
    # 所以signals列表中的元素的顺序不可更改。
    #一旦检测到包含"<="，立刻ｂｒｅａｋ终止检查，否则会被判断　包含"="　的结果所覆盖。
    for item in signals:
        signal=item
        if item  in branch:
            break

    flag=0
    if signals.index(signal)==2:#如果branch上面是"=value"的形式，就必然是离散特征
        flag=0
    else:
        flag=1#连续特征


    #判断该特征在数据集的第几列，并且根据该列特征筛选数据集，最后返回
    number=fea_list.index(best_feature)
    if flag==0:#根据离散特征来筛选
        subdata=datasets_discrete_filter(number,datasets,branch)
    if flag==1:#根据连续特征来筛选
        subdata=datasets_continuous_filter(number,datasets,branch)
        # print"subdata=",subdata
    return subdata

#－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－






    
if __name__ == '__main__':
    path='./abalone.data'
    datasets=read_data(path)

    fea_list=get_feature(name_path)
    # #['Sex', 'Length', 'Diameter', 'Height', 'Whole', 'Shucked', 'Viscera', 'Shell']

#-----------------离散特征测试－－－－－－－－－－－－－－－－
    # branch='=I'
    # result=splitdatasets("Sex",fea_list,branch,datasets)
#-----------------连续特征测试－－－－－－－－－－－－－－－－
    branch='>0.0285'
    result=splitdatasets("Viscera",fea_list,branch,datasets)
    print len(result)
#-----------------连续特征测试－－－－－－－－－－－－－－－－
    branch='<=0.0285'
    result=splitdatasets("Viscera",fea_list,branch,datasets)
    print len(result)



