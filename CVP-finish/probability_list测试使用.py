# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-11-28 18:34:04
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-11-28 21:31:49


import pandas as pd 
import copy


def get_class(path):
    class_list=[]
    result=""
    for line in open(path):
        if "|" in line:
            result=line.split("|")[0]
        break
    return result.split(',')


def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list


def Prior_Probability_list(data,class_list):
    dicts={}
    length=len(data)*1.0
    items=[item[-1] for item in data ]
    # classes=list(set(items))
    for item in items:
        if str(item)  in class_list:
            if item not in dicts:
                dicts[item]=1
            else:
                dicts[item]=dicts[item]+1

    dicts_count=copy.deepcopy(dicts)
    list_count=[]

    for key in dicts_count:
        list_count.append(dicts_count[key])

    dicts_frequency=dicts

    for key in dicts_frequency:
        print dicts_frequency[key]/length
        dicts_frequency[key]=dicts_frequency[key]/length
    return dicts_frequency,list_count


def read_data(path):
    source_data=pd.read_csv(path,header=None)#第一行不是标题
    lists=source_data.values.tolist()
    return lists

if __name__ == '__main__':
    #---------get Attribute list--------------------------
    name_path='./abalone.names'
    feature_list=get_Attribute(name_path)
    class_list=get_class(name_path)
    #-----------get datasets------------------------
    path='./abalone_parts.data'
    datasets=read_data(path)
    Prior_Probability_lists,Nj_list=Prior_Probability_list(datasets,class_list)
    print"Prior_Ｐrobability_lists=",Prior_Probability_lists
    print"Nj_list=",Nj_list
