from CreateDT.ID3 import createID3Tree
from CreateDT.C4_5 import createC4_5Tree
from CreateDT.CART import createCARTTree
from CreateDT.PlotDT import createPlot, getTreeDepth, getNumLeaf
from CreateDT.Operation import createSet, testAccuracy, splitDataSet, majorityCnt, findKeyNode
from PurnDT.Pre_REP_Pruning import PrePruning, PostPruning_REP
from PurnDT.Post_Pruning import PostPruning_PEP, PostPruning_MEP,\
PostPruning_CCP, prunBranch, crossValidation,IMEP_CrossValidation, PostPruning_IMEP
import matplotlib.pyplot as plt
import copy
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

classCount = {}
for data in dataSet:
    currentLabel = data[-1]
    if currentLabel not in classCount.keys():
        classCount[currentLabel] = 0
        classCount[currentLabel] += 1
k = len(classCount)


'''
trainSet=[]
testSet=[]
trainSet.extend(dataSet[0:3])
trainSet.extend(dataSet[5:7])
trainSet.append(dataSet[9])
trainSet.extend(dataSet[13:])
testSet.extend(dataSet[3:5])
testSet.extend(dataSet[7:9])
testSet.extend(dataSet[10:13])

print('trainSet=\n', labels, '\n')
for data in trainSet:
    print(data)
print('-------------------------')
print('testSet=\n', labels, '\n')
for data in testSet:
    print(data)
print('-------------------------')
'''
