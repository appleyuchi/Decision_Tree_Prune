# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-10-31 16:48:38
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-11-29 11:43:09

import treePlotter
from getNumofCommonSubstr import getNumofCommonSubstr

branches_label=["<=","=",">"]
def data_source(path):
    f = open(path)
    line = f.readline()#这里会读取第一行，所以这里先存储一行。
    items=[]
    items.append(line.split("\n")[0])
    matrixs=[]
    matrixs.append(items)

    while line:
        # print "line=",line
        items=[]
        line = f.readline()
        items.append(line.split("\n")[0])
        matrixs.append(items)
    f.close()
    
    # for index in range(len(model)):
    matrixs.remove([''])#去除一些小瑕疵
    return matrixs




def prints(matrix):
    for item in matrix:
        print item

def matrix_split(matrix):
    branches=[]
    branch_Number=[]
    for index,item in enumerate(matrix):
        result = "|" in item[0]
        if result==False:#如果是当前数据的最高层节点
            branch_Number.append(index)
            for sign in branches_label:
                if sign in item[0]:
                    split_node= item[0].split(sign)[0].strip()
                    branch=item[0].split(sign)[1].split(":")[0].strip()
                    branch=sign+branch#①
                    branches.append(branch)
                    break
    # print"-------------得到最高节点的名称以及它的树枝取值列表------------------"
    # print split_node#对应的分割节点的名称
    # print branches#每个branch的取值
    # print "branch_Number=",branch_Number#每个branch对应的行的下标
    branch_Number.append(len(matrix))#为了方便后面操作

    for item in matrix:
        if "|   " in item[0]:
            item[0]=item[0][4:]#也就是：删除最开头的"|   "
    # print"matrix=",matrix
   
    new_datasets=[]
    for i in range(len(branches)):#有几个分支就要把数据集分成几份：
        subdata=matrix[branch_Number[i]+1:branch_Number[i+1]]
        if subdata==[]:#如果剩下的数据只够叶子节点，不够分割
            subdata=matrix[branch_Number[i]]
        # print"subdata=",subdata


        new_datasets.append(subdata)#范围是branch_Number[i]+1～branch_Number[i+1]-1

    # for sub in new_datasets:
    #     print"++++++++++++++++"
    #     for it in sub:
    #         print it
    return split_node,branches,branch_Number,new_datasets

def branch_with_leaf(testdata):
    # print"testdata=",testdata

    #如果是单分支：
    if len(testdata)==1:
        return testdata[0].split(":")[1]#返回str叶子节点）
        
    #否则需要对很多分支进行整合
    branch_dict={}
    result={}
    split_node=""
    for item in testdata:
        for sign in branches_label:
            if sign in item[0]:
                split_node= item[0].split(sign)[0].strip()
                branch=item[0].split(sign)[1].split(":")[0].strip()
                branch=sign+branch#②
                leaf=item[0].split(sign)[1].split(":")[1]

                branch_dict[branch]=leaf
                break

    result[split_node]=branch_dict
    return result#返回字典

    # {A13:{"=g":" + (2.0)",
    #       "= p":" - (0.0)",
    #        "=s":""}}





#这里是在转化quinlan的模型的结果
def create_tree(matrixs):
    #----------------下面是截止条件--------------------------

    flag=0#
    for item in matrixs:
        # print "截止条件中item=",item
        if "|" in item[0]:
            flag=1
            break
    if flag==0:
        # print"进入了flag=0"
        # print"matrixs=",matrixs
        leaf=branch_with_leaf(matrixs)
        return  leaf

    #----------------上面是截止条件--------------------------
    # print"分割数据集以前"
    # print"matrixs=",matrixs
    split_node,branches,branch_Number,new_datasets=matrix_split(matrixs)
    # print"branches=",branches
    # print"new_datasets=",new_datasets
    myTree = {split_node: {}}
    for index,branch in enumerate(branches):
        sub_matrix=new_datasets[index]

        subTree=create_tree(sub_matrix)
        # print"-----------这里的数据集是start----------------"
        # print"branch=",branch
        # print"sub_matrix=",sub_matrix
        # print "subTree=",subTree
        # print"-----------end----------------"

        myTree[split_node][branch]=subTree
    return myTree

def classify(inputTree,features,testVec):#这里的inputTree就是决策树的序列化表示方法,python中是字典类型,也可以看做是json类型
    firstStr = inputTree.keys()[0]#获取决策树的当前分割属性
    secondDict = inputTree[firstStr]#当前分割节点下面的一堆树枝+节点

    featIndex = features.index(firstStr)#当前是第几个特征
    key = testVec[featIndex]#根据划分属性的下标来获取测试数据的对应下标的属性的具体取值

#--------------获得子树--------------------------------
    if isinstance(key, str):#如果是离散特征
        valueOfFeat = secondDict[key]#根据这个值来顺着树枝key选择子树secondDict[key](离散特征)
    else:
        item_lists=[]
        for item in secondDict:
            item_lists.append(item)
        common_str=getNumofCommonSubstr(item_lists[0],item_lists[1])[0]#common_str是
        if key<=float(common_str):
            key="<="+common_str
            valueOfFeat = secondDict[key]
        else:
            key=">"+common_str
            valueOfFeat = secondDict[key]

#----------------获得子树------------------------------

    if isinstance(valueOfFeat, dict): #如果是子树
        classLabel = classify(valueOfFeat, features, testVec)#递归调用
    else: #如果是叶子节点
        classLabel = valueOfFeat
    return classLabel#递归函数的结束条件


#注意，不支持包含缺失值的数据的测试
def classify_C45(valueOfFeat, features, data):#注意，这里的ｄａｔａ指的是一条数据，不是一堆数据
    for index,item in enumerate(data):#因为模型中离散特征有“＝”符号，所以这里给离散特征的数据加上“＝”，方便预测
        if isinstance(item, str):
            data[index]="="+data[index]
    return classify(valueOfFeat, features, data)
def test1():
    features=["A1","A2","A3","A4","A5","A6","A7","A8","A9","A10","A11","A12","A13","A14","A15"]
    testVec1=['b',30.83,0,'u','g','w','v',1.25,'t','t',01,'f','g',00202,0]#+
    testVec2=['a',19.17,0.585,'y','p','aa','v',0.585,'t','f',0,'t','g',00160,0]#-
    result1=classify_C45(myTree,features,testVec1)
    result2=classify_C45(myTree,features,testVec2)
    print "result1=",result1
    print "result2=",result2


if __name__ == '__main__':
    # path="./watermelon实验结果.txt"
    path="./result/unprune.txt"
    #---导入C4.5-Release8的数据---
    matrixs=data_source(path)
    #---转化为python决策树模型----
    myTree=create_tree(matrixs)
    #------输出决策树模型----
    print"模型是：",myTree
    #------绘制决策树----
    # treePlotter.createPlot(myTree)
    #------使用该模型进行预测----注意，如果数字被认为是离散特征，需要转化为str类型再输入--------
    #注意：为了预测，注释掉了①②处，绘图时可以取消注释,也可以不取消注释
    # test1()


