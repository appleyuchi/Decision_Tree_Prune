#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

import pandas as pd
import math
import treePlotter
import json
import collections
import treePlotter
from most_class_compute import most_class_computes

import copy
from copy import deepcopy

#读入数据
#取得特定的11条数据作为训练,
#剩余6条数据作为验证

def load_data(file_name):
    with open(file_name, 'r') as f:
      df = pd.read_csv(f,sep=",")
      print(df)
      train_data = df.values[:11, 1:].tolist()
    print("当前读取的数据是",json.dumps(train_data,ensure_ascii=False))
    test_data = df.values[11:, 1:].tolist()
    print(test_data)
    labels = df.columns.values[1:-1].tolist()
    #labels = df.columns.values
    print(labels)

    dataSet=train_data+test_data
    labels_all = ['色泽', '根蒂', '敲击', '纹理', '脐部', '触感']
    # 各个样本的权重

    # labels_full特征对应的所有可能的取值
    labels_full = {}
    for i in range(len(labels_all)):
        labelList = [example[i] for example in dataSet]
        uniqueLabel = set(labelList)
        labels_full[labels_all[i]] = uniqueLabel

    return train_data, test_data, dataSet,labels,labels_full



# def calcShannonEnt(dataSet):#计算Gini,函数名字故意和熵计算的函数名一样,仅仅是为了方便替换
#     numEntries=len(dataSet)
#     labelCounts={}
#     #给所有可能分类创建字典
#     for featVec in dataSet:
#         currentLabel=featVec[-1]
#         if currentLabel not in labelCounts.keys():
#             labelCounts[currentLabel]=0
#         labelCounts[currentLabel]+=1
#     Gini=1.0
#     #以2为底数计算香农熵
#     for key in labelCounts:
#         prob = float(labelCounts[key])/numEntries
#         Gini-=prob*prob
#     return Gini

#这个是在计算给定数据集的类别标签的信息熵
def calcShannonEnt(dataSet): 
    numEntries = len(dataSet) #数据集长度
    labelCounts = {} 
    # 给所有可能分类创建字典 
    for featVec in dataSet: 
        currentLabel = featVec[-1] #当前的类标签
        if currentLabel not in labelCounts.keys(): #计算当前类标签的频次
            labelCounts[currentLabel] = 0 
        labelCounts[currentLabel] += 1 
    shannonEnt = 0.0 
    # 以2为底数计算香农熵 
    for key in labelCounts: 
        prob = float(labelCounts[key]) / numEntries#每个类标签所对应的类权重 
        shannonEnt -= prob * math.log(prob, 2) #计算总的熵
    return shannonEnt

# 对离散变量划分数据集，取出该特征取值为value的所有样本
def splitDataSet(dataSet, axis, value):
    print"-----------------进入splitDataSet-------------------------------"
    retDataSet = []
    for featVec in dataSet:
        if featVec[axis] == value:
            reducedFeatVec = featVec[:axis]
            print"reducedFeatVec①=",json.dumps(reducedFeatVec,ensure_ascii=False)
            reducedFeatVec.extend(featVec[axis + 1:])
            print"reducedFeatVec②=",json.dumps(reducedFeatVec,ensure_ascii=False)
            retDataSet.append(reducedFeatVec)
    return retDataSet


#这里的数据集是"当前数据集",不是最初的数据集
#并且这个数据集是剩下未分割属性的数据集,里面的每一条数据不再是完整的一条数据
def chooseBestFeatureToSplit(dataSet, labels):
    print"chooseBestFeatureToSplit=",json.dumps(dataSet,ensure_ascii=False)
    numFeatures = len(dataSet[0]) - 1#数据集中剩余的没有划分的特征的数量
    baseEntropy = calcShannonEnt(dataSet)
    bestInfoGain = 0.0
    bestFeature = -1
    bestSplitDict = {}
    for i in range(numFeatures):
        featList = [example[i] for example in dataSet]
        uniqueVals = set(featList)
        newEntropy = 0.0
        # 计算该特征下每种划分的信息熵
        for value in uniqueVals:
            subDataSet = splitDataSet(dataSet, i, value)
            prob = float(len(subDataSet)) / float(len(dataSet))
            newEntropy += prob * calcShannonEnt(subDataSet)
        infoGain = baseEntropy - newEntropy
        if infoGain > bestInfoGain:
            bestInfoGain = infoGain
            bestFeature   = i
    return bestFeature



