from pylab import *

rootNode = dict(boxstyle='sawtooth', fc='#87CEEB')      # 根节点样式
decisionNode = dict(boxstyle='square', fc='#90EE90')    # 决策节点样式
leafNode = dict(boxstyle='round4', fc='#FF6347')        # 叶节点样式
arrow_args = dict(arrowstyle='<-')                      # 箭头样式
rootCount = 0

# 绘制父节点指向子节点带箭头的线，并在节点上添加文本
def plotNode(nodeTxt, childPt, parentPt, nodeType):
    """
    :param nodeTxt: 节点文本
    :param childPt: 子节点
    :param parentPt: 父节点
    :param nodeType: 节点类型
    """
    createPlot.ax1.annotate(nodeTxt, xy=parentPt, xytext=childPt,
                            xycoords='axes fraction', textcoords='axes fraction',
                            va='center', ha='center',
                            bbox=nodeType, arrowprops=arrow_args)
    # annotate为添加注释的函数；xy为父节点坐标；xytext为子节点坐标
    # xycoords和textcoords分别为父、子节点坐标样式，即左下角的坐标轴
    # va和ha表示节点分裂的位置，bbox节点文本的样式；arrowprop为箭头的样式

# 在父、子节点之间填充文本信息
def plotMidText(childPt, parentPt, txtString):
    """
    :param childPt: 子节点
    :param parentPt: 父节点
    :param txtString: 注释信息
    """
    xMid = (parentPt[0]-childPt[0])/2+childPt[0]  # 计算文字的x坐标
    yMid = (parentPt[1]-childPt[1])/2+childPt[1]  # 计算文字的y坐标
    createPlot.ax1.text(xMid, yMid, txtString)    # 横线上的信息

# 获取叶节点数量
def getNumLeaf(Tree):
    """
    :param Tree:决策树
    :return:返回树的叶节点
    """
    numLeafs = 0
    firstStr = list(Tree.keys())[0]  # 获得第一个键名key
    # print('------------------')
    # print('firstStr = ', firstStr)
    secondDict = Tree[firstStr]      # 通过键名key，获取对应键值value
    # print('secondDict = ', secondDict)
    for keys in secondDict.keys():    # 对于其中的一个子树
        # 如果这棵树下还有子树, 即其对应的value是字典dict
        # print('key = ', key)
        # print('name = ', type(secondDict[key]).__name__)
        if type(secondDict[keys]).__name__ == 'dict':
            # 递归调寻找叶子结点，把节点数目加到总数中
            # print('numLeafs = ',  numLeafs)
            numLeafs += getNumLeaf(secondDict[keys])
            # print('if numLeafs = ',  numLeafs)
        else:  # 如果这棵树已经是叶结点了, 即不再包含字典了
            numLeafs += 1             # 递归出口, 记录叶结点数增加了1
            # print('else numLeafs = ',  numLeafs)
    return numLeafs                   # 返回这个树下总的叶结点数目

# 获取树的深度
def getTreeDepth(Tree):
    """
    :param Tree:决策树
    :return:返回树的深度
    """
    maxDepth = 0
    firstStr = list(Tree.keys())[0]
    secondDict = Tree[firstStr]
    for keys in secondDict.keys():
        # 如果这棵树下还有子树, 即其对应的value是字典dict
        if type(secondDict[keys]).__name__ == 'dict':
            # 递归调寻找叶子结点，树的深度加1
            thisDepth = 1+getTreeDepth(secondDict[keys])
        else:  # 如果这棵树已经是叶结点了, 即不再包含字典了
            thisDepth = 1         # 递归出口, 节点处深度为1
        # 完成一次递归，比较取得各个子树深度，最大的深度即为树的深度
        # print('firstStr = ', firstStr)
        # print('key = ', key)
        # print('thisDepth = ', thisDepth)
        if thisDepth > maxDepth:
            maxDepth = thisDepth
    return maxDepth

