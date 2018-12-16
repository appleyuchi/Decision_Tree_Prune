import copy
import numpy as np

from CreateDT.C4_5 import createC4_5Tree
from CreateDT.Operation import majorityCnt, splitDataSet, classify
from CreateDT.PlotDT import getNumLeaf, getTreeDepth

# 计算非叶节点的误差
def nodeError(dataSet):
    """
    :param dataSet:数据集
    :return:误差
    """
    error = 0.0
    classList = [example[-1] for example in dataSet]
    majorClass = majorityCnt(classList)  # 找到数量最多的类别
    for i in range(len(dataSet)):        # 游历数据集每个元素，找出正确样本个数
        if dataSet[i][-1] != majorClass:
            error += 1                   # 如果不一致，错误加1
    return float(error)

# 计算叶节点的误差
def leafError(Tree, labels, dataSet):
    """
    :param Tree:生成的决策树
    :param labels:特征类别属性
    :param dataSet:数据集
    :return:误差
    """
    error = 0.0
    for i in range(len(dataSet)):         # 游历数据集每个元素，按照决策树分类，与样本类别比较
        # print('classify=', classify(Tree, labels, dataSet[i]))
        if classify(Tree, labels, dataSet[i]) != dataSet[i][-1]:
            error += 1                    # 如果不一致，错误加1
    return float(error)

# 将决策树按照PEP规则进行剪枝
def PostPruning_PEP(Tree, labels, dataSet):
    """
    :param Tree:预处理的决策树
    :param labels:特征类别属性
    :param dataSet:数据集
    :return:剪枝后的决策树
    """
    classList = [example[-1] for example in dataSet]
    majorClass = majorityCnt(classList)              # 找到数量最多的类别
    et = nodeError(dataSet) + 1/2                    # 计算非叶节点t误差
    Nt = getNumLeaf(Tree)                            # 子树Tt叶节点数目
    eTt = leafError(Tree, labels, dataSet) + Nt/2    # 子树Tt所有叶节点误差
    nt = len(dataSet)                                # 节点t训练实例数目
    if nt > eTt:
        SeTt = np.sqrt(eTt * (nt - eTt) / nt)     # 子树Tt总误差
    else:
        SeTt = 0
    # print('================================')
    # print('Tree=', Tree)
    # print('et=', et)
    # print('eTt=', eTt)
    # print('Nt=', Nt)
    # print('nt=', nt)
    #print('SeTt=', SeTt)
    # print('eTt + SeTt=', eTt + SeTt)
    if et < eTt + SeTt:                              # 若节点t误差小于子树Tt误差
        return majorClass                            # 则进行剪枝，直接返回最大类
    firstFeat = list(Tree.keys())[0]                 # 取出tree的第一个键名
    secondDict = Tree[firstFeat]                     # 取出tree第一个键值
    labelIndex = labels.index(firstFeat)             # 找到键名在特征属性的索引值
    # print('firstFeat=', firstFeat)
    # print('secondDict=', secondDict)
    subLabels = labels[:labelIndex]                  # 剔除预处理的键名
    subLabels.extend(labels[labelIndex + 1:])
    for keys in secondDict.keys():                   # 遍历第二个字典的键
        if type(secondDict[keys]).__name__ == 'dict':
            items = keys.split(',')                  # 如果该键包含多个特征值，那么进行分离
            # print('items=', items)
            subDataSet = splitDataSet(dataSet, labelIndex, items)  # 划分数据集
            secondDict[keys] = PostPruning_PEP(secondDict[keys], subLabels, subDataSet)
    return Tree

# 计算非叶节点的误差(MEP)
def nodeLapError(dataSet, k):
    """
    :param dataSet:数据集
    :return:误差
    """
    nt = len(dataSet)                      # 样本总数
    nct = nt - nodeError(dataSet)          # 主类的样本数目
    Ert = (nt - nct + (k - 1)) / (nt + k)  # 按公式计算误差
    return Ert

# 计算非叶节点下子树的误差加权和
def branLapError(branch, dataSet, labelIndex, k):
    """
    :param branch:子树分支集合
    :param dataSet:数据集
    :param labelIndex:类索引值
    :return:误差加权和
    """
    ErTt = []                    # 每个分支误差初值
    nTt = []                     # 每个分支样本总数初值
    for keys in branch.keys():   # 计算每个分支的误差和数目
        items = keys.split(',')  # 如果该键包含多个特征值，那么进行分离
        subDataSet = splitDataSet(dataSet, labelIndex, items)
        # for data in subDataSet:
        #     print(data)
        ErTt.append(nodeLapError(subDataSet, k))
        nTt.append(len(subDataSet))
    # print('ErTt=', ErTt)
    Sum_ErTt = np.dot(ErTt, nTt) / sum(nTt)  # 计算所有分支误差的加权和
    return Sum_ErTt