def classify(inputTree, featLabels, testVec):
    #featLabels包含当前数据集的所有特征
    print"-------------进入classify函数-----------------"
    firstStr = list(inputTree.keys())[0]#这个是当前根节点的名称
    print"firstStr=",firstStr
    secondDict = inputTree[firstStr]#这个是当前根节点下面所有的树枝
    print"secondDict=",json.dumps(secondDict,ensure_ascii=False)
    featIndex = featLabels.index(firstStr)#
    for key in secondDict.keys():#遍历决策树模型的当前根节点下面的所有的树枝
        if testVec[featIndex] == key:#如果有一个树枝上的取值和测试向量"对应"上了,就继续递归"对应"
            if type(secondDict[key]).__name__ == 'dict':#如果当前的树枝连着一颗子树,因为如果是子树,那么字数的存储形式一定是dict类型
                classLabel = classify(secondDict[key], featLabels, testVec)#那么就进行递归判断
            else:
                classLabel = secondDict[key]#如果是叶子节点,那么测试数据的类别就是该叶子节点的内容
    print"classLabel=",classLabel
    print"type(classLabel)=",type(classLabel)
    return classLabel



def majorityCnt(classList):
    classCount={}
    for vote in classList:
        if vote not in classCount.keys():
            classCount[vote]=0
        classCount[vote]+=1
    return max(classCount)


#这个用于预剪枝
def testing_feat(feat, train_data, test_data, labels):
    print"train_data=",json.dumps(train_data,ensure_ascii=False)
    class_list = [example[-1] for example in train_data]
    bestFeatIndex = labels.index(feat)
    train_data = [example[bestFeatIndex] for example in train_data]
    test_data = [(example[bestFeatIndex], example[-1]) for example in test_data]
    all_feat = set(train_data)
    error = 0.0
    for value in all_feat:
        class_feat = [class_list[i] for i in range(len(class_list)) if train_data[i] == value]
        major = majorityCnt(class_feat)
        for data in test_data:
            if data[0] == value and data[1] != major:
                error += 1.0
    # print 'myTree %d' % error
    return error


#这个函数用于预剪枝和后剪枝
def testingMajor(major, data_test):
    error = 0.0
    for i in range(len(data_test)):
        if major != data_test[i][-1]:
            error += 1
    # print 'major %d' % error
    return float(error) 


#这个函数专门用于"后剪枝"
def testing(myTree,data_test,labels):
    #这里输入的labels不是全部的特征名称
    #这里输入的data_test不带有全部的特征名称
    print"----------进入testing函数--------------"
    print"data_test=",json.dumps(data_test,ensure_ascii=False)

    print"labels=",json.dumps(labels,ensure_ascii=False)
    error=0.0    
    for i in range(len(data_test)):
        if classify(myTree,labels,data_test[i])!=data_test[i][-1]:#如果预测结果与验证数据的类别标签不一致
            error+=1    #那么错误数就+1
    print ('myTree %d' %error)
    return float(error)