# 决策树主体结合节点
def plotTree(Tree, parentPt, nodeTxt):
    """
    :param Tree:决策树
    :param parentPt:父节点
    :param nodeTxt:节点文本
    """
    global rootCount                 # 用来判断节点类型
    numLeaf = getNumLeaf(Tree)       # 获取树的叶节点
    firstStr = list(Tree.keys())[0]  # 找到第一个键名key
    secondDict = Tree[firstStr]      # 通过键名key，获取对应键值value
    # 计算子节点坐标，totalW为决策树的总宽度，totalD为决策树的总高度
    childPt = (plotTree.xOff+(1.0+float(numLeaf))/2.0/plotTree.totalW, plotTree.yOff)
    # print('-------------------')
    # print('childPt = ', childPt)
    # print('parentPt = ', parentPt)
    plotMidText(childPt, parentPt, nodeTxt)  # 绘制子、父节点间的信息
    if rootCount == 0:               # 第一次绘制的父节点为根节点，其余为决策节点
        plotNode(firstStr, childPt, parentPt, rootNode)
        rootCount += 1
    else:
        plotNode(firstStr, childPt, parentPt, decisionNode)
    # 为了绘制子树，y轴偏移量按比例减少，向下移动（树是向下绘制）
    plotTree.yOff -= 1.0 / plotTree.totalD
    # print('plotTree.yOff = ', plotTree.yOff)
    for keys in secondDict.keys():
        # 如果这棵树下还有子树, 即其对应的value是字典dict
        # print('firstStr = ',firstStr)
        # print('secondDict = ',secondDict)
        # print('key = ',key)
        if type(secondDict[keys]).__name__ == 'dict':
            plotTree(secondDict[keys], childPt, str(keys))  # 递归绘制其子树
        else:  # 如果这棵子树是叶结点了
            # 因为x轴偏移量要作为节点坐标，所以x轴偏移量按比例增加
            plotTree.xOff += 1.0 / plotTree.totalW
            # print('plotTree.xOff = ', plotTree.xOff)
            # 绘制叶节点，并在子父节点间添加注释
            # 子节点充当下一支树的父节点，为新的子节点
            # print('-------------------')
            # print('childPt = ', childPt)
            # print('parentPt = ', parentPt)
            plotNode(secondDict[keys], (plotTree.xOff, plotTree.yOff), childPt, leafNode)
            plotMidText((plotTree.xOff,  plotTree.yOff), childPt, str(keys))
    # 绘制完子节点，需要回到父节点绘制另一子节点，所以把减掉的y轴偏移量按比例加回来
    plotTree.yOff += 1.0 / plotTree.totalD
    # print('plotTree.yOff = ', plotTree.yOff)

# 结合各部分，绘制决策树
def createPlot(Tree, titleTree):
    """
    :param Tree: 决策树
    :param titleTree:树的名称
    """
    if type(Tree).__name__ != 'dict':
        print('This Tree cannot be displayed, since it is incomplete')
    else:
        global rootCount
        rootCount = 0
        fig = plt.figure(titleTree, facecolor='white')
        fig.clf()                                    # 清除figure
        axprops = dict(xticks=[], yticks=[])
        # 创建一个子图，并把第一个figure的Axes实例返回给ax1，作为函数createPlot()属性
        # 这个属性ax1相当于一个全局变量，并在plotNode函数中使用
        createPlot.ax1 = plt.subplot(111, frameon=False,  **axprops)
        plotTree.totalW = float(getNumLeaf(Tree))    # 叶节点为树的宽度
        plotTree.totalD = float(getTreeDepth(Tree))  # 树的深度为树的宽度
        # print('plotTree.totalW = ', plotTree.totalW)
        # print('plotTree.totalD = ', plotTree.totalD)
        # xOff和yOff两个全局变量定位节点位置，以及放置下一个节点的恰当位置
        plotTree.xOff = -0.5/plotTree.totalW         # 树在x轴的偏移
        plotTree.yOff = 1.0                          # 树在y轴的偏移
        # print('plotTree.xOff = ', plotTree.xOff)
        # print('plotTree.yOff = ', plotTree.yOff)
        plotTree(Tree, (0.5, 1.0), ' ')
