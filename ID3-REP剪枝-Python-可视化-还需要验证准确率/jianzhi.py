#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import pandas as pd
import math
import treePlotter
import json
import collections
import copy
from most_class_compute import most_class_computes
#读入数据




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
                sub_tree[label] = parentClass+"(虚)"#这里加了个虚,表示该叶子节点对应的训练集数据不存在
            # 否则设置为default
            else:
                sub_tree[label] = default

    # 当前层对应的分类列表
    current_class = []

    # 循环遍历一下子树，找到类别最多的那个，如果此时没有分类的话就是None
    # for sub_key in sub_tree.keys():
    #     if isinstance(sub_tree[sub_key], str):#如果是叶子节点
    #         current_class.append(sub_tree[sub_key])




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
    # 特征对应的所有可能的情况
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

def calcShannonEnt(dataSet): #计算熵
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
    retDataSet = []
    for featVec in dataSet:
        if featVec[axis] == value:
            reducedFeatVec = featVec[:axis]
            reducedFeatVec.extend(featVec[axis + 1:])
            retDataSet.append(reducedFeatVec)
    return retDataSet


def chooseBestFeatureToSplit(dataSet, labels):
    numFeatures = len(dataSet[0]) - 1
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
            prob = len(subDataSet) / float(len(dataSet))
            newEntropy += prob * calcShannonEnt(subDataSet)
        infoGain = baseEntropy - newEntropy
        if infoGain > bestInfoGain:
            bestInfoGain = infoGain
            bestFeature = i
    return bestFeature



def classify(inputTree, featLabels, testVec):
    firstStr = list(inputTree.keys())[0]
    secondDict = inputTree[firstStr]
    featIndex = featLabels.index(firstStr)
    for key in secondDict.keys():
        if testVec[featIndex] == key:
            if type(secondDict[key]).__name__ == 'dict':
                classLabel = classify(secondDict[key], featLabels, testVec)
            else:
                classLabel = secondDict[key]
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


def testing_feat(feat, train_data, test_data, labels):
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



def testingMajor(major, data_test):
    error = 0.0
    for i in range(len(data_test)):
        if major != data_test[i][-1]:
            error += 1
    # print 'major %d' % error
    return float(error) 


def testing(myTree,data_test,labels):
    error=0.0    
    for i in range(len(data_test)):
        if classify(myTree,labels,data_test[i])!=data_test[i][-1]:
            error+=1    
    print ('myTree %d' %error)
    return float(error)


#递归产生决策树**
import copy
from copy import deepcopy
def createTree(dataSet,labels,data_full,labels_full,test_data,mode):
    classList=[example[-1] for example in dataSet]
    if classList.count(classList[0])==len(classList):
       return classList[0]
    if len(dataSet[0])==1:
       return majorityCnt(classList)
    #labels_copy = labels
    labels_copy = copy.deepcopy(labels)
    bestFeat=chooseBestFeatureToSplit(dataSet,labels)
    bestFeatLabel=labels[bestFeat]

    if mode == "unpro" or mode == "post":
        myTree = {bestFeatLabel: {}}
    elif mode == "prev":
        if testing_feat(bestFeatLabel, dataSet, test_data, labels_copy) < testingMajor(majorityCnt(classList),test_data):
            myTree = {bestFeatLabel: {}}
        else:
            return majorityCnt(classList)
    featValues=[example[bestFeat] for example in dataSet]
    uniqueVals=set(featValues)

    if type(dataSet[0][bestFeat]).__name__ == 'unicode':
        currentlabel = labels_full.index(labels[bestFeat])
        featValuesFull = [example[currentlabel] for example in data_full]
        uniqueValsFull = set(featValuesFull)

    del (labels[bestFeat])

    for value in uniqueVals:
        subLabels = labels[:]
        if type(dataSet[0][bestFeat]).__name__ == 'unicode':
            uniqueValsFull.remove(value)

        myTree[bestFeatLabel][value] = createTree(splitDataSet(dataSet, bestFeat, value), subLabels, data_full, labels_full,splitDataSet(test_data, bestFeat, value), mode=mode)
    if type(dataSet[0][bestFeat]).__name__ == 'unicode':
        for value in uniqueValsFull:
            myTree[bestFeatLabel][value] = majorityCnt(classList)

    if mode == "post":
        if testing(myTree, test_data, labels_copy)> testingMajor(majorityCnt(classList), test_data):
            return majorityCnt(classList)
    return myTree



import json
if __name__ == "__main__":
    train_data, test_data,dataSet,labels,labels_full = load_data("watermelon_4_2_Chinese.csv")
    dataSet=train_data+test_data
    data_full = train_data[:]


    mode="unpro"#未剪枝
    # mode = "prev"#预剪枝
    mode="post"#REP后剪枝
    import treePlotter
    myTree = createTree(train_data, labels, data_full, labels_full, test_data, mode=mode)
    print"myTree=",myTree
    satisfy_lists=[]
    print ("satisfy_lists=",satisfy_lists)
    makeTreeFull(satisfy_lists,datasets=dataSet,myTree=myTree, labels_full=labels_full, parentClass=None, default='未知')
    treePlotter.createPlot(myTree)