# 将决策树按照MEP规则进行剪枝
def PostPruning_MEP(Tree, labels, dataSet, k):
    """
    :param Tree:预处理的决策树
    :param labels:特征类别属性
    :param dataSet:数据集
    :param k:数据集的类别数
    :return:剪枝后的决策树
    """
    classList = [example[-1] for example in dataSet]
    majorClass = majorityCnt(classList)              # 找到数量最多的类别
    firstFeat = list(Tree.keys())[0]                 # 取出tree的第一个键名
    secondDict = Tree[firstFeat]                     # 取出tree第一个键值
    labelIndex = labels.index(firstFeat)             # 找到键名在特征属性的索引值
    subLabels = labels[:labelIndex]                  # 剔除预处理的键名
    subLabels.extend(labels[labelIndex + 1:])
    # print('======================')
    # print('Tree=', Tree)
    # print('firstFeat=', firstFeat)
    for keys in secondDict.keys():                   # 遍历第二个字典的键
        # print('keys=', keys)
        if type(secondDict[keys]).__name__ == 'dict':
            items = keys.split(',')                  # 如果该键包含多个特征值，那么进行分离
            # print('items=', items)
            subDataSet = splitDataSet(dataSet, labelIndex, items)  # 划分数据集
            secondDict[keys] = PostPruning_MEP(secondDict[keys], subLabels, subDataSet, k)
    # print('--------------------')
    Ert = nodeLapError(dataSet, k)                                 # 计算非叶节点的误差
    Sum_ErTt = branLapError(secondDict, dataSet, labelIndex, k)    # 计算节点t分支的误差加权和
    # print('Ert=', Ert)
    # print('Sum_ErTt=', Sum_ErTt)
    if Ert > Sum_ErTt:                               # 如果节点误差大于分支误差和，则子树保留
        return Tree
    else:                                            # 否则进行剪枝，返回主类
        return majorClass

# 计算非叶节点误差增加率
def calErrorRatio(Tree, labels, dataSet, NT, infoSet):
    """
    :param Tree:决策树
    :param labels:特征类别属性
    :param dataSet:数据集
    :param NT:数据集总样本数目
    :param infoSet:所有节点的信息总集合
    :return:各个节点的信息集：
                            包括：子树，节点数目，误差增加率和子树分类前特征
    """
    firstFeat = list(Tree.keys())[0]                # 取出tree的第一个键名
    secondDict = Tree[firstFeat]                    # 取出tree第一个键值
    labelIndex = labels.index(firstFeat)            # 找到键名在特征属性的索引值
    subLabels = labels[:labelIndex]                 # 剔除预处理的键名
    subLabels.extend(labels[labelIndex + 1:])
    for keys in secondDict.keys():                  # 遍历第二个字典的键
        if type(secondDict[keys]).__name__ == 'dict':
            items = keys.split(',')                 # 如果该键包含多个特征值，那么进行分离
            subDataSet = splitDataSet(dataSet, labelIndex, items)  # 划分数据集
            info, infoSet = calErrorRatio(secondDict[keys], subLabels, subDataSet, NT, infoSet)
            info.setdefault('keys', keys)           # 在节点信息集中，增加分类前特征
            infoSet.append(info)
    # print('=============================')
    # print('Tree=', Tree)
    # print('firstFeat=', firstFeat)
    # print('secondDict=', secondDict)
    Rt = nodeError(dataSet) / NT                    # 计算节点误差率
    RTt = leafError(Tree, labels, dataSet) / NT     # 计算子树误差率
    Nt = getNumLeaf(Tree)                           # 计算叶节点数目
    if Nt == 1:
        a = 2.0
    else:
        a = (Rt - RTt) / (Nt - 1)                   # 计算误差增加率
    info = {'Tree': Tree, 'NumLeaf': Nt, 'a': a}    # 构建节点信息集
    # print('info=', info)
    return info, infoSet