#递归产生决策树**
def createTree(dataSet,labels,data_full,labels_full,test_data,mode):
    classList=[example[-1] for example in dataSet]
    # dataSet指的是当前的数据集,不是最初的数据集
    # classList指的是当前数据集的所有标签(不去重)

    #下面是递归截止条件
    if classList.count(classList[0])==len(classList):#这个意思是如果当前数据集中的所有数据都属于同一个类别
       return classList[0]
    if len(dataSet[0])==1:
       return majorityCnt(classList)

    #选择最佳分割特征
    #labels_copy = labels
    labels_copy = copy.deepcopy(labels)#深拷贝就是:labels_copy和lables撇清关系
    bestFeat=chooseBestFeatureToSplit(dataSet,labels)
    bestFeatLabel=labels[bestFeat]

    if mode == "unpru" or mode == "post":#因为后剪枝是"先建树-后剪枝"的思路,所以这里建了再说
        myTree = {bestFeatLabel: {}}
    elif mode == "prev":
        # 下面一句代码有2种理解方式
        #如果剪枝前的准确度大于剪枝后的准确度(第1种理解方式,与不等号左右对应)
        #如果剪枝前的错误数量小于剪枝后的错误数量(第2种理解方式,与不等号左右对应)
        #那么就保留该子树
        if testing_feat(bestFeatLabel, dataSet, test_data, labels_copy) < testingMajor(majorityCnt(classList),test_data):
            myTree = {bestFeatLabel: {}}
        else:
            return majorityCnt(classList)
            #这里的操作,相当于抹掉了前面的myTree的赋值操作,因为这里是直接返回一个叶子节点
            #也就是把根节点变成了叶子节点,实现了预剪枝操作

    featValues=[example[bestFeat] for example in dataSet]
    uniqueVals=set(featValues)
    #uniqueVals用来获得当前数据集的最佳分割属性剩余的取值有哪些

    del (labels[bestFeat])#删除根节点的已经用过的特征

    for value in uniqueVals:
    #遍历当前根节点的所有树枝,
    # 因为树枝就是当前分割节点对应的特征的取值,
    # 这些取值是剩余数据集的取值,
    # 并不是最初的所有可能的取值
    # 所以这里也会留下隐患,有些虚拟的叶子节点无法生成(虚拟叶子节点见上面的博客链接中)
        subLabels = labels[:]
        myTree[bestFeatLabel][value] = createTree(splitDataSet(dataSet, bestFeat, value), subLabels, data_full, labels_full,splitDataSet(test_data, bestFeat, value), mode=mode)

    if mode == "post":
        if testing(myTree, test_data, labels_copy)> testingMajor(majorityCnt(classList), test_data):
            return majorityCnt(classList)
            #实现后剪枝操作
            #无视当前的myThree,直接返回一个叶子节点,等效于实现了REP后剪枝

    return myTree



#补充算法中提到的虚拟节点
#虚拟节点见博客记录:
#https://blog.csdn.net/appleyuchi/article/details/83027964
def makeTreeFull(satisfy_lists,datasets,myTree, labels_full, parentClass, default):
    print("－－－－－－－－－－－－－－进入makeTreeFull函数－－－－－－－－－－－－－－－－－")
    print("１入口satisfy_lists=",satisfy_lists)
    print("-------------------------------")
    print("myTree=",myTree)
    print("prarentClass=",parentClass)
    print("-------------------------------")
    """
    将数中的不存在的特征标签进行补全，补全为父节点中出现最多的类别
    :param myTree: 生成的树
    :param labels_full: 特征的全部标签
    :param parentClass: 父节点中所含最多的类别
    :param default: 如果父节点没有类别，而此时又有缺失的标签，默认标签分类设置为default
    :return:
    """
    # 拿到当前的根节点☆☆☆☆☆☆☆☆☆
    root_key=''
    for item in myTree.keys():
        if item!='Entropy=':
            root_key=item
    # root_key = list(myTree.keys())[0]
    # 得到根节点对应的子树，也就是key对应的内容
    sub_tree = myTree[root_key]

    # 如果是叶子节点就结束
    if isinstance(sub_tree, str):
        print"如果认为是叶子节点"
        return



    if satisfy_lists!=[]:
        print("-----------------进入most_class_computes------------------------")
        print("satisfy_lists=",satisfy_lists)
        datasete,most_class=most_class_computes(datasets,satisfy_lists)
        print("most_class=",most_class)
        print("-----------------离开most_class_computes-----------------------")
        #等号左侧的ｄａｔａｓｅｔｓ是满足新的特征取值
    else:
        most_class = None
    print("离开２入口satisfy_lists=",satisfy_lists)
    print("most_class=",most_class)
    parentClass=most_class
