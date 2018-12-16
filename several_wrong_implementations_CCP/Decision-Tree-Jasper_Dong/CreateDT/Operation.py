#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from math import log
import operator
import random

# 计算数据集的香农熵
def CalcEntropy(dataSet):
    """
    :param dataSet:数据集
    :return:返回香农信息熵
    """
    lenDataSet = len(dataSet)                          # 类别数量
    labelCount = {}                                    # 统计类别标签的数量
    Hc = 0.0                                           # 按类别计算熵
    for data in dataSet:
        currentLabel = data[-1]                        # 获取最后一列数据,即类别
        if currentLabel not in labelCount.keys():      # 若不存在该类别，即创建该标签
            labelCount[currentLabel] = 0
        labelCount[currentLabel] += 1                  # 若存在该类型，递增类别标签的值
    for key in labelCount:
        pc = float(labelCount[key])/float(lenDataSet)  # 计算标签的概率
        Hc -= pc*log(pc, 2)                            # 计算信息熵
    return Hc

# 计算数据集的基尼系数，即gini
def CalcGini(dataSet):
    """
    :param dataSet:数据集
    :return:返回基尼系数
    """
    lenDataSet = len(dataSet)
    labelCount = {}
    for data in dataSet:
        currentLabel = data[-1]
        if currentLabel not in labelCount.keys():
            labelCount[currentLabel] = 0
        labelCount[currentLabel] += 1
    Gini = 1.0
    for key in labelCount:           # 根据公式计算gini
        px = float(labelCount[key])/float(lenDataSet)
        Gini -= px*px
    return Gini

# 划分数据集，取出该label特征取值为value的所有样本
def splitDataSet(dataSet, feature, value):
    """
    :param dataSet:数据集
    :param feature:需要提取的特征
    :param value:相应特征的具体值
    :return:划分后的数据集
    """

    subDataSet = []
    valueList = []
    # print('value=', value, type(value))
    if type(value).__name__ == 'str':               # 如果特征值为字符，说明分离一个特征值
        valueList.append(value)                     # 需要把字符串放在列表中
    else:                                           # 否则特征值为列表，说明分裂多个特征
        valueList = value
    # print('valueList=', valueList, type(valueList))
    for data in dataSet:
        for value in valueList:
            if data[feature] == value:
                subData = data[:feature]            # 取第0到第axis-1进subData;
                subData.extend(data[feature + 1:])  # 取第axis+1之后的数进subData
                # 相当于把label特征取值剔除，将其他特征取值输出
                subDataSet.append(subData)          # 将每个符合条件的特征列表，组成列表集合
    return subDataSet

# 投票表决,缺失取值的类别选择多数
def majorityCnt(classList):
    """
    :param classList:需要投票的类别集
    :return:数量最多的类别
    """
    classCount = {}
    for vote in classList:
        if vote not in classCount.keys():
            classCount[vote] = 0
        classCount[vote] += 1           # 统计每种类别的个数
    sortedClassCount = sorted(classCount.items(), key=operator.itemgetter(1), reverse=True)
    # items所有数据，key排序第1个域的值，reverse降序或升序
    return sortedClassCount[0][0]

# 生成训练集和测试集
def createSet(dataSet):
    """
    :param dataSet:数据集
    :return:训练集和测试集
    """
    trainSet = []
    testSet = []
    N = len(dataSet)
    number = int(0.8*N)
    sample = []
    for i in range(N):
        sample.append(i)
    dataList = random.sample(sample, number)  # 随机抽取样本
    for i in range(N):
        if i in dataList:
            trainSet.append(list(dataSet[i]))
        else:
            testSet.append(list(dataSet[i]))
    return trainSet, testSet

# 使用决策树执行分类，返回分类结果
def classify(tree, label, testData):
    """
    :param tree:决策树
    :param label:类别标签
    :param testData:测试数据
    :return:分类结果
    """
    firstFeat = list(tree.keys())[0]           # 取出tree的第一个键名
    secondDict = tree[firstFeat]               # 取出tree第一个键值，即tree的第二个字典（包含关系）
    labelIndex = label.index(firstFeat)        # 得到第一个特征firstFeat在标签label中的索引
    classLabel = ['uncertain']
    for keys in secondDict.keys():             # 遍历第二个字典的键
        # print('key=', keys, type(keys))
        # print('secondDict=', secondDict)
        items = keys.split(',')                # 如果该键包含多个特征值，那么进行分离
        # print('items=', items)
        if testData[labelIndex] in items:      # 如果测试数据特征值与字典的键相等时
            # 如果第二个字典的值还是一个字典，说明分类还没结束，递归执行classify函数
            if type(secondDict[keys]).__name__ == 'dict':
                classLabel = classify(secondDict[keys], label, testData)
            else:
                classLabel = secondDict[keys]  # 最后将得到的分类值赋给classLabel输出
    return classLabel

# 计算决策树的准确性
def testAccuracy(Tree, labels, dataSet):
    """
    :param Tree:生成的决策树
    :param labels:特征类别属性
    :param dataSet:数据集
    :return:决策树精确度
    """
    if type(Tree).__name__ != 'dict':
        return('This Tree is incomplete')
    else:
        accuracy = 0.0
        N = len(dataSet)
        for i in range(len(dataSet)):         # 游历数据集每个元素，按照决策树分类，与样本类别比较
            # print('classify=', classify(Tree, labels, dataSet[i]))
            if classify(Tree, labels, dataSet[i]) == dataSet[i][-1]:
                accuracy += 1                    # 如果不一致，错误加1
        return str(round(accuracy*100 / N, 2))+'%'

