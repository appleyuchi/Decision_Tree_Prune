import matplotlib.pylab as plt
import matplotlib

# # 能够显示中文
# matplotlib.rcParams['font.sans-serif'] = ['SimHei']
# matplotlib.rcParams['font.serif'] = ['SimHei']

# 分叉节点，也就是决策节点
decisionNode = dict(boxstyle="sawtooth", fc="0.8")

# 叶子节点
leafNode = dict(boxstyle="round4", fc="0.8")

# 箭头样式
arrow_args = dict(arrowstyle="<-")


def plotNode(nodeTxt, centerPt, parentPt, nodeType):
    nodeTxt=nodeTxt
    """
    绘制一个节点
    :param nodeTxt: 描述该节点的文本信息
    :param centerPt: 文本的坐标
    :param parentPt: 点的坐标，这里也是指父节点的坐标
    :param nodeType: 节点类型,分为叶子节点和决策节点
    :return:
    """
    createPlot.ax1.annotate(nodeTxt, xy=parentPt, xycoords='axes fraction',
                            xytext=centerPt, textcoords='axes fraction',
                            va="center", ha="center", bbox=nodeType, arrowprops=arrow_args)


def getNumLeafs(myTree):
    """
    获取叶节点的数目
    :param myTree:
    :return:
    """
    # 统计叶子节点的总数
    numLeafs = 0

    # 得到当前第一个key，也就是根节点
    for item in myTree.keys():
        if item !='Entropy=':
            firstStr=item

    # 得到第一个key对应的内容
    secondDict = myTree[firstStr]

    # 递归遍历树枝
    print("secondDict=",secondDict)
    for key in secondDict.keys():
        if type(secondDict[key]).__name__ == 'dict':
            numLeafs += getNumLeafs(secondDict[key])
        # 不是的话，说明此时是一个叶子节点
        else:
            numLeafs += 1
    return numLeafs


def getTreeDepth(myTree):
    """
    得到数的深度层数
    :param myTree:
    :return:
    """
    # 用来保存最大层数
    maxDepth = 0

    # 得到根节点
    for item in myTree.keys():
        if item!='Entropy=':
            firstStr=item
    # firstStr = list(myTree.keys())[0]

    # 得到key对应的内容
    secondDic = myTree[firstStr]

    # 遍历所有子节点
    for key in secondDic.keys():
        # 如果该节点是字典，就递归调用
        if type(secondDic[key]).__name__ == 'dict':
            # 子节点的深度加1
            thisDepth = 1 + getTreeDepth(secondDic[key])

        # 说明此时是叶子节点
        else:
            thisDepth = 1

        # 替换最大层数
        if thisDepth > maxDepth:
            maxDepth = thisDepth

    return maxDepth


def plotMidText(cntrPt, parentPt, txtString):
    txtString=txtString
    """
    计算出父节点和子节点的中间位置，填充信息
    :param cntrPt: 子节点坐标
    :param parentPt: 父节点坐标
    :param txtString: 填充的文本信息
    :return:
    """
    # 计算x轴的中间位置
    xMid = (parentPt[0]-cntrPt[0])/2.0 + cntrPt[0]
    # 计算y轴的中间位置
    yMid = (parentPt[1]-cntrPt[1])/2.0 + cntrPt[1]
    # 进行绘制
    createPlot.ax1.text(xMid, yMid, txtString)


