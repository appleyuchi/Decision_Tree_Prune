from CreateDT.Operation import CalcEntropy, splitDataSet, majorityCnt

# 最大信息增益为标准，选择最佳特征
def chooseBestFeature(dataSet):
    """
    :param dataSet:数据集
    :return:返回最佳特征
    """
    lenFeature = len(dataSet[0])-1                       # 计算特征维度时去掉"类别"
    Hd = CalcEntropy(dataSet)                            # 按类别，计算数据集的信息熵
    baseValue = 0.0                                      # 比较基准值
    bestFeature = 0                                      # 最佳特征
    for i in range(lenFeature):
        Hd_a = 0.0                                       # 被特征i固定的条件熵
        feature = [example[i] for example in dataSet]    # 提取每组数据的第i个特征
        feature = set(feature)                           # 得到第i个特征的种类个数
        for fea in feature:
            subData = splitDataSet(dataSet, i, fea)      # 将特征i数据集按特征值fea分类
            p = float(len(subData))/float(len(dataSet))  # 计算特征值fea在特征i集合的概率
            # print('p = ',p,'i = ',i,'fea = ',fea)
            # 在特征i集合中，特征值fea数据的概率与信息熵乘积相加，即为被特征i固定的条件熵
            Hd_a += p*CalcEntropy(subData)
        Gain = Hd-Hd_a  # 计算信息增益，即数据信息熵与单个特征的条件熵的差
        # print(i, 'Gain=', Gain)
        if Gain > baseValue:                             # 将信息增益与比较基准量baseValue比较
            baseValue = Gain
            bestFeature = i                              # 根据增益选择最佳特征
    return bestFeature

# 创建分类决策树
def createID3Tree(dataSet, labels):
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
    feaBest = chooseBestFeature(dataSet)                 # 选择最佳分类特征
    feature = [example[feaBest] for example in dataSet]  # 提取每组数据的第i个特征
    feature = set(feature)                               # 得到第i个特征的种类个数
    newLabel = labels[feaBest]                           # 得到该特征的名称
    # print('------------\n feaBest = ',newlabel)
    # print('feature = ',feature)
    del(labels[feaBest])                                 # 删掉已分类的特征名称
    Tree = {newLabel: {}}                                # 创建一个多重字典，存储决策树分类结果
    for value in feature:
        subLabels = labels[:]  # 构建特征名称子集合
        # 利用递归函数不断创建分支，直到所有特征完成分类，分类结束
        Tree[newLabel][value] = createID3Tree(splitDataSet(dataSet, feaBest, value), subLabels)
    return Tree