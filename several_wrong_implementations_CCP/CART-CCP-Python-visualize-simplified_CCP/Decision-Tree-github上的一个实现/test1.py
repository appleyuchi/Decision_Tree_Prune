from CreateDT.ID3 import createID3Tree
from CreateDT.C4_5 import createC4_5Tree
from CreateDT.CART import createCARTTree
from CreateDT.PlotDT import createPlot
import matplotlib.pyplot as plt

# 读取数据集文件
def loadDataSet(fileName):
    """
    :param fileName:数据集文件
    :return:数据集
    """
    file = open(fileName)                   # 打开数据集文件
    line = file.readline()                  # 读取每行所有元素
    dataSet = []                            # 数据集初始化
    while line:
        data = line.strip('\n').split(',')  # 按照','划分数据，并剔除回车符
        dataSet.append(data)                # 将每行数据放到数据集
        line = file.readline()
    file.close()
    return dataSet

# 构造原始数据集和属性集合
originalDataSet = loadDataSet('DataSet/watermelon.txt')
labels = originalDataSet[0]
dataSet = originalDataSet[1:]

def showDT(dataSet, labels):
    """
    :param dataSet:数据集
    :param labels:属性标签
    """

    # ID3算法生成分类决策树
    ID3Tree = createID3Tree(list(dataSet), list(labels))
    print('The ID3 Decision Tree is', ID3Tree)

    # C4.5算法生成分类决策树
    C4_5Tree = createC4_5Tree(list(dataSet), list(labels))
    print('The C4.5 Decision Tree is', C4_5Tree)

    # CART算法生成分类决策树
    CARTTree = createCARTTree(list(dataSet), list(labels))
    print('The CART Decision Tree is', CARTTree)

    # 显示各个决策树
    createPlot(ID3Tree, 'ID3 Decision Tree')
    createPlot(C4_5Tree, 'C4.5 Decision Tree')
    createPlot(CARTTree, 'CART Decision Tree')
    plt.show()  # 显示决策树

showDT(dataSet, labels)
