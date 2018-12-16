from CreateDT.ID3 import createID3Tree
from CreateDT.C4_5 import createC4_5Tree
from CreateDT.CART import createCARTTree
from CreateDT.Operation import createSet, testAccuracy,findKeyNode
from CreateDT.PlotDT import getNumLeaf, getTreeDepth
from PurnDT.Pre_REP_Pruning import PrePruning, PostPruning_REP
from PurnDT.Post_Pruning import PostPruning_PEP, PostPruning_MEP, PostPruning_CCP, IMEP_CrossValidation, \
    PostPruning_IMEP
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
originalDataSet = loadDataSet('DataSet/nursery.data.txt')
labels = originalDataSet[0]

# 读取数据集
dataSet = originalDataSet[1:1000]
dataSet = tuple(dataSet)

classCount = {}
for data in dataSet:
    currentLabel = data[-1]
    if currentLabel not in classCount.keys():
        classCount[currentLabel] = 0
        classCount[currentLabel] += 1
k = len(classCount)

# 生成训练集和测试集
trainSet, testSet = createSet(dataSet)
trainSet = tuple(trainSet)
'''
print('trainSet=\n', labels, '\n')
for data in trainSet:
    print(data)
print('-------------------------')
print('testSet=\n', labels, '\n')
for data in testSet:
    print(data)
print('-------------------------')
'''

"""对ID3算法生成的决策树，进行剪枝"""
''''''
print('===========================================')
# 先用ID3算法，将训练集生成决策树
ID3TrainTree = createID3Tree(list(trainSet), list(labels))
print('The ID3 Decision Tree:', 'Depth:', getTreeDepth(ID3TrainTree), ';Leaf:', getNumLeaf(ID3TrainTree))
print('The Node with largest number is', findKeyNode(ID3TrainTree))

# 根据测试集，进行预剪枝
PreID3Tree = PrePruning(list(trainSet), testSet, list(labels))
print('The Pre_Pruning_ID3 Decision Tree:', 'Depth:', getTreeDepth(PreID3Tree), ';Leaf:', getNumLeaf(PreID3Tree))
print('The Node with largest number is', findKeyNode(PreID3Tree))

# 根据测试集，使用REP方法进行后剪枝
REPID3Tree = PostPruning_REP(list(trainSet), testSet, list(labels))
print('The REP_Pruning_ID3 Decision Tree:', 'Depth:', getTreeDepth(REPID3Tree), ';Leaf:', getNumLeaf(REPID3Tree))
print('The Node with largest number is', findKeyNode(REPID3Tree))

# 根据测试集，使用REP方法进行后剪枝
PEPID3Tree = PostPruning_PEP(ID3TrainTree, list(labels), list(dataSet))
print('The PEP_Pruning_ID3 Decision Tree:', 'Depth:', getTreeDepth(PEPID3Tree), ';Leaf:', getNumLeaf(PEPID3Tree))
print('The Node with largest number is', findKeyNode(PEPID3Tree))

# 计算剪枝前后准确度
ID3Accuracy = testAccuracy(ID3TrainTree, labels, trainSet)
print('The Accuracy of ID3 Decision Tree is', ID3Accuracy)
PreID3Accuracy = testAccuracy(PreID3Tree, labels, trainSet)
print('The Accuracy of Pre_Pruning_ID3 Decision Tree is', PreID3Accuracy)
REPID3Accuracy = testAccuracy(REPID3Tree, labels, trainSet)
print('The Accuracy of REP_Pruning_ID3 Decision Tree is', REPID3Accuracy)
PEPID3Accuracy = testAccuracy(PEPID3Tree, labels, trainSet)
print('The Accuracy of PEP_Pruning_ID3 Decision Tree is', PEPID3Accuracy)


"""对C4.5算法生成的决策树，进行剪枝"""
''''''
print('===========================================')
# 先用C4.5算法，生成决策树
C4_5Tree = createC4_5Tree(list(trainSet), list(labels))
print('The C4.5 Decision Tree:', 'Depth:', getTreeDepth(C4_5Tree), ';Leaf:', getNumLeaf(C4_5Tree))
print('The Node with largest number is', findKeyNode(C4_5Tree))
copyC4_5Tree = copy.deepcopy(C4_5Tree)  # 将生成树拷贝
copyC4_5Tree1 = copy.deepcopy(C4_5Tree)  # 将生成树拷贝

