# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-11-23 20:16:45
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-11-30 22:17:38

#C4.5的叶子格式
# ｌｅａｆ(total number/error number)
#model中只使用＜=,>,=这三种符号，不使用≤，≥，＞=这三种符号

import numpy as np
import copy
import scipy.stats
import csv
import pandas as pd 
from scipy import stats
from collections import Counter
from split import splitdatasets
from getNumofCommonSubstr import getNumofCommonSubstr
from contingency_table import chi_square_discrete,chi_square_continuous
from treePlotter import createPlot
from predict import accuracy_analysis,classify_C45

alpha=0.05

def conti_or_discrete_list(path):
    conti_or_discrete=[]
    for line in open(path):
        if ":" in line:
            conti_or_discrete.append(line.split(":")[1])
    return conti_or_discrete


def get_class(path):
    class_list=[]
    result=""
    for line in open(path):
        if "|" in line:
            result=line.split("|")[0]
        break
    result=result.split(',')
    for index,item in enumerate(result):
        result[index]=result[index].strip()
    return result


def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list

 
def count_list(data,class_list):
    print "class_list=",class_list
    dicts={}
    class_count=[]
    print"class_list=",class_list
    for index,item in enumerate(class_list):
        dicts[item]=0
        class_count.append(0)#初始化 每一类包含的数据条数
    length=len(data)*1.0
    items_class=[item[-1] for item in data ]
    print"items_class=",items_class#这里输出的全部都是数字

    for item in items_class:
        print"class_list=",class_list.index(str(item))
        print"class_list.index(str(item))=",class_list.index(str(item))
        if class_count[class_list.index(str(item))]==0:
            print"class_list.index(str(item))=",class_list.index(str(item))
            print"type(class_count)=",type(class_count)
            print"class_count[0]=",class_count[0]
            class_count[class_list.index(str(item))]=1
        else:
            class_count[class_list.index(str(item))]=class_count[class_list.index(str(item))]+1
    return class_count


def read_data(path):
    source_data=pd.read_csv(path,header=None)#第一行不是标题
    lists=source_data.values.tolist()
    return lists

def prune_current_tree(model_under_judge,bestfeature,conti_discrete,sub_datas,count_list,class_list,feature_list):#需要修改
    print"进入prune_current_tree后的count_list=",count_list
    feature_flag=conti_discrete[feature_list.index(bestfeature)]
    DF=0.0 #degree of freedom
    flag=-1
    if "continuous" in feature_flag:#there's a"\n" in feature_flag,so here we use "in",NOT "=="
        #if "bestfeature" is continuous
        DF=(2-1)*(len(class_list)-1)#because continuous feature is always corresponding to binary sub-tree
        flag=1
    else:
        #if "bestfeature" is discrete
        factor_level=feature_flag.split(",")
        DF=(len(factor_level)-1)*1.0*(len(class_list)-1)#degree of freedom=(r-1)(s-1)
        flag=0

#☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆Set Critical Value here☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
    critical_value=scipy.stats.chi2.ppf(alpha,DF)#you can use different critical value with just setting "alhpa"
    critical_value=3.84#here conforms to the original CVP theory,of course this line will make the above line no working.
#-------------the above compute critical value----------------------------
    chi_square_statistics=0.0
    model=model_under_judge
    branches=model[bestfeature]
    branch_names=[]
    for key in branches:
        branch_names.append(key)

    if flag==1:#如果是连续属性
        chi_square_statistics=chi_square_continuous(bestfeature,branch_names,sub_datas,feature_list)
    if flag==0:#如果是离散属性
        chi_square_statistics=chi_square_discrete(bestfeature,sub_datas,feature_list)

#-------------judge whether to prune or NOT---－
    model_result=""
    if chi_square_statistics<=critical_value:
        #下面是剪枝操作
        print"剪枝以前的count_list=",count_list
        major_index=count_list.index(max(count_list))
        if len(sub_datas)==max(count_list):#如果不存在误判的数据
            model_result=str(class_list[major_index])+" ("+str(len(sub_datas))+")"
        else:
            print"class_list=",class_list
            print"major_index=",major_index
            print"count_list=",count_list
            model_result=str(class_list[major_index])+" ("+str(len(sub_datas))+"/"+str(len(sub_datas)-max(count_list))+")"
            print"model_result=",model_result

    else:#不剪枝,保留当前子树模型
        model_result=model#it means NOT prune
    return model_result