# 根据误差增加率，剪掉支树
def prunBranch(Tree, labels, infoBran, dataSet):
    """
    :param Tree:决策树
    :param labels:特征类别属性
    :param infoBran:需剪掉的支树信息集
    :param dataSet:数据集
    :return:剪枝后的决策树
    """
    firstFeat = list(Tree.keys())[0]                # 取出tree的第一个键名
    secondDict = Tree[firstFeat]                    # 取出tree第一个键值
    labelIndex = labels.index(firstFeat)            # 找到键名在特征属性的索引值
    subLabels = labels[:labelIndex]                 # 剔除预处理的键名
    subLabels.extend(labels[labelIndex + 1:])
    for keys in secondDict.keys():                  # 遍历第二个字典的键
        items = keys.split(',')                     # 如果该键包含多个特征值，那么进行分离
        subDataSet = splitDataSet(dataSet, labelIndex, items)  # 划分数据集
        classList = [example[-1] for example in subDataSet]
        majorClass = majorityCnt(classList)                    # 找到数量最多的类别
        # 如果当前支树分类前特征和支树都和预处理相同，则把该支树剪掉
        if keys == infoBran['keys'] and secondDict[keys] == infoBran['Tree']:
            secondDict[keys] = majorClass                      # 剪掉支树，即返回最大类
            return Tree
        elif type(secondDict[keys]).__name__ == 'dict':        # 如果不相同，继续向下寻找
            secondDict[keys] = prunBranch(secondDict[keys], subLabels, infoBran, subDataSet)
    return Tree

# 对决策树进行剪枝，生成剪枝后的子树
def createSubTree(Tree, labels, dataSet):
    """
    :param Tree:决策树
    :param labels:特征类别属性
    :param dataSet:数据集
    :return:剪枝后的决策树
    """
    NT = len(dataSet)                       # 计算数据集长度
    infoSet = []                            # 构建节点信息总集合
    calErrorRatio(Tree, labels, dataSet, NT, infoSet)  # 计算误差增加率，并生成信息集合
    baseValue = 1.0                         # a的比较基准值
    for i in range(len(infoSet)):
        # print(infoSet[i])
        if infoSet[i]['a'] < baseValue:     # 先判断误差增加率a，选择最小的支树
            baseValue = infoSet[i]['a']
            bestNode = i
        elif infoSet[i]['a'] == baseValue:  # 如果a值相同，判断节点数目
            if infoSet[i]['NumLeaf'] > infoSet[bestNode]['NumLeaf']:
                bestNode = i                # 选择节点数目最大的子树
    infoBran = infoSet[bestNode]
    # print('infoBran=', infoBran)          # 剪掉子树，生成剪枝后的决策树
    subTree = prunBranch(Tree, labels, infoBran, dataSet)
    # print('subTree=', subTree)
    return subTree

# 将决策树按照CCP规则进行剪枝
def PostPruning_CCP(Tree, labels, dataSet, testSet):#注意，这个函数不是在剪枝，而是在选择最佳树
    """
    :param Tree:决策树
    :param labels:特征类别属性
    :param dataSet:数据集
    :return:剪枝后的决策树
    """
    TreeSet = [Tree]                         # 创建子树序列集
    i = 0
    while getTreeDepth(TreeSet[i]) > 1:      # 如果树的深度大于1，即非根节点
        preTree = copy.deepcopy(TreeSet[i])  # 将预剪枝树拷贝
        subTree = createSubTree(preTree, labels, dataSet)  # 生成剪枝后的子树
        # print('subTree=', subTree)
        TreeSet.append(subTree)              # 构成子树序列
        i += 1
    # for tree in TreeSet:
    #     print(tree)
    baseValue = len(testSet[0])              # 误差平均值的比较基准值
    bestTree = 0
    for i in range(1, len(TreeSet)):
        error = leafError(TreeSet[i], labels, testSet)  # 单个测试集的误差
        # print('--------------------')
        # print('TreeSet=', TreeSet[i])
        # print('evalValue=', evalValue)
        if error < baseValue:                # 利用交叉验证，选择最优决策树
            baseValue = error
            bestTree = i
    return TreeSet[bestTree]

# 计算非叶节点的误差(IMEP)
def newNodeLapError(dataSet, m):
    """
    :param dataSet:数据集
    :param m:先验概率对后验概率的影响因子
    :return:误差
    """
    nt = len(dataSet)  # 样本总数
    nct = nt - nodeError(dataSet)  # 主类的样本数目
    Pai = nct / nt
    Ert = (nt - nct + m * (1 - Pai)) / (nt + m)  # 按公式计算误差
    return Ert

# 计算非叶节点下子树的误差加权和(IMEP)
def newBranLapError(branch, dataSet, labelIndex, m):
    """
    :param branch:子树分支集合
    :param dataSet:数据集
    :param labelIndex:类索引值
    :param m:先验概率对后验概率的影响因子
    :return:误差加权和
    """
    ErTt = []                    # 每个分支误差初值
    nTt = []                     # 每个分支样本总数初值
    for keys in branch.keys():   # 计算每个分支的误差和数目
        items = keys.split(',')  # 如果该键包含多个特征值，那么进行分离
        subDataSet = splitDataSet(dataSet, labelIndex, items)
        # for data in subDataSet:
        #     print(data)
        ErTt.append(newNodeLapError(subDataSet, m))
        nTt.append(len(subDataSet))
    # print('ErTt=', ErTt)
    Sum_ErTt = np.dot(ErTt, nTt) / sum(nTt)  # 计算所有分支误差的加权和
    return Sum_ErTt