###########################################
    # 循环遍历全部特征标签，将不存在标签添加进去
    for label in labels_full[root_key]:
        if label not in sub_tree.keys():
            print"查看是否存在该标签=",label
            print("parentClass=",parentClass)
            # 如果此时父标签最多的分类不为None，则将新的标签设置为父标签
            if parentClass is not None:
                # sub_tree[label] = parentClass+"(虚)"#这里加了个虚,表示该叶子节点对应的训练集数据不存在
                sub_tree[label] = parentClass
            # 否则设置为default
            else:
                sub_tree[label] = default


    print("离开２入口satisfy_lists=",satisfy_lists)
    print("most_class=",most_class)
    # 递归处理


    for sub_key in sub_tree.keys():
        print"树枝sub_key=",sub_key
        print"most_class=",most_class
        print"sub_tree[sub_key]=",sub_tree[sub_key]
        print("－－－－－－－－－－－－－－－－进入ｆｏｒ循环☆－－－－－－－－－－－－－－")
    #这里的ｓｕｂ_key是树枝的取值
    #所以这个ｆｏｒ循环在遍历当前根节点的每个树枝
    # 也就是在遍历当前特征的每种取值
        if isinstance(sub_tree[sub_key], dict):#如果子树不是叶子节点
            print("－－－－－－－－－－－－－－递归调用前－－－－－－－－－－－－－－")
            temp=copy.copy(satisfy_lists)
            print("satisfy_lists=",satisfy_lists)
            satisfy_lists.append(sub_key)
            makeTreeFull(satisfy_lists,dataSet,myTree=sub_tree[sub_key], labels_full=labels_full, parentClass=most_class, default=default)
            satisfy_lists=temp

def precision_accuracy(myTree,test_data):
    features=['色泽', '根蒂', '敲击', '纹理', '脐部', '触感']
    P_N=len(test_data)
    test=['青绿', '蜷缩', '沉闷', '稍糊', '稍凹', '硬滑']
    TP_TN=0
    for item in test_data:
        label=item[-1]
        raw_data=item[0:-1]#裸数据的意思就是不带标签的数据
        predict=classify(myTree,features,raw_data)
        print "predict=",predict
        print"label=",label
        if predict==label:
            TP_TN=TP_TN+1
        else:
            print"预测错误的数据是=",json.dumps(item,ensure_ascii=False)
    accuracy=float(TP_TN)/float(P_N)
    return accuracy


# accuracy = （TP+TN）/(P+N)，
# 被分对的样本数除以所有的样本数，通常来说，正确率越高，分类器越好；




import json
if __name__ == "__main__":
    train_data, test_data,dataSet,labels,labels_full = load_data("watermelon_4_2_Chinese.csv")
    dataSet=train_data+test_data
    data_full = train_data[:]

# --------------pre-pruning,annote here when compare the accuracy before and after post-pruning-------------
    # labels_pre=copy.deepcopy(labels)
    # mode="prev"#未剪枝
    # satisfy_lists=[]
    # myTree = createTree(train_data, labels_pre, data_full, labels_full, test_data, mode=mode)
    # makeTreeFull(satisfy_lists,datasets=dataSet,myTree=myTree, labels_full=labels_full, parentClass=None, default='未知')
    # treePlotter.createPlot(myTree)#绘制图4.6

#--------------pre-pruning,annote here when compare the accuracy before and after post-pruning---------------


    print"labels=",json.dumps(labels,ensure_ascii=False)
    print"--------------------未剪枝决策树--------------------------------"
    labels_unpru=copy.deepcopy(labels)#因为labels使用后会改变,为了让它不改变,这里使用深拷贝,以便于labels保持原样,好方便让post-prune继续使用
    mode="unpru"#未剪枝
    satisfy_lists=[]
    myTree = createTree(train_data, labels_unpru, data_full, labels_full, test_data, mode=mode)
    makeTreeFull(satisfy_lists,datasets=dataSet,myTree=myTree, labels_full=labels_full, parentClass=None, default='未知')
    print"--------------------计算未剪枝决策树的accuracy--------------------"
    accuracy=precision_accuracy(myTree,test_data)
    print"myTree=",json.dumps(myTree,ensure_ascii=False)
    print"剪枝前accuracy=",accuracy
    print"labels=",json.dumps(labels,ensure_ascii=False)

    print"-----------------建立REP剪枝后的决策树------------------"
    mode="post"#后剪枝
    myTree_post = createTree(train_data, labels, data_full, labels_full, test_data, mode=mode)
    satisfy_lists=[]
    makeTreeFull(satisfy_lists,datasets=dataSet,myTree=myTree_post, labels_full=labels_full, parentClass=None, default='未知')
    print"---------------计算REP剪枝后的accuracy---------------------"
    accuracy=precision_accuracy(myTree_post,test_data)
    print"剪枝后accuracy=",accuracy



# # #-----------Draw ID3--------------------
    # treePlotter.createPlot(myTree)#绘制图4.5
    # treePlotter.createPlot(myTree_post)#绘制图4.7

