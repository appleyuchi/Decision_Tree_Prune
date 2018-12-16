#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from CreateDT.Operation import CalcGini, splitDataSet, majorityCnt
from itertools import combinations

# 分离特征值函数，将特征值分为二分序列
def featureSplit(feature):
    """
    :param feature:特征值集合
    :return:特征二分序列
    """
    comList = []                              # 创建分序列组合
    for i in range(1, len(feature)):
        com = list(combinations(feature, i))  # 生成N-1个分序列，分别包含1～N-1个元素
        comList.extend(com)                   # 将所有分序列构成集合
    # print(comList)
    featureGroup = []
    N = len(comList)
    for i in range(N//2):                     # 分别将前后对称元素组成二分序列
        tem = [list(comList[i]), list(comList[N - 1 - i])]
        featureGroup.append(tem)              # 将所有二分序列组成集合
    return featureGroup

# 最小基尼系数为标准，选择最佳特征
def chooseBestFeature(dataSet):
    """
    :param dataSet:数据集
    :return:返回最佳特征，最佳特征值组合
    """
    lenFeature = len(dataSet[0])-1                               # 计算特征维度时去掉"类别"
    featureBase = 1.0                                            # 基尼系数比较基准值
    bestFeature = 0                                              # 最佳特征
    bestFeatureTuple = []                                        # 最佳特征二分组合
    for i in range(lenFeature):
        GiniD_A = 0.0                                            # 被特征i固定的基尼系数
        valueBase = 1.0                                          # 该特征下某个特征比较基准值
        feature = [example[i] for example in dataSet]            # 提取每组数据的第i个特征
        feature = set(feature)                                   # 得到第i个特征的种类个数
        # print('feature=', feature)
        if len(feature) == 1:                                    # 如果该特征只有一个特征值
            bestValueTuple = [feature]                                  # 不需要划分特征组合
            GiniD_A = 1.0
        else:
            featureGroup = featureSplit(feature)                 # 将该特征划分为二分序列组合
            for valueTuple in featureGroup:
                GiniD_a = 0.0                                    # 某个特征下某个特征值的基尼系数
                for value in valueTuple:
                    subData = splitDataSet(dataSet, i, value)    # 将特征i数据集按特征值value分类
                    p = float(len(subData))/float(len(dataSet))  # 计算特征值value在特征i集合的概率
                    # 在二分序列组合中，将每个特征值value概率与基尼系数乘积再相加，即为该组合基尼系数
                    GiniD_a += p*CalcGini(subData)
                # print('valueTuple=', valueTuple)
                # print('GiniD_a=', GiniD_a)
                if GiniD_a < valueBase:                          # 将基尼系数与比较基准量valueBase比较
                    valueBase = GiniD_a
                    GiniD_A = GiniD_a                            # 特征值组合最小的基尼系数，即为该特征的基尼系数
                    bestValueTuple = valueTuple                  # 记录该最佳特征值组合
        bestFeatureTuple.append(bestValueTuple)                  # 将所有特征值组合构成集合
        # print('bestFeatureTuple=', bestFeatureTuple)
        # print('GiniD_A=', GiniD_A)
        if GiniD_A < featureBase:                                # 根据基尼系数选出最佳特征
            featureBase = GiniD_A
            bestFeature = i
    return bestFeature, bestFeatureTuple[bestFeature]            # 返回最佳特征，以及该特征下最佳特征值组合

# 创建分类决策树
def createCARTTree(dataSet, labels):
    """
    :param dataSet: 数据集
    :param labels: 类别标签
    :return:决策树
    """
    classList = [example[-1] for example in dataSet]     # classList是数据集的所有类别

    # 情况1：若classList只有一类，则停止划分
    if classList.count(classList[0]) == len(classList):
        return classList[0]

    # 情况2：若完成所有的特征分类,返回个数最多的
    if len(dataSet[0]) == 1:
        return majorityCnt(classList)                    # 投票选出最多的特征

    # 情况3：classList有多类，开始进行分类
    feaBest, feature = chooseBestFeature(dataSet)        # 选择最佳分类特征
    # print('=====================================')
    # print('feaBest=', feaBest, 'feature', feature)
    newLabel = labels[feaBest]                           # 得到该特征的名称
    del(labels[feaBest])                                 # 删掉已分类的特征名称
    Tree = {newLabel: {}}                                # 创建一个多重字典，存储决策树分类结果
    for value in feature:
        subLabels = labels[:]                            # 构建特征名称子集合
        strValue = ','.join(value)
        # 利用递归函数不断创建分支，直到所有特征完成分类，分类结束
        subData = splitDataSet(dataSet, feaBest, value)
        # print('---------------------------------------')
        # print('value=', value)
        Tree[newLabel][strValue] = createCARTTree(subData, subLabels)
    return Tree
