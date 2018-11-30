# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-10-31 16:48:38
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-11-30 19:17:31

import treePlotter
import copy
from getNumofCommonSubstr import getNumofCommonSubstr






branches_label=["<=","=",">"]




def classify(inputTree,features,testVec,conti_or_discrete):#这里的inputTree就是决策树的序列化表示方法,python中是字典类型,也可以看做是json类型
    firstStr = inputTree.keys()[0]#获取决策树的当前分割属性
    secondDict = inputTree[firstStr]#当前分割节点下面的一堆树枝+节点

    featIndex = features.index(firstStr)#当前是第几个特征
    key = testVec[featIndex]#根据划分属性的下标来获取测试数据的对应下标的属性的具体取值

#--------------获得子树--------------------------------
    if conti_or_discrete[featIndex].strip().split(".")[0]!="continuous":
        # print"secondDict=",secondDict
        # print"testVec=",testVec
        valueOfFeat = secondDict[key]#根据这个值来顺着树枝key选择子树secondDict[key](离散特征)
    else:
        item_lists=[]
        for item in secondDict:
            item_lists.append(item)
        common_str=getNumofCommonSubstr(item_lists[0],item_lists[1])[0]#common_str是
        # print"item_lists[0]=",item_lists[0]
        # print"item_lists[1]=",item_lists[1]
        # print"common_str=",common_str
        if key<=float(common_str):
            key="<="+common_str
            valueOfFeat = secondDict[key]
        else:
            key=">"+common_str
            valueOfFeat = secondDict[key]

#----------------获得子树------------------------------

    if isinstance(valueOfFeat, dict): #如果是子树
        classLabel = classify(valueOfFeat, features, testVec,conti_or_discrete)#递归调用
    else: #如果是叶子节点
        classLabel = valueOfFeat
    return classLabel#递归函数的结束条件


#注意，不支持包含缺失值的数据的测试
def classify_C45(valueOfFeat, features, data,conti_or_discrete):#注意，这里的ｄａｔａ指的是一条数据，不是一堆数据
    for index,item in enumerate(conti_or_discrete):#因为模型中离散特征有“＝”符号，所以这里给离散特征的数据加上“＝”，方便预测
        item=item.strip().split(".")[0]
        if item!="continuous":
            data[index]="="+data[index]
        else:
            try:
                data[index]=float(data[index])
            except:
                pass
    return classify(valueOfFeat, features, data,conti_or_discrete) 

def conti_or_discrete_list(path):
    conti_or_discrete=[]
    for line in open(path):
        if ":" in line:
            conti_or_discrete.append(line.split(":")[1])
    return conti_or_discrete


def accuracy_analysis(model,model_pruned,datasets,feature_list,name_path):
    conti_or_discrete=conti_or_discrete_list(name_path)
    count=0
    unpruned_accuracy=0.0
    for item in copy.deepcopy(datasets):
        predict=classify_C45(model,feature_list,item,conti_or_discrete)
        if predict.split("(")[0].strip()==str(item[-1]):
            count+=1
        else:
            print item
    accuracy_unprune=float(count)/len(datasets)
    count=0
    print"---------------这里是分割线------------"
    for item in datasets:
        # print "item=",item
        predict=classify_C45(model_pruned,feature_list,item,conti_or_discrete)
        if predict.split("(")[0].strip()==str(item[-1]):
            count+=1
        else:
            print item
    accuracy_prune=float(count)/len(datasets)
    return accuracy_unprune,accuracy_prune


def test1():
    name_path="./crx.names"
    features=["A1","A2","A3","A4","A5","A6","A7","A8","A9","A10","A11","A12","A13","A14","A15"]
    testVec1=['b',30.83,0,'u','g','w','v',1.25,'t','t',01,'f','g',00202,0]#+
    testVec2=['a',19.17,0.585,'y','p','aa','v',0.585,'t','f',0,'t','g',00160,0]#-
    testVec1=['b',30.83,0,'u','g','w','v',1.25,'t','t',01,'f','g',00202,0]
    conti_or_discrete=conti_or_discrete_list(name_path)
    result1=classify_C45(myTree,features,testVec1,conti_or_discrete)
    result2=classify_C45(myTree,features,testVec2,conti_or_discrete)

    print "result1=",result1
    print "result2=",result2
if __name__ == '__main__':
    myTree=model={'A9': {'=t': {'A15': {'>228': ' + (106.0/2.0)', '<=228': {'A11': {'>3': {'A15': {'>4': {'A15': {'<=5': ' - (2.0)', '>5': {'A7': {'=v': ' + (5.0)', '=z': ' - (1.0)', '=dd': ' + (0.0)', '=ff': ' + (0.0)', '=o': ' + (0.0)', '=n': ' + (0.0)', '=h': ' + (3.0)', '=bb': ' + (1.0)', '=j': ' + (0.0)'}}}}, '<=4': ' + (25.0)'}}, '<=3': {'A4': {'=u': {'A7': {'=v': {'A14': {'<=110': ' + (18.0/1.0)', '>110': {'A15': {'>8': ' + (4.0)', '<=8': {'A6': {'=aa': {'A2': {'<=41': ' - (3.0)', '>41': ' + (2.0)'}}, '=w': {'A12': {'=t': ' - (2.0)', '=f': ' + (3.0)'}}, '=q': {'A12': {'=t': ' + (4.0)', '=f': ' - (2.0)'}}, '=ff': ' - (0.0)', '=r': ' - (0.0)', '=i': ' - (0.0)', '=x': ' - (0.0)', '=e': ' - (0.0)', '=d': ' - (2.0)', '=c': ' - (4.0/1.0)', '=m': {'A13': {'=g': ' + (2.0)', '=p': ' - (0.0)', '=s': ' - (5.0)'}}, '=cc': ' + (2.0/1.0)', '=k': ' - (2.0)', '=j': ' - (0.0)'}}}}}}, '=z': ' + (1.0)', '=bb': {'A14': {'<=164': ' + (3.4/0.4)', '>164': ' - (5.6)'}}, '=ff': ' - (1.0)', '=o': ' + (0.0)', '=n': ' + (0.0)', '=h': ' + (18.0)', '=dd': ' + (0.0)', '=j': ' - (1.0)'}}, '=l': ' + (0.0)', '=y': {'A13': {'=g': {'A14': {'<=204': ' - (16.0/1.0)', '>204': ' + (5.0/1.0)'}}, '=p': ' - (0.0)', '=s': ' + (2.0)'}}, '=t': ' + (0.0)'}}}}}}, '=f': {'A13': {'=g': ' - (204.0/10.0)', '=p': {'A2': {'<=36': ' - (4.0/1.0)', '>36': ' + (2.0)'}}, '=s': {'A4': {'=u': {'A6': {'=aa': ' - (0.0)', '=w': ' - (0.0)', '=q': ' - (1.0)', '=ff': ' - (2.0)', '=r': ' - (0.0)', '=i': ' - (3.0)', '=x': ' + (1.0)', '=e': ' - (0.0)', '=d': ' - (2.0)', '=c': ' - (3.0)', '=m': ' - (3.0)', '=cc': ' - (1.0)', '=k': ' - (4.0)', '=j': ' - (0.0)'}}, '=l': ' + (1.0)', '=y': ' - (8.0/1.0)', '=t': ' - (0.0)'}}}}}}
   
    test1()




