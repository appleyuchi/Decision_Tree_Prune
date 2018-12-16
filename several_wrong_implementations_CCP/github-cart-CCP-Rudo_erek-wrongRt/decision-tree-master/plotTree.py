#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

import matplotlib.pyplot as plt
import pandas as pd
from decisionTree import DecisionTree
from decisionTreeC45 import DecisionTreeC45
from decisionTreeCART import DecisionTreeCART


class PlotTree:
    def __init__(self):
        self.decisionNode = dict(boxstyle="sawtooth", fc="0.8")
        self.leafNode = dict(boxstyle="round4", fc="0.8")
        self.arrow_args = dict(arrowstyle="<-")

    # compute the number of leaf node
    def getNumLeafs(self, myTree):
        numLeafs = 0
        if myTree.feature is not None:
            for key in myTree.children:
                numLeafs += self.getNumLeafs(myTree.children[key])
        elif myTree.category is not None:
            numLeafs += 1
        else:
            return 0
        return numLeafs

    # compute the depth of the tree
    def getTreeDepth(self, myTree):
        if myTree.feature is not None:
            currDepth = 0
            for key in myTree.children:
                thisDepth = self.getTreeDepth(myTree.children[key])
                if thisDepth > currDepth:
                    currDepth = thisDepth
            return currDepth + 1

        elif myTree.category is not None:
            return 1
        else:
            return 0

    # draw the tree
    def plotTree(self, myTree, parentPt, MidText):
        numLeafs = self.getNumLeafs(myTree)
        depth = self.getTreeDepth(myTree)
        # firstStr = list(myTree.children)[0]

        cntrpt = (PlotTree.plotTree.xOff + (1.0 + float(numLeafs))/2.0/PlotTree.plotTree.totalW, PlotTree.plotTree.yOff)
        self.plotNode(myTree.feature, cntrpt, parentPt, self.decisionNode)
        if myTree.parent is not None:
            self.plotMidText(cntrpt, parentPt, MidText)
        secondDict = myTree.children
        PlotTree.plotTree.yOff = PlotTree.plotTree.yOff - 1.0/PlotTree.plotTree.totalD
        for key in secondDict:
            if secondDict[key].feature is not None:
                self.plotTree(secondDict[key], cntrpt, str(key))
            else:
                PlotTree.plotTree.xOff = PlotTree.plotTree.xOff + 1.0/PlotTree.plotTree.totalW
                self.plotNode(secondDict[key].category, (PlotTree.plotTree.xOff, PlotTree.plotTree.yOff), cntrpt, self.leafNode)
                self.plotMidText((PlotTree.plotTree.xOff, PlotTree.plotTree.yOff), cntrpt, str(key))
        PlotTree.plotTree.yOff = PlotTree.plotTree.yOff + 1.0/PlotTree.plotTree.totalD


    # 在父子节点之间填充文本信息
    def plotMidText(self, cntrPt, parentPt, textString):
        xMid = (parentPt[0] - cntrPt[0])/2.0 + cntrPt[0]
        yMid = (parentPt[1] - cntrPt[1])/2.0 + cntrPt[1]
        PlotTree.createPlot.ax1.text(xMid, yMid, textString, va="center", ha="center", rotation=30)

    # 绘制带箭头的注解
    def plotNode(self, nodeText, centerPt, parentPt, nodeType):
        PlotTree.createPlot.ax1.annotate(nodeText, xy=parentPt, xycoords='axes fraction', xytext=centerPt, textcoords='axes fraction', va='center', ha="center", bbox=nodeType, arrowprops=self.arrow_args)

    def createPlot(self, inTree):
        fig = plt.figure(1, facecolor='white', figsize=(80, 30))
        fig.clf()
        axprops = dict(xticks=[], yticks=[])
        PlotTree.createPlot.ax1 = plt.subplot(111, frameon=False, **axprops)
        PlotTree.plotTree.totalW = float(self.getNumLeafs(inTree))
        PlotTree.plotTree.totalD = float(self.getTreeDepth(inTree))
        PlotTree.plotTree.xOff = -0.5/self.plotTree.totalW;
        PlotTree.plotTree.yOff = 1.0
        # self.plotNode(inTree.feature, (0.5, 1.0), (0.5, 1.0), self.decisionNode)
        self.plotTree(inTree, (0.5, 1.0), MidText='')
        plt.savefig('out/images/Titanic_CART_Cut_3.png')
        plt.show()

# 西瓜书训练数据
# train_data = pd.read_csv('data/watermelon_data.csv')
# X_data = train_data.drop('haogua', axis=1)
# Y_data = train_data['haogua']

# Titanic训练数据
# train_data = pd.read_csv('data/train_dealt.csv')
# Y_data = train_data['Survived']
# X_data = train_data.drop('Survived', axis=1)

# ID3算法
# tree = DecisionTree(X_data, Y_data).root_node
# pt = PlotTree()
# pt.createPlot(tree)

# C4.5算法
# tree = DecisionTreeC45(X_data, Y_data).root_node
# pt = PlotTree()
# pt.createPlot(tree)

# CART算法
# tree = DecisionTreeCART(X_data, Y_data).root_node
# pt = PlotTree()
# pt.createPlot(tree)