def CVP_result(model_input,fea_list,datasets,class_count,class_list,conti_discrete):#down->top
    if isinstance(model_input,str):
        return model_input
    best_feature=model_input.items()[0][0]#因为当前树的根节点肯定只有一个特征，第一个[0]是获取ｋｅｙ列表，当然，该列表中只有一个元素，所以再使用第二个[0]
    branches=model_input[best_feature]
    #如果还未到达树的最底层的叶子节点的上面一个分割节点，那么继续往下搜索。
    feature_list_origin=copy.deepcopy(fea_list)
    for branch in branches:#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
        sub_data=splitdatasets(best_feature,fea_list,branch,datasets)
        # print"model_input=",model_input
        # print"model_input[best_feature]=",model_input[best_feature]

        branch_count_list=count_list(sub_data,class_list)
        print"branch_count_list=",branch_count_list
        # print"进入递归调用前的特征列表：",fea_list

        model_input[best_feature][branch]=CVP_result(model_input[best_feature][branch],fea_list,sub_data,branch_count_list,class_list,conti_discrete)
    #处理完当前树的分支是否剪枝的判断以后，判断该树的子树是不是需要剪枝为一个叶子节点。  
    model_input=prune_current_tree(model_input,best_feature,conti_discrete,datasets,class_count,class_list,feature_list_origin)#如果树枝下面对应的子树都没有剪枝成功，可能存在剪枝的需要
    return model_input



# 离散的是（离散特征取值数－１）（类别数－１）
# 连续的自由度是类别数－１
# 然后当前数据集－>contingency table
# 计算卡方检验值，根据显著水平查找ｃｒｉｔｉｃａｌ　ｖａｌｕｅ，
# 如果小于critical value，返回叶子节点。
# 如果大于critical value 返回原来的模型。

# -----------------------------------------
    # sub_data是整个contingency列联表
    # ｃｌａｓｓ_list外面传入即可
    # feature_list外面传入即可
    # 计算ΣΣ的结果
    # 根据ａｌｐｈａ值求解critical value
    # 将计算结果与critical value进行比较，如果是小于，那么就返回剪枝后的模型

def abalone_parts_test():
    #下面这个是abalone的模型
    model={'Viscera': {'>0.0145': {'Shell': {'<=0.0345': {'Viscera': {'<=0.0285': ' 5 (50.0/9.0)', '>0.0285': ' 4 (3.0)'}}, '>0.0345': {'Sex': {'=M': ' 6 (6.0/3.0)', '=F': ' 5 (3.0)', '=I': ' 5 (59.0/12.0)'}}}}, '<=0.0145': {'Shucked': {'>0.007': ' 4 (66.0/31.0)', '<=0.007': {'Shucked': {'>0.0045': {'Shucked': {'>0.005': {'Height': {'<=0.02': ' 4 (2.0)', '>0.02': ' 3 (4.0)'}}, '<=0.005': ' 4 (3.0)'}}, '<=0.0045': {'Height': {'<=0.025': ' 1 (2.0/1.0)', '>0.025': ' 3 (2.0)'}}}}}}}}
    
    #---------get Attribute list--------------------------
    path='./abalone_parts.data'
    datasets=read_data(path)

    name_path='./abalone.names'
    feature_list=get_Attribute(name_path)
    class_list=get_class(name_path)
    print "class_list=",class_list
    conti_discrete=conti_or_discrete_list(name_path)#获取特征是离散还是连续的列表,这个是用来计算“根据显著水平获取critical value值的”
    print"unpruned_model=",model
    createPlot(model)

    # #-----------get datasets------------------------
    print"class_list=",class_list
    print"------------------q----------------------------"
    counts_list=count_list(datasets,class_list)
    result=CVP_result(copy.deepcopy(model),feature_list,datasets,counts_list,class_list,conti_discrete)
    print "pruned_model=",result
    createPlot(result)
    accuracy_unprune,accuracy_prune=accuracy_analysis(model,result,datasets,feature_list,name_path)
    print"accuracy_unprune=",accuracy_unprune
    print"accuracy_prune=",accuracy_prune