# 使用PEP方法，对决策树进行后剪枝
PEPC4_5Tree = PostPruning_PEP(C4_5Tree, list(labels), list(trainSet))
print('The PEP_Pruning_C4.5 Decision Tree:', 'Depth:', getTreeDepth(PEPC4_5Tree), ';Leaf:', getNumLeaf(PEPC4_5Tree))
print('The Node with largest number is', findKeyNode(PEPC4_5Tree))

# 使用MEP方法，对决策树进行后剪枝
MEPC4_5Tree = PostPruning_MEP(copyC4_5Tree, list(labels), list(trainSet), k)
print('The MEP_Pruning_C4.5 Decision Tree:', 'Depth:', getTreeDepth(MEPC4_5Tree), ';Leaf:', getNumLeaf(MEPC4_5Tree))
print('The Node with largest number is', findKeyNode(MEPC4_5Tree))

# 使用IMEP方法，对决策树进行后剪枝
m = IMEP_CrossValidation(list(trainSet), list(labels))
IMEPC4_5Tree = PostPruning_IMEP(copyC4_5Tree1, list(labels), list(trainSet), m)
print('The IMEP_Pruning_C4.5 Decision Tree:', 'Depth:', getTreeDepth(IMEPC4_5Tree), ';Leaf:', getNumLeaf(IMEPC4_5Tree))
print('The Node with largest number is', findKeyNode(IMEPC4_5Tree))

# 计算剪枝前后准确度
C4_5Accuracy = testAccuracy(C4_5Tree, labels, trainSet)
print('The Accuracy of C4.5 Decision Tree is', C4_5Accuracy)
PEPC4_5Accuracy = testAccuracy(PEPC4_5Tree, labels, trainSet)
print('The Accuracy of PEP_Pruning_C4.5 Decision Tree is', PEPC4_5Accuracy)
MEPC4_5Accuracy = testAccuracy(MEPC4_5Tree, labels, trainSet)
print('The Accuracy of MEP_Pruning_C4.5 Decision Tree is', MEPC4_5Accuracy)
IMEPC4_5Accuracy = testAccuracy(IMEPC4_5Tree, labels, trainSet)
print('The Accuracy of IMEP_Pruning_C4.5 Decision Tree is', IMEPC4_5Accuracy)


"""对CART算法生成的决策树，进行剪枝"""
''''''
print('===========================================')
# 先用CART算法，将测试集生成决策树
CARTTree = createCARTTree(list(trainSet), list(labels))
print('The CARTTree Decision Tree:', 'Depth:', getTreeDepth(CARTTree), ';Leaf:', getNumLeaf(CARTTree))
print('The Node with largest number is', findKeyNode(CARTTree))
copyCARTTree = copy.deepcopy(CARTTree)  # 将生成树拷贝
copyCARTTree1 = copy.deepcopy(CARTTree)  # 将生成树拷贝

# 使用MEP方法，对决策树进行后剪枝
MEPCARTTree = PostPruning_MEP(CARTTree, list(labels), list(trainSet), k)
print('The MEP_Pruning_CART Decision Tree:', 'Depth:', getTreeDepth(MEPCARTTree), ';Leaf:', getNumLeaf(MEPCARTTree))
print('The Node with largest number is', findKeyNode(MEPCARTTree))

# 使用CCP方法，对决策树进行后剪枝
CCPCARTTree = PostPruning_CCP(copyCARTTree, list(labels), list(trainSet), testSet)
print('The CCP_Pruning_CART Decision Tree:', 'Depth:', getTreeDepth(CCPCARTTree), ';Leaf:', getNumLeaf(CCPCARTTree))
print('The Node with largest number is', findKeyNode(CCPCARTTree))
print"CCPCARTR"
'''
# 使用IMEP方法，对决策树进行后剪枝
m = IMEP_CrossValidation(list(dataSet), list(labels))
IMEPCARTTree = PostPruning_MEP(copyCARTTree1, list(labels), list(trainSet), m)
print('The IMEP_Pruning_CART Decision Tree:', 'Depth:', getTreeDepth(IMEPCARTTree), ';Leaf:', getNumLeaf(IMEPCARTTree))
print('The Node with largest number is', findKeyNode(IMEPCARTTree))
'''

# 计算剪枝前后准确度
CARTAccuracy = testAccuracy(CARTTree, labels, trainSet)
print('The Accuracy of CART Decision Tree is', CARTAccuracy)
MEPCARTAccuracy = testAccuracy(MEPCARTTree, labels, trainSet)
print('The Accuracy of MEP_Pruning_CART Decision Tree is', MEPCARTAccuracy)
CCPCARTAccuracy = testAccuracy(CCPCARTTree, labels, trainSet)
print('The Accuracy of CCP_Pruning_CART Decision Tree is', CCPCARTAccuracy)

