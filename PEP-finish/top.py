# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-11-10 20:20:03
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-11-18 19:42:44

from treePlotter import createPlot
from math import sqrt
from split import splitdatasets
import copy
from predict import *

#----------------ｇｅｔ Attribute list of datasets--------------------------------------------
def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list


#------------------------------------------------------------
def counts(model):#统计当前树的叶子数量
    leaves_count=0
    if isinstance(model,str):
        leaves_count=leaves_count+1
        return leaves_count

    best_feature=model.items()[0][0]#因为当前树的根节点肯定只有一个特征，第一个[0]是获取ｋｅｙ列表，当然，该列表中只有一个元素，所以再使用第二个[0]
    branches=model[best_feature]
    for sub_tree in branches:#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
        # print sub_tree
        leaves_count+=counts(model[best_feature][sub_tree])
    return leaves_count



#------------------------------------------------------------
# data=' 4 (2.0)'
# data='6 (6.0/3.0)'
def brackets_data(data):#用来从叶子的ｓｔｒ中提取到分类错误的数据，用来被后面的ＰＥＰ判据使用。
    if '/' in data:
        leaf_error=data.split('/')[1].split(')')[0]
        leaf_error=float(leaf_error)
    else:
        leaf_error=0
    return leaf_error


def errors_counts(models):#统计当前整棵树（还没剪枝的时候）的分类错误数量
    error_count=0
    if isinstance(models,str):
        error_count=error_count+ brackets_data(models)
        # print "当前的error_count=",error_count
        return error_count

    best_feature=models.items()[0][0]#因为当前树的根节点肯定只有一个特征，第一个[0]是获取ｋｅｙ列表，当然，该列表中只有一个元素，所以再使用第二个[0]
    branches=models[best_feature]
    for sub_tree in branches:#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
        error_count+=errors_counts(models[best_feature][sub_tree])
    return error_count

#------------------------------------------------------------
# data=' 4 (2.0)'
# data='7 (6.0/3.0)'
def leaf_items(data):
    if '/' in data:
        leaf_item_counts=data.split('/')[0].split('(')[1]#如果是上面的例子，那么得到6
        leaf_item_counts=float(leaf_item_counts)
    else:
        leaf_item_counts=data.split('(')[1].split(')')[0]#如果是上面的例子，那么得到2
        leaf_item_counts=float(leaf_item_counts)
    return leaf_item_counts

def items_count(model_in):#统计当前数据集数量
    item_count=0
    if isinstance(model_in,str):
        # print"model=",model
        item_count=item_count+ leaf_items(model_in)
        return item_count

    best_feature=model_in.items()[0][0]#因为当前树的根节点肯定只有一个特征，第一个[0]是获取ｋｅｙ列表，当然，该列表中只有一个元素，所以再使用第二个[0]
    branches=model_in[best_feature]
    for sub_tree in branches:#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
        item_count+=items_count(model_in[best_feature][sub_tree])
    return item_count


#------------------criterion-relevant------------------------------------------
def Standard_deviation(total_items,errors):
    return sqrt(float(errors)/total_items*(total_items-errors))

def unpruned_criterion(unprune_errors,leaf_count,standard_deviation):
    return  unprune_errors+0.5*leaf_count+standard_deviation

def pruned_criterion(pruned_errors):
    return pruned_errors+0.5


def criterion_comparison(unpruned_errors,pruned_errors):
    if unpruned_errors<pruned_errors:#不满足PEP剪枝条件
        return False

    else:
        return True#满足PEP剪枝条件
#----------------------------top module--------------------------------
import csv
from collections import Counter
import pandas as pd 

def read_data(path):
    source_data=pd.read_csv(path,header=None)#第一行不是标题
    lists=source_data.values.tolist()
    return lists


def pruned_error_counts(datasets):
    column = [row[-1] for row in datasets]
    # print column
    total_items=len(column)#quantity of total items
    majority_class=Counter(column).most_common(1)[0][0]
    majority_items=Counter(column).most_common(1)[0][1]#quantity of right classified 
    error_mis_classified=total_items-majority_items
    return majority_class,majority_items,error_mis_classified


def PEP_result(model_input,fea_list,datasets):#top-down
    #－－－－－－－－－－－－－－－－截止条件－－－－－－－－－－－－－
    if isinstance(model_input,str):#如果是叶子节点，直接返回。
        return model_input
        #注意，这里不要指望直接通过判断该节点的孩子节点来判断是否直接返回。
        # 因为可能存在一种情况，左孩子节点是叶子节点，右孩子节点是子树的根节点
#--------------for criterion_unprune----------------------------------
    leaf_count=counts(model_input)#count quantity of leaves of current tree
    errors=errors_counts(model_input)#count quantity of errors of current tree
    total_items=items_count(model_input)#count quantity of current datasets distributed on all leaves
    standard_deviation=Standard_deviation(total_items,errors)
    criterion_unprune=unpruned_criterion(errors,leaf_count,standard_deviation)

#--------------for criterion_pruned----------------------------------
    majority_class,majority_items,error_mis_classified=pruned_error_counts(datasets)
    criterion_pruned=pruned_criterion(error_mis_classified)
    result=criterion_comparison(criterion_unprune,criterion_pruned)
    if result is True:
        leaf=str(majority_class)+"("+str(len(datasets))+"/"+str(error_mis_classified)+")"
        return leaf#完成剪枝操作

#－－－－－－－－－－－－－－－－递归调用部分－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－
    if result is False:
        best_feature=model_input.items()[0][0]#因为当前树的根节点肯定只有一个特征，第一个[0]是获取ｋｅｙ列表，当然，该列表中只有一个元素，所以再使用第二个[0]
        branches=model_input[best_feature]
        for branch in branches:#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
            sub_data=splitdatasets(best_feature,fea_list,branch,datasets)
            model_input[best_feature][branch]=PEP_result(model_input[best_feature][branch],fea_list,sub_data)
            #model of subtree is model_input[best_feature][branch]
    return model_input