def crx_test():
    #下面这个是abalone的模型
    model={'A9': {'=t': {'A15': {'>228': ' + (106.0/2.0)', '<=228': {'A11': {'>3': {'A15': {'>4': {'A15': {'<=5': ' - (2.0)', '>5': {'A7': {'=v': ' + (5.0)', '=z': ' - (1.0)', '=dd': ' + (0.0)', '=ff': ' + (0.0)', '=o': ' + (0.0)', '=n': ' + (0.0)', '=h': ' + (3.0)', '=bb': ' + (1.0)', '=j': ' + (0.0)'}}}}, '<=4': ' + (25.0)'}}, '<=3': {'A4': {'=u': {'A7': {'=v': {'A14': {'<=110': ' + (18.0/1.0)', '>110': {'A15': {'>8': ' + (4.0)', '<=8': {'A6': {'=aa': {'A2': {'<=41': ' - (3.0)', '>41': ' + (2.0)'}}, '=w': {'A12': {'=t': ' - (2.0)', '=f': ' + (3.0)'}}, '=q': {'A12': {'=t': ' + (4.0)', '=f': ' - (2.0)'}}, '=ff': ' - (0.0)', '=r': ' - (0.0)', '=i': ' - (0.0)', '=x': ' - (0.0)', '=e': ' - (0.0)', '=d': ' - (2.0)', '=c': ' - (4.0/1.0)', '=m': {'A13': {'=g': ' + (2.0)', '=p': ' - (0.0)', '=s': ' - (5.0)'}}, '=cc': ' + (2.0/1.0)', '=k': ' - (2.0)', '=j': ' - (0.0)'}}}}}}, '=z': ' + (1.0)', '=bb': {'A14': {'<=164': ' + (3.4/0.4)', '>164': ' - (5.6)'}}, '=ff': ' - (1.0)', '=o': ' + (0.0)', '=n': ' + (0.0)', '=h': ' + (18.0)', '=dd': ' + (0.0)', '=j': ' - (1.0)'}}, '=l': ' + (0.0)', '=y': {'A13': {'=g': {'A14': {'<=204': ' - (16.0/1.0)', '>204': ' + (5.0/1.0)'}}, '=p': ' - (0.0)', '=s': ' + (2.0)'}}, '=t': ' + (0.0)'}}}}}}, '=f': {'A13': {'=g': ' - (204.0/10.0)', '=p': {'A2': {'<=36': ' - (4.0/1.0)', '>36': ' + (2.0)'}}, '=s': {'A4': {'=u': {'A6': {'=aa': ' - (0.0)', '=w': ' - (0.0)', '=q': ' - (1.0)', '=ff': ' - (2.0)', '=r': ' - (0.0)', '=i': ' - (3.0)', '=x': ' + (1.0)', '=e': ' - (0.0)', '=d': ' - (2.0)', '=c': ' - (3.0)', '=m': ' - (3.0)', '=cc': ' - (1.0)', '=k': ' - (4.0)', '=j': ' - (0.0)'}}, '=l': ' + (1.0)', '=y': ' - (8.0/1.0)', '=t': ' - (0.0)'}}}}}}
    path="./crx.data"
    name_path="./crx.names"
    datasets=read_data(path)

    feature_list=get_Attribute(name_path)
    class_list=get_class(name_path)
    print "class_list=",class_list
    conti_discrete=conti_or_discrete_list(name_path)#获取特征是离散还是连续的列表,这个是用来计算“根据显著水平获取critical value值的”
    print"unpruned_model=",model
    createPlot(model)

    # #-----------get datasets------------------------
    print"class_list=",class_list
    print"------------------q----------------------------"
    counts_list=count_list(datasets,class_list)
    result=CVP_result(copy.deepcopy(model),feature_list,datasets,counts_list,class_list,conti_discrete)
    print "pruned_model=",result
    createPlot(result)
    accuracy_unprune,accuracy_prune=accuracy_analysis(model,result,datasets,feature_list,name_path)
    print"accuracy_unprune=",accuracy_unprune
    print"accuracy_prune=",accuracy_prune


if __name__ == '__main__':
    # abalone_parts_test()
    crx_test()


