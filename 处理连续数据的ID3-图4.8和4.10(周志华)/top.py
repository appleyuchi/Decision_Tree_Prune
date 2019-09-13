#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import collections
from math import log
import operator
import treePlotter
import pandas as pd

def createDataSet():
    """
    西瓜数据集3.0,绘制图4.8
    :return:
    """
    # dataSet = [
    #     # 1
    #     ['青绿', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', 0.697, 0.460, '好瓜'],
    #     # 2
    #     ['乌黑', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', 0.774, 0.376, '好瓜'],
    #     # 3
    #     ['乌黑', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', 0.634, 0.264, '好瓜'],
    #     # 4
    #     ['青绿', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', 0.608, 0.318, '好瓜'],
    #     # 5
    #     ['浅白', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', 0.556, 0.215, '好瓜'],#######3
    #     # 6
    #     ['青绿', '稍蜷', '浊响', '清晰', '稍凹', '软粘', 0.403, 0.237, '好瓜'],
    #     # 7
    #     ['乌黑', '稍蜷', '浊响', '稍糊', '稍凹', '软粘', 0.481, 0.149, '好瓜'],
    #     # 8
    #     ['乌黑', '稍蜷', '浊响', '清晰', '稍凹', '硬滑', 0.437, 0.211, '好瓜'],

    #     # ----------------------------------------------------
    #     # 9
    #     ['乌黑', '稍蜷', '沉闷', '稍糊', '稍凹', '硬滑', 0.666, 0.091, '坏瓜'],
    #     # 10
    #     ['青绿', '硬挺', '清脆', '清晰', '平坦', '软粘', 0.243, 0.267, '坏瓜'],
    #     # 11
    #     ['浅白', '硬挺', '清脆', '模糊', '平坦', '硬滑', 0.245, 0.057, '坏瓜'],##############
    #     # 12
    #     ['浅白', '蜷缩', '浊响', '模糊', '平坦', '软粘', 0.343, 0.099, '坏瓜'],###########
    #     # 13
    #     ['青绿', '稍蜷', '浊响', '稍糊', '凹陷', '硬滑', 0.639, 0.161, '坏瓜'],
    #     # 14
    #     ['浅白', '稍蜷', '沉闷', '稍糊', '凹陷', '硬滑', 0.657, 0.198, '坏瓜'],###########
    #     # 15
    #     ['乌黑', '稍蜷', '浊响', '清晰', '稍凹', '软粘', 0.360, 0.370, '坏瓜'],
    #     # 16
    #     ['浅白', '蜷缩', '浊响', '模糊', '平坦', '硬滑', 0.593, 0.042, '坏瓜'],###########3
    #     # 17
    #     ['青绿', '蜷缩', '沉闷', '稍糊', '稍凹', '硬滑', 0.719, 0.103, '坏瓜']
    # ]

