import collections
from math import log
import operator
import treePlotter_entropy

from most_class_compute import most_class_computes

# 具体熵计算实例
# https://blog.csdn.net/appleyuchi/article/details/82220044

# 代码架构是:
# calcShannonEnt  calcInfoGain
#            |                |
#            \              /
#       chooseBestFeatureToSplit          majorityCnt
#                 |                                   |
#                 \                                   /
#                             createTree              makeTreeFull
#                                 |                           |          
#                                 \                           /
#                                         treePlotter

def createDataSet():
    """
    创建测试的数据集
    :return:
    """
# 下面这个是西瓜数据集2.0,绘制周志华<机器学习>图4.4
    dataSet = [
        # 1
        ['青绿', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
        # 2
        ['乌黑', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
        # 3
        ['乌黑', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
        # 4
        ['青绿', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
        # 5
        # ['浅白', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
#--------------------------------------------
        # 6
        ['青绿', '稍蜷', '浊响', '清晰', '稍凹', '软粘', '好瓜'],
        # 7
        ['乌黑', '稍蜷', '浊响', '稍糊', '稍凹', '软粘', '好瓜'],
        # 8
        # ['乌黑', '稍蜷', '浊响', '清晰', '稍凹', '硬滑', '好瓜'],
        # 9
        # ['乌黑', '稍蜷', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜'],
        # 10
        ['青绿', '硬挺', '清脆', '清晰', '平坦', '软粘', '坏瓜'],
#--------------------------------------------
        # 11
        # ['浅白', '硬挺', '清脆', '模糊', '平坦', '硬滑', '坏瓜'],
        # 12
        # ['浅白', '蜷缩', '浊响', '模糊', '平坦', '软粘', '坏瓜'],
        # 13
        # ['青绿', '稍蜷', '浊响', '稍糊', '凹陷', '硬滑', '坏瓜'],
        # 14
        ['浅白', '稍蜷', '沉闷', '稍糊', '凹陷', '硬滑', '坏瓜'],
        # 15
        ['乌黑', '稍蜷', '浊响', '清晰', '稍凹', '软粘', '坏瓜'],
        # 16
        ['浅白', '蜷缩', '浊响', '模糊', '平坦', '硬滑', '坏瓜'],
        # 17
        ['青绿', '蜷缩', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜']
    ]

# 西瓜数据集2.0α,,绘制周志华<机器学习>图4.9
    # dataSet = [
        # 1
        # ['-', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
        # # 2
        # ['乌黑', '蜷缩', '沉闷', '清晰', '凹陷', '-', '好瓜'],
        # # 3
        # ['乌黑', '蜷缩', '-', '清晰', '凹陷', '硬滑', '好瓜'],
        # # 4
        # ['青绿', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
        # 5
        # ['-', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
#--------------------------------------------
        # 6
        # ['青绿', '稍蜷', '浊响', '清晰', '-', '软粘', '好瓜'],
        # # 7
        # ['乌黑', '稍蜷', '浊响', '稍糊', '稍凹', '软粘', '好瓜'],
        # 8
        # ['乌黑', '稍蜷', '浊响', '-', '稍凹', '硬滑', '好瓜'],
        # 9
        # ['乌黑', '-', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜'],
        # 10
        # ['青绿', '硬挺', '清脆', '-', '平坦', '软粘', '坏瓜'],
#--------------------------------------------
        # 11
        # ['浅白', '硬挺', '清脆', '模糊', '平坦', '-', '坏瓜'],
        # 12
        # ['浅白', '蜷缩', '-', '模糊', '平坦', '软粘', '坏瓜'],
        # 13
        # ['-', '稍蜷', '浊响', '稍糊', '凹陷', '硬滑', '坏瓜'],
        # 14
    #     ['浅白', '稍蜷', '沉闷', '稍糊', '凹陷', '硬滑', '坏瓜'],
    #     # 15
    #     ['乌黑', '稍蜷', '浊响', '清晰', '-', '软粘', '坏瓜'],
    #     # 16
    #     ['浅白', '蜷缩', '浊响', '模糊', '平坦', '硬滑', '坏瓜'],
    #     # 17
    #     ['青绿', '-', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜']
    # ]


    #特征名称列表
    labels_all = ['色泽', '根蒂', '敲击', '纹理', '脐部', '触感']
    # 各个样本的权重
    Wx = []
    for i in range(len(dataSet)):
        Wx.append(1)
    # 特征对应的所有可能的情况
    labels_full = {}
    for i in range(len(labels_all)):
        labelList = [example[i] for example in dataSet if example[i] != '-']
        uniqueLabel = set(labelList)
        labels_full[labels_all[i]] = uniqueLabel
    return dataSet, labels_all, labels_full, Wx
#这里的labels_full指的是所有特征的取值的可能的汇总


#对其中一个"特征取值-不同类别"计算熵
#Entropy(feature_value)
def calcShannonEnt(dataSet, Wx):
    # print("-----------------------進入calcShannonEnt--------------------------")
    """
    计算给定数据集的信息熵(香农熵)，需要考虑权重
    :param dataSet:数据集
    :param Wx:数据集对应的权重
    :return:
    """
    # 计算出数据集的总数
    numEntries = len(dataSet)
    print("numEntries=",numEntries)
    print("sum(Wx)=",sum(Wx))
    
    # 得到类别的总集合
    classify = [example[-1] for example in dataSet]
    # 去重，得到唯一的分类集合
    classify = set(classify)
    # 用来统计标签
    labelCounts = collections.defaultdict(int)

    # 遍历所有的分类集合
    for currentClassify in classify:
        # 循环遍历一遍数据集，找到与当前分类相等的数据权重
        for i in range(numEntries):
            example = dataSet[i]
            # 如果此时的数据等于当前的分类，找到此时数据的权重加入其中
            if example[-1] == currentClassify:
                labelCounts[currentClassify] += Wx[i]
                # print("Wx[i]=",Wx[i])#这里全都是1
    # 默认的信息熵
    shannonEnt = 0.0
    for key in labelCounts:#这里的key就是"好瓜、坏瓜"
        print("这里的key=",key)
        # 计算当前分类的权重
        prob = float(labelCounts[key]) / float(sum(Wx))
        # print("labelCounts[key]=",labelCounts[key])
        # print("sum(Wx)=",sum(Wx))
        # print("prob=",prob)
        # 以2为底求对数
        shannonEnt -= prob * log(prob, 2)
    return shannonEnt


def current_system_entropy(dataSet, Wx):
    print("进入current_system_entropy=",current_system_entropy)
    # print("-----------------------進入calcShannonEnt--------------------------")
    """
    计算给定数据集的信息熵(香农熵)，需要考虑权重
    :param dataSet:数据集
    :param Wx:数据集对应的权重
    :return:
    """
    # 计算出数据集的总数
    numEntries = len(dataSet)
    # 得到类别的总集合
    classify = [example[-1] for example in dataSet]
    # 去重，得到唯一的分类集合
    classify = set(classify)
    # 用来统计标签
    labelCounts = collections.defaultdict(int)

    # 遍历所有的分类集合
    for currentClassify in classify:
        # 循环遍历一遍数据集，找到与当前分类相等的数据权重
        for i in range(numEntries):
            example = dataSet[i]
            # 如果此时的数据等于当前的分类，找到此时数据的权重加入其中
            if example[-1] == currentClassify:
                labelCounts[currentClassify] += Wx[i]
                # print("Wx[i]=",Wx[i])#这里全都是1
    # 默认的信息熵
    shannonEnt = 0.0
    for key in labelCounts:#这里的key就是"好瓜、坏瓜"
        print("这里的key=",key)
        # 计算当前分类的权重
        print("float(labelCounts[key])=",float(labelCounts[key]))
        prob = float(labelCounts[key]) / 17.0
        # print("labelCounts[key]=",labelCounts[key])
        # print("sum(Wx)=",sum(Wx))
        # print("prob=",prob)
        # 以2为底求对数
        print("prob=",prob)
        print("prob * log(prob, 2)=",prob * log(prob, 2))
        shannonEnt -= prob * log(prob, 2)
    return shannonEnt

def splitDataSet(dataSet, axis, value, Wx):#axis是数据集中的列坐标
    """
    按照给定的特征值，将数据集划分
    :param dataSet: 数据集
    :param axis: 给定特征值的坐标
    :param value: 给定特征值满足的条件，只有给定特征值等于这个value的时候才会返回
    :param Wx: 数据集对应的权重
    :return:
    """
    # 创建一个新的列表，防止对原来的列表进行修改
    retDataSet = []
    # 子集的权重列表
    subWx = []
    # 遍历整个数据集
    for i in range(len(dataSet)):#i是行坐标
        featVec = dataSet[i]
        # 如果给定特征值等于想要的特征值
        if featVec[axis] == value:
            # 将该特征值前面的内容保存起来
            reducedFeatVec = featVec[:axis]
            # 将该特征值后面的内容保存起来，所以将给定特征值给去掉了
            reducedFeatVec.extend(featVec[axis + 1:])
            # 添加到返回列表中
            retDataSet.append(reducedFeatVec)
            subWx.append(Wx[i])
    return retDataSet, subWx

# 计算如果以该特征作为划分时的信息增益
def calcInfoGain(dataSet , i, baseEntropy, Wx):
    """
    计算信息增益
    :param dataSet: 数据集
    :param featList: 当前特征列表
    :param i: 当前特征值下标
    :param baseEntropy: 基础信息熵
    :param subWx: 子集权重
    :return:
    """
    # 得到总的特征列表
    featList = [example[i] for example in dataSet]
    # 将当前特征唯一化，也就是说当前特征值中共有多少种
    uniqueVals = set(featList)
    # 新的熵，代表当前特征值的熵
    newEntropy = 0.0

    # 遍历某特征的所有取值,计算条件熵
    for value in uniqueVals:
        # 在全部数据集的当前特征位置上，找到该特征值等于当前值的集合，并且得到对应的权重
        subDataSet, subWx = splitDataSet(dataSet=dataSet, axis=i, value=value, Wx=Wx)
        # 计算出此时子集的信息熵
        subEntropy = calcShannonEnt(subDataSet, subWx)
        # 计算出当前特征之的权重
        prob = float(sum(subWx)) / float(sum(Wx))
        # 计算出带权重的信息增益
        newEntropy += prob * subEntropy
    # 计算出“信息增益”
    infoGain = baseEntropy - newEntropy#如果以该特征作为划分
    #这里的含义是IGT=E(S)-E(S|T)=E(S)-Σp·E(feature_value)
    return infoGain


def chooseBestFeatureToSplit(dataSet, labels, Wx):
    """
    选择最好的分类标签，根据信息增益值来计算，可以处理缺失值
    :param dataSet: 需要划分的数据集
    :param labels: 特征值列表
    :param Wx: 样本的权重
    :return:
    """
    # 得到数据的特征值总数
    numFeatures = len(dataSet[0]) - 1

    # 得到满足特征值要求的样本的总数
    dataSetSum = len(dataSet)

    # 基础信息增益为0.0
    bestInfoGain = 0.0

    # 最好的特征值
    bestFeature = -1

#-----------------------for循环开始-------------------------------
    # 循环遍历所有的特征
    
    for i in range(numFeatures):
        print("------------外圍for-這是第%s輪循環----------"%i)
        print("当前关注特征=",i)

        # 除去当前特征值的缺失值之后的子集
        subDataSet = []

        # 子集的权重
        subWx = []

        # 计算子集的权重
        for j in range(dataSetSum):
            example = dataSet[j]
            if example[i] != '-':
                subDataSet.append(example)
                print("j=",j)
                subWx.append(Wx[j])
        print("看看这个subWx=",subWx)

        print("------------计算子集的信息熵-------------")
        print("当前关注的特征是=",labels[i])
        print("当前剩下的数据集是=",subDataSet)
        baseEntropy = calcShannonEnt(subDataSet, subWx)
        print("baseEntropy=",baseEntropy)

        # 计算出权重
        print("subWx=",subWx)
        print("Wx=",Wx)
        prob = float(sum(subWx)) / float(sum(Wx))
        print("prob=",prob)
        # print("prob=",prob)

        # 计算当前特征值的信息增益
        infoGain = prob * calcInfoGain(subDataSet, i, baseEntropy, subWx)

        # print('当前的特征值为:' + labels[i])
        # print('当前的信息增益值为：' + str(infoGain))
        # 记录此时最高的信息增益
        print("infoGain=",infoGain)
        if infoGain > bestInfoGain:
            bestInfoGain = infoGain
            bestFeature = i
#-----------------------for循环结束----------------------------------
    # print('最好的划分特征值：' + labels[bestFeature])
    # print('此时的信息增益值为：' + str(bestInfoGain))
    print("labels=",labels)
    print("选择出来的bestFeature=",bestFeature)
    print("选择出来的bestFeature=",labels[bestFeature])
    return bestFeature,baseEntropy


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


def createTree(dataSet, labels, Wx):
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
        return classList[0],str(classList[0])+'\n'+'Entropy=0'
        #因爲到了葉子節點，熵肯定是降低爲０了

    # 计算第一行有多少个数据，如果只有一个的话说明所有的特征属性都遍历完了，剩下的一个就是类别标签
    if len(dataSet[0]) == 1:
        # 返回剩下标签中出现次数较多的那个
        resultm=majorityCnt(classList)
        return resultm,str(resultm)+'\n'+'Entropy=0'

    # 选择最好的划分特征，得到该特征的下标
    bestFeat,nodeEntropy= chooseBestFeatureToSplit(dataSet=dataSet, labels=labels, Wx=Wx)#????????
    # nodeEntropy=current_system_entropy(dataSet,Wx)
    # 得到最好特征的名称
    print("bestFeat=",bestFeat)
    bestFeatLabel = labels[bestFeat]
    print("最佳特征=",bestFeatLabel)

    # 使用一个字典来存储树结构，分叉处为划分的特征名称
    myTree = {bestFeatLabel: {}}

    #*****新增维护一个熵显示的树
    ｅｎｔｒｏｐｙ＿Tree ={bestFeatLabel: {},"Entropy=":nodeEntropy}


    # 将本次划分的特征值从列表中删除掉
    print("即将删除的特征是:",labels[bestFeat])
    del(labels[bestFeat])
    print("删除特征以后的特征名称列表是:",labels)

    # 得到当前特征标签的所有可能值
    featValues = [example[bestFeat] for example in dataSet if example[bestFeat] != '-']

    # 唯一化，去掉重复的特征值
    uniqueVals = set(featValues)

    # 非缺失值的权重???????????????
    noHiatusWx = []

    # 遍历数据集，找到当前划分的特征值的所有非缺失值??????????????
    for i in range(len(dataSet)):
        example = dataSet[i]
        if example[bestFeat] != '-':
            noHiatusWx.append(Wx[i])

    # 遍历所有的特征值
    for value in uniqueVals:
        # 得到剩下的标签
        subLabels = labels[:]
        # 得到对应的特征值子集和权重
        subDataSet, subWx = splitDataSet(dataSet=dataSet, axis=bestFeat, value=value, Wx=Wx)
        # 用来保存缺失值的样本
        hiatusList = []
        # 保存对应的权重
        hiatusWx = []
        # 遍历所有的数据集，找到缺失值并计算权重
        for i in range(len(dataSet)):
            example = dataSet[i]
            # 找到缺失值
            if example[bestFeat] == '-':
                hiatusExample = example[:bestFeat]
                hiatusExample.extend(example[bestFeat+1:])
                # 添加到子集中
                hiatusList.append(hiatusExample)
                # 缺失值的新权重
                newWx = Wx[i]
                # 重新计算权重
                newWx *= (sum(subWx)/sum(noHiatusWx))
                # 添加到子集权重中
                hiatusWx.append(newWx)
        # 将其添加到子集中
        for i in range(len(hiatusList)):
            subDataSet.append(hiatusList[i])
            subWx.append(hiatusWx[i])

        # print('当前划分点：' + bestFeatLabel + '，对应的特征值：' + value)
        # print(subDataSet)
        # print(subWx)

        # 递归调用
        subTree,sub_entropy_tree= createTree(subDataSet, subLabels, subWx)
        #subLabels指的是劃分屬性
        print("subDataSet=",subDataSet)
        myTree[bestFeatLabel][value] = subTree
        #这句代码的意思是ｍｙＴｅｅ下面的最佳特征［bestFeatLabel］下面的对应的（树枝）特征取值[value]
        #连接的子树是ｓｕｂＴｒｅｅ
        ｅｎｔｒｏｐｙ＿Tree[bestFeatLabel][value] = sub_entropy_tree


    return myTree,ｅｎｔｒｏｐｙ＿Tree


#注意每次递归都是在处理当前的根节点和下面的几个树枝
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
    sub_tree = myTree[root_key]#这里是获取"根节点下面的所有树枝",也就是特征的所有取值

    # 如果是叶子节点就结束
    if isinstance(sub_tree, str):
        return
###########################################
    

###########################################
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




    # 遍历当前节点特征的所有取值，将不存在标签添加进去
    for label in labels_full[root_key]:#①
        if label not in sub_tree.keys():
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

    # 找到本层出现最多的类别，作为parentLabel传递给下一次递归
    #这里的"层"指的是包含根节点的这一层
    print("current_class=",current_class)
    print("myTree=",myTree)
    print("collections.Counter(current_class)=",collections.Counter(current_class))
    #这里的current class指的是类别标签列表,里面可以有各种类别,诸如[好瓜,坏瓜]等
    # if len(current_class):
    #     most_class = collections.Counter(current_class).most_common(1)[0][0]
    print("２入口satisfy_lists=",satisfy_lists)


    # 递归处理

    print("当前sub_tree.keys()=",sub_tree.keys())
    for sub_key in sub_tree.keys():
        ｐｒｉｎｔ("－－－－－－－－－－－－－－－－进入ｆｏｒ循环－－－－－－－－－－－－－－")
        print("当前的sub_key=",sub_key)
        print("sub_tree[sub_key]=",sub_tree[sub_key])
        print("sub_tree[sub_key]==未知",sub_tree[sub_key]=='未知')
        print("type(sub_tree[sub_key])=",type(sub_tree[sub_key]))
        print("isinstance(sub_tree[sub_key], dict)=",isinstance(sub_tree[sub_key], dict))
    #这里的ｓｕｂ_key是树枝的取值
    #所以这个ｆｏｒ循环在遍历当前根节点的每个树枝
    # 也就是在遍历当前特征的每种取值



        if isinstance(sub_tree[sub_key], dict):#如果子树不是叶子节点
            ｐｒｉｎｔ("－－－－－－－－－－－－－－递归调用makeTreeFull前－－－－－－－－－－－－－－")
            temp=satisfy_lists.copy()
            ｐｒｉｎｔ("satisfy_lists=",satisfy_lists)
            print("most_class=",most_class)
            satisfy_lists.append(sub_key)
            makeTreeFull(ｓａｔｉｓｆｙ＿ｌｉｓｔｓ,dataSet,myTree=sub_tree[sub_key], labels_full=labels_full, parentClass=most_class, default=default)
            satisfy_lists=temp


if __name__ == '__main__':
    print("---------python3----------------------")
    """
    处理存在缺失值的决策树
    """
    dataSet, labels_all, labels_full, Wx = createDataSet()
    # chooseBestFeatureToSplit(dataSet, labels, Wx)
    myTree,entropy_tree= createTree(dataSet, labels_all, Wx)
    print("myTree=",myTree)
    print("entropy_tree=",entropy_tree)
    # myTree = {'纹理': {'稍糊': {'敲击': {'清脆': '坏瓜', '浊响': '坏瓜', '沉闷': '好瓜'}}, '模糊': {'色泽': {'青绿': '好瓜', '浅白': '坏瓜'}}}}
    # treePlotter.createPlot(myTree)
    # labels_full = {'纹理': {'稍糊', '模糊'}, '敲击': {'清脆', '沉闷', '浊响'}, '色泽': {'乌黑', '青绿', '浅白'}}
    satisfy_lists=[]
    print ("satisfy_lists=",satisfy_lists)
    makeTreeFull(satisfy_lists,datasets=dataSet,myTree=entropy_tree, labels_full=labels_full, parentClass=None, default='未知')
    treePlotter_entropy.createPlot(entropy_tree)