#--------------------剪枝前后的准确率预测---------------------------------


#for credit-a datasets from UCI
def credit_a_test():
    model={'A9': {'=t': {'A15': {'>228': ' + (106.0/2.0)', '<=228': {'A11': {'>3': {'A15': {'>4': {'A15': {'<=5': ' - (2.0)', '>5': {'A7': {'=v': ' + (5.0)', '=z': ' - (1.0)', '=dd': ' + (0.0)', '=ff': ' + (0.0)', '=o': ' + (0.0)', '=n': ' + (0.0)', '=h': ' + (3.0)', '=bb': ' + (1.0)', '=j': ' + (0.0)'}}}}, '<=4': ' + (25.0)'}}, '<=3': {'A4': {'=u': {'A7': {'=v': {'A14': {'<=110': ' + (18.0/1.0)', '>110': {'A15': {'>8': ' + (4.0)', '<=8': {'A6': {'=aa': {'A2': {'<=41': ' - (3.0)', '>41': ' + (2.0)'}}, '=w': {'A12': {'=t': ' - (2.0)', '=f': ' + (3.0)'}}, '=q': {'A12': {'=t': ' + (4.0)', '=f': ' - (2.0)'}}, '=ff': ' - (0.0)', '=r': ' - (0.0)', '=i': ' - (0.0)', '=x': ' - (0.0)', '=e': ' - (0.0)', '=d': ' - (2.0)', '=c': ' - (4.0/1.0)', '=m': {'A13': {'=g': ' + (2.0)', '=p': ' - (0.0)', '=s': ' - (5.0)'}}, '=cc': ' + (2.0/1.0)', '=k': ' - (2.0)', '=j': ' - (0.0)'}}}}}}, '=z': ' + (1.0)', '=bb': {'A14': {'<=164': ' + (3.4/0.4)', '>164': ' - (5.6)'}}, '=ff': ' - (1.0)', '=o': ' + (0.0)', '=n': ' + (0.0)', '=h': ' + (18.0)', '=dd': ' + (0.0)', '=j': ' - (1.0)'}}, '=l': ' + (0.0)', '=y': {'A13': {'=g': {'A14': {'<=204': ' - (16.0/1.0)', '>204': ' + (5.0/1.0)'}}, '=p': ' - (0.0)', '=s': ' + (2.0)'}}, '=t': ' + (0.0)'}}}}}}, '=f': {'A13': {'=g': ' - (204.0/10.0)', '=p': {'A2': {'<=36': ' - (4.0/1.0)', '>36': ' + (2.0)'}}, '=s': {'A4': {'=u': {'A6': {'=aa': ' - (0.0)', '=w': ' - (0.0)', '=q': ' - (1.0)', '=ff': ' - (2.0)', '=r': ' - (0.0)', '=i': ' - (3.0)', '=x': ' + (1.0)', '=e': ' - (0.0)', '=d': ' - (2.0)', '=c': ' - (3.0)', '=m': ' - (3.0)', '=cc': ' - (1.0)', '=k': ' - (4.0)', '=j': ' - (0.0)'}}, '=l': ' + (1.0)', '=y': ' - (8.0/1.0)', '=t': ' - (0.0)'}}}}}}
    path="./crx.data"
    name_path="./crx.names"
    fea_list=get_Attribute(name_path)
    datasets=read_data(path)
    model_pruned=PEP_result(copy.deepcopy(model),fea_list,copy.deepcopy(datasets))
 
    accuracy_unprune,accuracy_prune=accuracy_analysis(model,model_pruned,datasets,fea_list,name_path)
    print"accuracy_unprune=",accuracy_unprune
    print"accuracy_prune=",accuracy_prune
    print"model=",model
    print"model_pruned=",model_pruned
    createPlot(model)
    createPlot(model_pruned)

def abalone_parts_test():
    model={'Viscera': {'>0.0145': {'Shell': {'<=0.0345': {'Viscera': {'<=0.0285': ' 5 (50.0/9.0)', '>0.0285': ' 4 (3.0)'}}, '>0.0345': {'Sex': {'=M': ' 6 (6.0/3.0)', '=F': ' 5 (3.0)', '=I': ' 5 (59.0/12.0)'}}}}, '<=0.0145': {'Shucked': {'>0.007': ' 4 (66.0/31.0)', '<=0.007': {'Shucked': {'>0.0045': {'Shucked': {'>0.005': {'Height': {'<=0.02': ' 4 (2.0)', '>0.02': ' 3 (4.0)'}}, '<=0.005': ' 4 (3.0)'}}, '<=0.0045': {'Height': {'<=0.025': ' 1 (2.0/1.0)', '>0.025': ' 3 (2.0)'}}}}}}}}

#---------get Attribute list--------------------------
    name_path='./abalone.names'
    feature_list=get_Attribute(name_path)
#-----------get datasets------------------------
    path='./abalone_parts.data'
    datasets=read_data(path)
# #--------Start PEP_pruning---------------------------
    model_pruned=PEP_result(copy.deepcopy(model),feature_list,datasets)

    print"剪枝前的模型=",model
    print"剪枝后的模型=",model_pruned

    createPlot(model)
    createPlot(model_pruned)
#--------Start accuracy computation---------------------------
    print "unpruned_accuracy,pruned_accuracy",accuracy_analysis(model,model_pruned,datasets,feature_list,name_path)



if __name__ == '__main__':
    # abalone_parts_test()
    credit_a_test()




