from CreateDT.ID3 import chooseBestFeature
from CreateDT.Operation import classify
from CreateDT.Operation import majorityCnt, splitDataSet


# 将数目最多类别替换节点，计算验证集精度
def testMajor(trainSet, testSet):
    """
    :param trainSet:训练集
    :param testSet:测试集
    :return:验证集精度
    """
    classList = [example[-1] for example in trainSet]
    majorClass = majorityCnt(classList)   # 找到数量最多的类别
    # print('majorClass=', majorClass)
    accuracy = 0.0
    for i in range(len(testSet)):         # 游历测试集每个元素，找出正确样本个数
        if testSet[i][-1] == majorClass:
            accuracy += 1
    return float(accuracy)

# 按决策树划分后，计算验证集精度
def testCreate(trainSet, testSet, feaBest, labelIndex):
    """
    :param trainSet:训练集
    :param testSet:测试集
    :param feaBest:最佳分类特征
    :param labelIndex:该特征在原集合的索引值
    :return:验证集精度
    """
    accuracy = 0.0
    feature = [example[feaBest] for example in trainSet]  # 提取每组数据的最佳分类特征
    feature = set(feature)                                # 得到该特征的种类集合
    for value in feature:                                 # 游历该特征下每个属性
        # print('value=', value)
        subTrainSet = splitDataSet(trainSet, feaBest, value)
        subClass = [example[-1] for example in subTrainSet]
        majorSub = majorityCnt(subClass)                  # 统计得到最大类别
        # print('majorSub=', majorSub)
        for data in testSet:                              # 游历测试集每个元素
            if data[labelIndex] == value:                 # 对应特征下特征属性相同
                if data[-1] == majorSub:                  # 如果类别与对应最大类别相同，则样本正确
                    accuracy += 1
    return float(accuracy)

# 用预剪枝方法，对ID3算法生成决策树进行剪枝
def PrePruning(trainSet, testSet, labels):
    """
    :param trainSet: 训练集
    :param testSet: 测试集
    :param labels: 类别标签
    :return: 剪枝后的决策树
    """
    classList = [example[-1] for example in trainSet]     # classList是训练集的所有类别

    # 情况1：若classList只有一类，则停止划分
    if classList.count(classList[0]) == len(classList):
        return classList[0]

    # 情况2：若完成所有的特征分类,返回个数最多的
    if len(trainSet[0]) == 1:
        return majorityCnt(classList)                     # 投票选出最多的特征

    # 情况3：classList有多类，开始进行分类
    feaBest = chooseBestFeature(trainSet)                 # 选择最佳分类特征
    feature = [example[feaBest] for example in trainSet]  # 提取每组数据的第i个特征
    feature = set(feature)                                # 得到第i个特征的种类个数
    # print('feaBest=', labels[feaBest])
    # print('feature=', feature)
    newLabel = labels[feaBest]                            # 得到该特征的名称
    labelIndex = labels.index(newLabel)                   # 获取该特征在原特征集合的索引值
    beforeAccuracy = testMajor(trainSet, testSet)         # 划分前样本集精度
    afterAccuracy = testCreate(trainSet, testSet, feaBest, labelIndex)  # 划分后样本集精度
    # print('beforeAccuracy=', beforeAccuracy)
    # print('afterAccuracy=', afterAccuracy)
    del (labels[feaBest])                                 # 删掉已分类的特征名称
    Tree = {newLabel: {}}                                 # 创建一个多重字典，存储决策树分类结果
    if afterAccuracy > beforeAccuracy:                    # 如果划分后验证集精度比划分前高
        for value in feature:                             # 那么该特征可以划分，构造决策树
            # print('==============================')
            # print('value=', value)
            subLabels = labels[:]                         # 构建特征名称子集合
            # 利用递归函数不断创建分支，直到所有特征完成分类，分类结束
            subTrainSet = splitDataSet(trainSet, feaBest, value)  # 按对应特征，划分训练集
            subTestSet = splitDataSet(testSet, feaBest, value)    # 按对应特征，划分测试集
            Tree[newLabel][value] = PrePruning(subTrainSet, subTestSet, subLabels)
        return Tree
    else:                                                 # 如果划分后没有划分前精度高
        return majorityCnt(classList)                     # 那么停止划分，返回最多的类别

# 剪枝前按照决策树，计算验证集精度
def testPrun(testSet, Tree, labels):
    """
    :param testSet:训练集
    :param Tree:决策树
    :param labels:类别集
    :return:剪枝前的精度
    """
    accuracy = 0.0
    for i in range(len(testSet)):  # 游历测试集每个元素，按照决策树分类，与样本类别比较
        # print('classify=', classify(Tree, labels, testSet[i]))
        if classify(Tree, labels, testSet[i]) == testSet[i][-1]:
            accuracy += 1          # 如果分类结果与测试集类别相同，精度加1
    return float(accuracy)

# 用REP后剪枝方法，对ID3算法生成决策树进行剪枝
def PostPruning_REP(trainSet, testSet, labels):
    """
    :param trainSet:训练集
    :param testSet:测试集
    :param labels:类别属性集
    :return:剪枝后的决策树
    """
    classList = [example[-1] for example in trainSet]     # classList是训练集的所有类别

    # 情况1：若classList只有一类，则停止划分
    if classList.count(classList[0]) == len(classList):
        return classList[0]

    # 情况2：若完成所有的特征分类,返回个数最多的
    if len(trainSet[0]) == 1:
        return majorityCnt(classList)                     # 投票选出最多的特征

    # 情况3：classList有多类，开始进行分类
    feaBest = chooseBestFeature(trainSet)                 # 选择最佳分类特征
    feature = [example[feaBest] for example in trainSet]  # 提取每组数据的第i个特征
    feature = set(feature)                                # 得到第i个特征的种类个数
    newLabel = labels[feaBest]                            # 得到该特征的名称
    del (labels[feaBest])                                 # 删掉已分类的特征名称
    Tree = {newLabel: {}}                                 # 创建一个多重字典，存储决策树分类结果
    for value in feature:                                 # 那么该特征可以划分，构造决策树
        subLabels = labels[:]                             # 构建特征名称子集合
        # 利用递归函数不断创建分支，直到所有特征完成分类，分类结束
        subTrainSet = splitDataSet(trainSet, feaBest, value)  # 按对应特征，划分训练集
        subTestSet = splitDataSet(testSet, feaBest, value)    # 按对应特征，划分测试集
        Tree[newLabel][value] = PostPruning_REP(subTrainSet, subTestSet, subLabels)
    # print('=============================')
    # print('Tree=', Tree)
    labels.insert(feaBest, newLabel)                      # 为了使用决策树分类，将删掉的类别还原
    beforeAccuracy = testPrun(testSet, Tree, labels)      # 计算剪枝前测试集精度
    afterAccuracy = testMajor(trainSet, testSet)          # 计算剪枝后测试集精度
    # print('beforeAccuracy=', beforeAccuracy)
    # print('afterAccuracy=', afterAccuracy)
    del (labels[feaBest])                                 # 再将分类好的类别删掉
    if afterAccuracy > beforeAccuracy:                    # 如果剪枝后精度比剪枝前大，那么进行剪枝
        return majorityCnt(classList)
    else:                                                 # 否则不剪枝，返回原决策树
        return Tree