# 将决策树按照IMEP规则进行剪枝
def PostPruning_IMEP(Tree, labels, dataSet, m):
    """
    :param Tree:预处理的决策树
    :param labels:特征类别属性
    :param dataSet:数据集
    :param m:先验概率对后验概率的影响因子
    :return:剪枝后的决策树
    """
    classList = [example[-1] for example in dataSet]
    majorClass = majorityCnt(classList)              # 找到数量最多的类别
    firstFeat = list(Tree.keys())[0]                 # 取出tree的第一个键名
    secondDict = Tree[firstFeat]                     # 取出tree第一个键值
    labelIndex = labels.index(firstFeat)             # 找到键名在特征属性的索引值
    subLabels = labels[:labelIndex]                  # 剔除预处理的键名
    subLabels.extend(labels[labelIndex + 1:])
    # print('======================')
    # print('Tree=', Tree)
    # print('firstFeat=', firstFeat)
    for keys in secondDict.keys():                   # 遍历第二个字典的键
        # print('keys=', keys)
        if type(secondDict[keys]).__name__ == 'dict':
            items = keys.split(',')                  # 如果该键包含多个特征值，那么进行分离
            # print('items=', items)
            subDataSet = splitDataSet(dataSet, labelIndex, items)  # 划分数据集
            secondDict[keys] = PostPruning_IMEP(secondDict[keys], subLabels, subDataSet, m)
    # print('--------------------')
    Ert = newNodeLapError(dataSet, m)  # 计算非叶节点的误差
    Sum_ErTt = newBranLapError(secondDict, dataSet, labelIndex, m)  # 计算节点t分支的误差加权和
    # print('Ert=', Ert)
    # print('Sum_ErTt=', Sum_ErTt)
    if Ert > Sum_ErTt:                               # 如果节点误差大于分支误差和，则子树保留
        return Tree
    else:                                            # 否则进行剪枝，返回主类
        return majorClass

# 利用V折交叉验证，选择最优模型
def crossValidation(trainSet, testSet, labels, m):
    """
    :param trainSet:训练集
    :param testSet:测试集
    :param labels:特征类别属性
    :param m:先验概率对后验概率的影响因子
    :return:该模型的性能指标
    """
    v = len(trainSet)
    TreeSet = []
    for i in range(v):     # 分别进行K次生成决策树、决策树剪枝的过程
        temData = tuple(trainSet[i])
        Tree = createC4_5Tree(list(temData), list(labels))
        purnTree = PostPruning_IMEP(Tree, labels, trainSet[i], m)
        # print('purnTree=', purnTree)
        TreeSet.append(purnTree)
    errorSum = 0.0
    for i in range(0, v):  # 将剪枝后的决策树进行错误估计
        error = leafError(TreeSet[i], labels, testSet[i])
        errorSum += error
    return errorSum / v

# 使用IMEP算法，并利用交叉验证，选择最优参数m
def IMEP_CrossValidation(dataSet, labels):
    """
    :param dataSet:数据集
    :param labels:特征类别属性
    :return:最优决策树
    """
    v = 10                     # 数据集分组值K，默认为5或10
    N = len(dataSet)           # 数据集长度
    Nk = round(N / v)          # 计算每个分组的长度
    # print('Nk=', Nk)
    trainSet = []              # 构建训练集
    testSet = []               # 构建测试集
    for i in range(0, v - 1):
        # print(i*Nk, (i+1)*Nk-1)
        train = dataSet[:i * Nk]
        testSet.append(dataSet[i * Nk:(i + 1) * Nk])
        train.extend(dataSet[(i + 1) * Nk:])
        trainSet.append(train)
    # print((k-1)*Nk, N-1)
    trainSet.append(dataSet[:(v - 1) * Nk])
    testSet.append(dataSet[(v - 1) * Nk:N])
    classCount = {}
    for data in dataSet:       # 统计数据集类别
        currentLabel = data[-1]
        if currentLabel not in classCount.keys():
            classCount[currentLabel] = 0
            classCount[currentLabel] += 1
    k = len(classCount)
    n = 100
    baseValue = N / k
    for i in range(1, n):      # 通过交叉验证选择最优m值
        m = pow(k, i)
        index = crossValidation(trainSet, testSet, labels, m)
        if index < baseValue:
            baseValue = index
            bestValue = m
    return bestValue