def plotTree(myTree, parentPt, nodeTxt):
    """
    绘制出树的所有节点，递归绘制
    :param myTree: 树
    :param parentPt: 父节点的坐标
    :param nodeTxt: 节点的文本信息
    :return:
    """
    # 计算叶子节点数
    numLeafs = getNumLeafs(myTree=myTree)

    # 计算树的深度
    depth = getTreeDepth(myTree=myTree)

    # 得到根节点的信息内容
    # firstStr = list(myTree.keys())[0]
    for item in myTree:
        if item!='Entropy=':
            firstStr=item
    print("firstStr=",firstStr)

    # 计算出当前根节点在所有子节点的中间坐标,也就是当前x轴的偏移量加上计算出来的根节点的中心位置作为x轴（比如说第一次：初始的x偏移量为：-1/2W,计算出来的根节点中心位置为：(1+W)/2W，相加得到：1/2），当前y轴偏移量作为y轴
    cntrPt = (plotTree.xOff + (1.0 + float(numLeafs))/2.0/plotTree.totalW, plotTree.yOff)


    # 绘制该节点与父节点的联系
    plotMidText(cntrPt, parentPt, nodeTxt)
    # 绘制该节点
    plotNode(firstStr+"\n"+"Entropy="+str(myTree['Entropy=']), cntrPt, parentPt, decisionNode)
    #上面两句中：
    # nodeTxt:树枝内容
    # fｉｒｔＳtr:节点内容



    # 得到当前根节点对应的子树
    secondDict = myTree[firstStr]

    # 计算出新的y轴偏移量，向下移动1/D，也就是下一层的绘制y轴
    plotTree.yOff = plotTree.yOff - 1.0/plotTree.totalD

    # 循环遍历所有的key
    for key in secondDict.keys():
        # 如果当前的key是字典的话，代表还有子树，则递归遍历
        if isinstance(secondDict[key], dict):
            print("str(key)=",str(key))#这个是树枝，也就是特征值
            plotTree(secondDict[key], cntrPt, str(key))
        else:
            # 计算新的x轴偏移量，也就是下个叶子绘制的x轴坐标向右移动了1/W
            plotTree.xOff = plotTree.xOff + 1.0/plotTree.totalW
            # 打开注释可以观察叶子节点的坐标变化
            # print((plotTree.xOff, plotTree.yOff), secondDict[key])
            # 绘制叶子节点
            plotNode(secondDict[key], (plotTree.xOff, plotTree.yOff), cntrPt, leafNode)
            # 绘制叶子节点和父节点的中间连线内容
            plotMidText((plotTree.xOff, plotTree.yOff), cntrPt, str(key))

    # 返回递归之前，需要将y轴的偏移量增加，向上移动1/D，也就是返回去绘制上一层的y轴
    plotTree.yOff = plotTree.yOff + 1.0/plotTree.totalD


def createPlot(inTree):
    """
    需要绘制的决策树
    :param inTree: 决策树字典
    :return:
    """
    # 创建一个图像
    fig = plt.figure(1, facecolor='white')
    fig.clf()
    axprops = dict(xticks=[], yticks=[])
    createPlot.ax1 = plt.subplot(111, frameon=False, **axprops)
    # 计算出决策树的总宽度
    plotTree.totalW = float(getNumLeafs(inTree))
    # 计算出决策树的总深度
    plotTree.totalD = float(getTreeDepth(inTree))
    # 初始的x轴偏移量，也就是-1/2W，每次向右移动1/W，也就是第一个叶子节点绘制的x坐标为：1/2W，第二个：3/2W，第三个：5/2W，最后一个：(W-1)/2W
    plotTree.xOff = -0.5/plotTree.totalW
    # 初始的y轴偏移量，每次向下或者向上移动1/D
    plotTree.yOff = 1.0
    # 调用函数进行绘制节点图像
    plotTree(inTree, (0.5, 1.0), '')
    # 绘制
    plt.show()


myTree= {'纹理': {'稍糊': {'敲击': {'浊响': {'脐部': {'稍凹': '好瓜', '凹陷': '坏瓜'}}, '清脆': '坏瓜', '沉闷': '坏瓜'}}, '清晰': {'根蒂': {'稍蜷': {'色泽': {'青绿': '好瓜', '乌黑': {'触感': {'软粘': '坏瓜', '硬滑': '好瓜'}}}}, '硬挺': '坏瓜', '蜷缩': '好瓜'}}, '模糊': {'色泽': {'浅白': '坏瓜', '青绿': '坏瓜', '乌黑': '好瓜'}}}}
entropy_tree= {'Entropy=': 0.9967916319816366, '纹理': {'稍糊': {'Entropy=': 0.787126586201269, '敲击': {'浊响': {'Entropy=': 0.9852281360342516, '脐部': {'稍凹': '好瓜\nEntropy=0', '凹陷': '坏瓜\nEntropy=0'}}, '清脆': '坏瓜\nEntropy=0', '沉闷': '坏瓜\nEntropy=0'}}, '模糊': {'Entropy=': 0.41381685030363374, '色泽': {'浅白': '坏瓜\nEntropy=0', '乌黑': '好瓜\nEntropy=0', '青绿': '坏瓜\nEntropy=0'}}, '清晰': {'Entropy=': 0.7444131797881748, '根蒂': {'蜷缩': '好瓜\nEntropy=0', '稍蜷': {'Entropy=': 0.9740248644357521, '色泽': {'乌黑': {'Entropy=': 0.9023932827949789, '触感': {'软粘': '坏瓜\nEntropy=0', '硬滑': '好瓜\nEntropy=0'}}, '青绿': '好瓜\nEntropy=0'}}, '硬挺': '坏瓜\nEntropy=0'}}}}

if __name__ == '__main__':
    # print(list(testTree.keys()))
    print('Entropy='+str(entropy_tree['Entropy=']))

    # createPlot(myTree)
    createPlot(entropy_tree)