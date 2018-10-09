#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import pandas as pd
import math

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
        # 对连续型特征进行处理
        if type(featList[0]).__name__ == 'float' or type(featList[0]).__name__ == 'int':
            # 产生n-1个候选划分点
            sortfeatList = sorted(featList)
            splitList = []
            for j in range(len(sortfeatList) - 1):
                splitList.append((sortfeatList[j] + sortfeatList[j + 1]) / 2.0)

            bestSplitEntropy = 10000
            slen = len(splitList)
            # 求用第j个候选划分点划分时，得到的信息熵，并记录最佳划分点
            for j in range(slen):
                value = splitList[j]
                newEntropy = 0.0
                subDataSet0 = splitContinuousDataSet(dataSet, i, value, 0)
                subDataSet1 = splitContinuousDataSet(dataSet, i, value, 1)
                prob0 = len(subDataSet0) / float(len(dataSet))
                newEntropy += prob0 * calcShannonEnt(subDataSet0)
                prob1 = len(subDataSet1) / float(len(dataSet))
                newEntropy += prob1 * calcShannonEnt(subDataSet1)
                if newEntropy < bestSplitEntropy:
                    bestSplitEntropy = newEntropy
                    bestSplit = j
            # 用字典记录当前特征的最佳划分点
            bestSplitDict[labels[i]] = splitList[bestSplit]
            infoGain = baseEntropy - bestSplitEntropy
        # 对离散型特征进行处理
        else:
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
    # 若当前节点的最佳划分特征为连续特征，则将其以之前记录的划分点为界进行二值化处理
    # 即是否小于等于bestSplitValue
    if type(dataSet[0][bestFeature]).__name__ == 'float' or type(dataSet[0][bestFeature]).__name__ == 'int':
        bestSplitValue = bestSplitDict[labels[bestFeature]]
        labels[bestFeature] = labels[bestFeature] + '<=' + str(bestSplitValue)
        for i in range(shape(dataSet)[0]):
            if dataSet[i][bestFeature] <= bestSplitValue:
                dataSet[i][bestFeature] = 1
            else:
                dataSet[i][bestFeature] = 0
    return bestFeature



def classify(inputTree, featLabels, testVec):
    firstStr = list(inputTree.keys())[0]
    if u'<=' in firstStr:
        featvalue = float(firstStr.split(u"<=")[1])
        featkey = firstStr.split(u"<=")[0]
        secondDict = inputTree[firstStr]
        featIndex = featLabels.index(featkey)
        if testVec[featIndex] <= featvalue:
            judge = 1
        else:
            judge = 0
        for key in secondDict.keys():
            if judge == int(key):
                if type(secondDict[key]).__name__ == 'dict':
                    classLabel = classify(secondDict[key], featLabels, testVec)
                else:
                    classLabel = secondDict[key]
    else:
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
        if testing_feat(bestFeatLabel, dataSet, test_data, labels_copy) < testingMajor(majorityCnt(classList),
                                                                                       test_data):
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

        myTree[bestFeatLabel][value] = createTree(splitDataSet \
                                                      (dataSet, bestFeat, value), subLabels, data_full, labels_full,
                                                  splitDataSet \
                                                      (test_data, bestFeat, value), mode=mode)
    if type(dataSet[0][bestFeat]).__name__ == 'unicode':
        for value in uniqueValsFull:
            myTree[bestFeatLabel][value] = majorityCnt(classList)

    if mode == "post":
        if testing(myTree, test_data, labels_copy)> testingMajor(majorityCnt(classList), test_data):
            return majorityCnt(classList)
    return myTree

#读入数据

def load_data(file_name):
    with open(file_name, 'r') as f:
      df = pd.read_csv(f,sep=",")
      print(df)
      train_data = df.values[:11, 1:].tolist()
    print(train_data)
    test_data = df.values[11:, 1:].tolist()
    print(test_data)
    labels = df.columns.values[1:-1].tolist()
    #labels = df.columns.values
    print(labels)
    return train_data, test_data, labels


import json
if __name__ == "__main__":
    train_data, test_data, labels = load_data("watermelon_4_2_Chinese.csv")
    data_full = train_data[:]
    labels_full = labels[:]

    mode="unpro"
    mode = "prev"
    mode="post"
    import treePlotter
    myTree = createTree(train_data, labels, data_full, labels_full, test_data, mode=mode)
    treePlotter.createPlot(myTree)
    print(json.dumps(myTree, ensure_ascii=False, indent=4))

