# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-11-29 18:16:10
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-11-30 18:15:46

import numpy as np
import pandas as pd
from scipy.stats import chi2_contingency
import copy
from getNumofCommonSubstr import getNumofCommonSubstr
def read_data(path):
    source_data=pd.read_csv(path,header=None)#第一行不是标题
    lists=source_data.values.tolist()
    return lists

def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list

def chi_square_discrete(bestfeature,sub_datas,feature_list):
    # print"bestfeature=",bestfeature
    data=copy.deepcopy(sub_datas)
    feature=bestfeature
    feature_index=feature_list.index(feature)
    # Contingency table.
    feature=bestfeature
    feature_list.append('class')
    data=pd.DataFrame(data, columns=feature_list)
    contingency = pd.crosstab(data[feature], data["class"])
    chi2, p_value, Degree_Freedom, expected_Xij = chi2_contingency(contingency)
    # print"DF=",Degree_Freedom
    # print"当期数据的长度=",len(data)
    return chi2


def chi_square_continuous(bestfeature,branch_names,sub_datas,feature_list):
    data=copy.deepcopy(sub_datas)
    # print"feature_list=",feature_list
    feature_index=feature_list.index(bestfeature)
    feature_list.append('class')

    feature=bestfeature
    #根据《概率论与数理统计》中的双因素检验可知，需要把连续特征转化为有限个“取值水平”
    threshold=getNumofCommonSubstr(branch_names[0], branch_names[1])[0]
    threshold=float(threshold)
    # print "feature_index=",feature_index
    for index,item in enumerate(data):
        try:
            if float(item[feature_index])<=threshold:
                data[index][feature_index]="<="+str(threshold)#这里的用意是把连续特征改为因素的不同水平，便于计算自由度，由于C4.5对连续特征的处理是“二分处理”，所以这里改成两个水平，"≥"和“小于”
            else:
                data[index][feature_index]=">"+str(threshold)
        except:
            pass


    # Contingency table.
    feature=bestfeature
    datas = pd.DataFrame(data, columns=feature_list)
    contingency = pd.crosstab(datas[feature], datas["class"])
    #这个应该是指定特征和最后一列类别列之间的卡方检验计算


    # Chi-square test of independence.
    chi2, p_value, Degree_Freedom, expected_Xij = chi2_contingency(contingency)
    # print "Degree_Freedom=",Degree_Freedom
    # print"当期数据的长度=",len(datas)
    return chi2
if __name__ == '__main__':
#--------------------测试-计算离散属性的卡方值------------------------------------
    path='./abalone_parts.data'
    datasets=read_data(path)
    bestfeature='Sex'

    path="./abalone.names"
    feature_lists=get_Attribute(path)
    print feature_lists
    print bestfeature
    chi2=chi_square_discrete(bestfeature,copy.deepcopy(datasets),feature_lists)
    print"chi2=",chi2

#--------------------测试-计算连续属性的卡方值------------------------------------
    # path='./abalone_parts.data'
    # datasets=read_data(path)
    # bestfeature='Viscera'
    # path="./abalone.names"
    # feature_lists=get_Attribute(path)
    # branch_names=["＜=0.0145",">0.0145"]
    # chi2=chi_square_continuous(bestfeature,branch_names,copy.deepcopy(datasets),feature_lists)
    # print"chi2=",chi2