#西瓜数据集2.0a
    dataSet = [
        # 1
        ['-', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
        # 2
        ['乌黑', '蜷缩', '沉闷', '清晰', '凹陷', '-', '好瓜'],
        # 3
        ['乌黑', '蜷缩', '-', '清晰', '凹陷', '硬滑', '好瓜'],
        # 4
        ['青绿', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
        # 5
        ['-', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
#--------------------------------------------
        # 6
        ['青绿', '稍蜷', '浊响', '清晰', '-', '软粘', '好瓜'],
        # 7
        ['乌黑', '稍蜷', '浊响', '稍糊', '稍凹', '软粘', '好瓜'],
        # 8
        ['乌黑', '稍蜷', '浊响', '-', '稍凹', '硬滑', '好瓜'],
        # 9
        ['乌黑', '-', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜'],
        # 10
        ['青绿', '硬挺', '清脆', '-', '平坦', '软粘', '坏瓜'],
#--------------------------------------------
        # 11
        ['浅白', '硬挺', '清脆', '模糊', '平坦', '-', '坏瓜'],
        # 12
        ['浅白', '蜷缩', '-', '模糊', '平坦', '软粘', '坏瓜'],
        # 13
        ['-', '稍蜷', '浊响', '稍糊', '凹陷', '硬滑', '坏瓜'],
        # 14
        ['浅白', '稍蜷', '沉闷', '稍糊', '凹陷', '硬滑', '坏瓜'],
        # 15
        ['乌黑', '稍蜷', '浊响', '清晰', '-', '软粘', '坏瓜'],
        # 16
        ['浅白', '蜷缩', '浊响', '模糊', '平坦', '硬滑', '坏瓜'],
        # 17
        ['青绿', '-', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜']
    ]



#下面是西瓜数据集3.0a,绘制图4.10

    # dataSet = [
    #     # 1
    #     [0.697, 0.460, '好瓜'],
    #     # 2
    #     [0.774, 0.376, '好瓜'],
    #     # 3
    #     [0.634, 0.264, '好瓜'],
    #     # 4
    #     [0.608, 0.318, '好瓜'],
    #     # 5
    #     [0.556, 0.215, '好瓜'],
    #     # 6
    #     [0.403, 0.237, '好瓜'],
    #     # 7
    #     [0.481, 0.149, '好瓜'],
    #     # 8
    #     [0.437, 0.211, '好瓜'],
    
    #     # ----------------------------------------------------
    #     # 9
    #     [0.666, 0.091, '坏瓜'],
    #     # 10
    #     [0.243, 0.267, '坏瓜'],
    #     # 11
    #     [0.245, 0.057, '坏瓜'],
    #     # 12
    #     [ 0.343, 0.099, '坏瓜'],
    #     # 13
    #     [ 0.639, 0.161, '坏瓜'],
    #     # 14
    #     [0.657, 0.198, '坏瓜'],
    #     # 15
    #     [0.360, 0.370, '坏瓜'],
    #     # 16
    #     [0.593, 0.042, '坏瓜'],
    #     # 17
    #     [ 0.719, 0.103, '坏瓜']
    # ]

#     # 西瓜数据集2.0 图4.4
#     dataSet = [
#         # 1
#         ['青绿', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
#         # 2
#         ['乌黑', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
#         # 3
#         ['乌黑', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
#         # 4
#         ['青绿', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
#         # 5
#         ['浅白', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
# #--------------------------------------------
#         # 6
#         ['青绿', '稍蜷', '浊响', '清晰', '稍凹', '软粘', '好瓜'],
#         # 7
#         ['乌黑', '稍蜷', '浊响', '稍糊', '稍凹', '软粘', '好瓜'],
#         # 8
#         ['乌黑', '稍蜷', '浊响', '清晰', '稍凹', '硬滑', '好瓜'],
#         # 9
#         ['乌黑', '稍蜷', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜'],
#         # 10
#         ['青绿', '硬挺', '清脆', '清晰', '平坦', '软粘', '坏瓜'],
# #--------------------------------------------
#         # 11
#         ['浅白', '硬挺', '清脆', '模糊', '平坦', '硬滑', '坏瓜'],
#         # 12
#         ['浅白', '蜷缩', '浊响', '模糊', '平坦', '软粘', '坏瓜'],
#         # 13
#         ['青绿', '稍蜷', '浊响', '稍糊', '凹陷', '硬滑', '坏瓜'],
#         # 14
#         ['浅白', '稍蜷', '沉闷', '稍糊', '凹陷', '硬滑', '坏瓜'],
#         # 15
#         ['乌黑', '稍蜷', '浊响', '清晰', '稍凹', '软粘', '坏瓜'],
#         # 16
#         ['浅白', '蜷缩', '浊响', '模糊', '平坦', '硬滑', '坏瓜'],
#         # 17
#         ['青绿', '蜷缩', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜']
#     ]
    
    # 西瓜数据集2.0和2.0a特征列表
    labels = ['色泽', '根蒂', '敲击', '纹理', '脐部', '触感']

    # 西瓜数据集3.0特征列表
    # labels = ['色泽', '根蒂', '敲击', '纹理', '脐部', '触感', '密度', '含糖率']
    # 西瓜数据集3.0a特征列表
    # labels = ['密度', '含糖率']


    # 特征对应的所有可能的情况
    labels_full = {}

    for i in range(len(labels)):
        labelList = [example[i] for example in dataSet]
        uniqueLabel = set(labelList)
        labels_full[labels[i]] = uniqueLabel
    print("--------------------------------------")
    for item in labels_full:
        print("item=",unicode(item))
    print("--------------------------------------")
    print("len(labels_full)=",len(labels_full))
    print("len(labels)=",len(labels))

    return dataSet, labels, labels_full


def calcShannonEnt(dataSet):
    """
    计算给定数据集的信息熵(香农熵)
    :param dataSet:
    :return:
    """
    # 计算出数据集的总数
    numEntries = len(dataSet)

    # 用来统计标签
    labelCounts = collections.defaultdict(int)

    # 循环整个数据集，得到数据的分类标签
    for featVec in dataSet:
        # 得到当前的标签
        currentLabel = featVec[-1]

        # 将对应的标签值加一
        labelCounts[currentLabel] += 1

    # 默认的信息熵
    shannonEnt = 0.0

    for key in labelCounts:
        # 计算出当前分类标签占总标签的比例数
        prob = float(labelCounts[key]) / numEntries

        # 以2为底求对数
        shannonEnt -= prob * log(prob, 2)

    return shannonEnt


def splitDataSetForSeries(dataSet, axis, value):
    print("进入splitDataSetForSeries,axis=",axis)
    """
    按照给定的数值，将数据集分为不大于和大于两部分
    :param dataSet: 要划分的数据集
    :param i: 特征值所在的下标
    :param value: 划分值
    :return:
    """
    # 用来保存不大于划分值的集合
    eltDataSet = []
    # 用来保存大于划分值的集合
    gtDataSet = []
    # 进行划分，保留该特征值
    print("axis=",axis)
    for feat in dataSet:
        if feat[axis] <= value:
            eltDataSet.append(feat)
        else:
            gtDataSet.append(feat)

    return eltDataSet, gtDataSet


def splitDataSet(dataSet, axis, value):
    """
    按照给定的特征值，将数据集划分
    :param dataSet: 数据集
    :param axis: 给定特征值的坐标
    :param value: 给定特征值满足的条件，只有给定特征值等于这个value的时候才会返回
    :return:
    """
    # 创建一个新的列表，防止对原来的列表进行修改
    retDataSet = []

    # 遍历整个数据集
    for featVec in dataSet:
        # 如果给定特征值等于想要的特征值
        if featVec[axis] == value:
            # 将该特征值前面的内容保存起来
            reducedFeatVec = featVec[:axis]
            # 将该特征值后面的内容保存起来，所以将给定特征值给去掉了
            reducedFeatVec.extend(featVec[axis + 1:])
            # 添加到返回列表中
            retDataSet.append(reducedFeatVec)

    return retDataSet




#這個函數是在尋找最佳分割點,使得熵增益最大.
def calcInfoGainForSeries(dataSet, i, baseEntropy):
    print("进入calcInfoGainForSeries,i=",i)
    """
    计算连续值的信息增益
    :param dataSet:整个数据集
    :param i: 对应的特征值下标
    :param baseEntropy: 基础信息熵
    :return: 返回一个信息增益值，和当前的划分点
    """

    # 记录最大的信息增益
    maxInfoGain = 0.0

    # 最好的划分点
    bestMid = -1

    # 得到数据集中所有的当前特征值列表
    featList = [example[i] for example in dataSet]

    # 得到分类列表
    classList = [example[-1] for example in dataSet]

    dictList = dict(zip(featList, classList))

    # 将其从小到大排序，按照连续值的大小排列
    sortedFeatList = sorted(dictList.items(), key=operator.itemgetter(0))

    # 计算连续值有多少个
    numberForFeatList = len(sortedFeatList)

    # midFeatList = [round((sortedFeatList[i][0] + sortedFeatList[i+1][0])/2.0, 3)for i in range(numberForFeatList - 1)]
    midFeatList = [round((sortedFeatList[k][0] + sortedFeatList[k+1][0])/2.0, 3)for k in range(numberForFeatList - 1)]
    #上面一句代码注意:
    # 由于作者在这里使用的是python3.x的语法,所以原有代码中列表推导式中的i会干扰calcInfoGainForSeries(dataSet, i, baseEntropy)中的i
    #所以为了避免python解释器混淆,上面的i->k

    # 计算出各个划分点信息增益
    for mid in midFeatList:
        # 将连续值划分为不大于当前划分点和大于当前划分点两部分
        eltDataSet, gtDataSet = splitDataSetForSeries(dataSet, i, mid)

        # 计算两部分的特征值熵和权重的乘积之和
        newEntropy = float(len(eltDataSet))/float(len(sortedFeatList))*float(calcShannonEnt(eltDataSet)) + float(len(gtDataSet))/float(len(sortedFeatList))*float(calcShannonEnt(gtDataSet))

        # 计算出信息增益
        infoGain = baseEntropy - newEntropy
        # print('当前划分值为：' + str(mid) + '，此时的信息增益为：' + str(infoGain))
        if infoGain > maxInfoGain:
            bestMid = mid
            maxInfoGain = infoGain

    return maxInfoGain, bestMid


def calcInfoGain(dataSet ,featList, i, baseEntropy):
    """
    计算信息增益
    :param dataSet: 数据集
    :param featList: 当前特征列表
    :param i: 当前特征值下标
    :param baseEntropy: 基础信息熵
    :return:
    """
    # 将当前特征唯一化，也就是说当前特征值中共有多少种
    uniqueVals = set(featList)

    # 新的熵，代表当前特征值的熵
    newEntropy = 0.0

    # 遍历现在有的特征的可能性
    for value in uniqueVals:
        # 在全部数据集的当前特征位置上，找到该特征值等于当前值的集合
        subDataSet = splitDataSet(dataSet=dataSet, axis=i, value=value)
        # 计算出权重
        prob = float(len(subDataSet)) / float(len(dataSet))
        # 计算出当前特征值的熵
        newEntropy += prob * calcShannonEnt(subDataSet)

    # 计算出“信息增益”
    infoGain = baseEntropy - newEntropy

    return infoGain


def chooseBestFeatureToSplit(dataSet, labels):
    """
    选择最好的数据集划分特征，根据信息增益值来计算，可处理连续值
    :param dataSet:
    :return:
    """
    # 得到数据的特征值总数
    numFeatures = len(dataSet[0]) - 1

    # 计算出基础信息熵
    baseEntropy = calcShannonEnt(dataSet)

    # 基础信息增益为0.0
    bestInfoGain = 0.0

    # 最好的特征值
    bestFeature = -1

    # 标记当前最好的特征值是不是连续值
    flagSeries = 0

    # 如果是连续值的话，用来记录连续值的划分点
    bestSeriesMid = 0.0

    # 对每个特征值进行求信息熵
    for i in range(numFeatures):
        print("i=",i)

        # 得到数据集中所有的当前特征值列表
        featList = [example[i] for example in dataSet]

        if isinstance(featList[0], str):
            infoGain = calcInfoGain(dataSet, featList, i, baseEntropy)
        else:
            # print('当前划分属性为：' + str(labels[i]))
            infoGain, bestMid = calcInfoGainForSeries(dataSet, i, baseEntropy)

        # print('当前特征值为：' + labels[i] + '，对应的信息增益值为：' + str(infoGain))

        # 如果当前的信息增益比原来的大
        if infoGain > bestInfoGain:
            # 最好的信息增益
            bestInfoGain = infoGain
            # 新的最好的用来划分的特征值
            bestFeature = i

            flagSeries = 0
            if not isinstance(dataSet[0][bestFeature], str):
                flagSeries = 1
                bestSeriesMid = bestMid

    # print('信息增益最大的特征为：' + labels[bestFeature])
    if flagSeries:
        return bestFeature, bestSeriesMid
    else:
        return bestFeature


def majorityCnt(classList):
    """
    找到次数最多的类别标签
    :param classList:
    :return:
    """
    # 用来统计标签的票数
    classCount = collections.defaultdict(int)

    # 遍历所有的标签类别
    for vote in classList:
        classCount[vote] += 1

    # 从大到小排序
    sortedClassCount = sorted(classCount.items(), key=operator.itemgetter(1), reverse=True)

    # 返回次数最多的标签
    return sortedClassCount[0][0]


def createTree(dataSet, labels):
    """
    创建决策树
    :param dataSet: 数据集
    :param labels: 特征标签
    :return:
    """
    # 拿到所有数据集的分类标签
    classList = [example[-1] for example in dataSet]

    # 统计第一个标签出现的次数，与总标签个数比较，如果相等则说明当前列表中全部都是一种标签，此时停止划分
    if classList.count(classList[0]) == len(classList):
        return classList[0]

    # 计算第一行有多少个数据，如果只有一个的话说明所有的特征属性都遍历完了，剩下的一个就是类别标签
    if len(dataSet[0]) == 1:
        # 返回剩下标签中出现次数较多的那个
        return majorityCnt(classList)

    # 选择最好的划分特征，得到该特征的下标
    bestFeat = chooseBestFeatureToSplit(dataSet=dataSet, labels=labels)

    # 得到最好特征的名称
    bestFeatLabel = ''

    # 记录此刻是连续值还是离散值,1连续，2离散
    flagSeries = 0

    # 如果是连续值，记录连续值的划分点
    midSeries = 0.0

    # 如果是元组的话，说明此时是连续值
    if isinstance(bestFeat, tuple):
        # 重新修改分叉点信息
        bestFeatLabel = str(labels[bestFeat[0]]) + '小于' + str(bestFeat[1]) + '?'
        # 得到当前的划分点
        midSeries = bestFeat[1]
        # 得到下标值
        bestFeat = bestFeat[0]
        # 连续值标志
        flagSeries = 1
    else:
        # 得到分叉点信息
        bestFeatLabel = labels[bestFeat]
        # 离散值标志
        flagSeries = 0

    # 使用一个字典来存储树结构，分叉处为划分的特征名称
    myTree = {bestFeatLabel: {}}

    # 得到当前特征标签的所有可能值
    featValues = [example[bestFeat] for example in dataSet]

    # 连续值处理
    if flagSeries:
        # 将连续值划分为不大于当前划分点和大于当前划分点两部分
        eltDataSet, gtDataSet = splitDataSetForSeries(dataSet, bestFeat, midSeries)
        # 得到剩下的特征标签
        subLabels = labels[:]
        # 递归处理小于划分点的子树
        subTree = createTree(eltDataSet, subLabels)
        myTree[bestFeatLabel]['小于'] = subTree

        # 递归处理大于当前划分点的子树
        subTree = createTree(gtDataSet, subLabels)
        myTree[bestFeatLabel]['大于'] = subTree

        return myTree

    # 离散值处理
    else:
        # 将本次划分的特征值从列表中删除掉
        del (labels[bestFeat])
        # 唯一化，去掉重复的特征值
        uniqueVals = set(featValues)
        # 遍历所有的特征值
        for value in uniqueVals:
            # 得到剩下的特征标签
            subLabels = labels[:]
            # 递归调用，将数据集中该特征等于当前特征值的所有数据划分到当前节点下，递归调用时需要先将当前的特征去除掉
            subTree = createTree(splitDataSet(dataSet=dataSet, axis=bestFeat, value=value), subLabels)
            # 将子树归到分叉处下
            myTree[bestFeatLabel][value] = subTree
        return myTree


if __name__ == '__main__':
    dataSet, labels, labels_full = createDataSet()
    myTree = createTree(dataSet, labels)
    print(myTree)
    treePlotter.createPlot(myTree)