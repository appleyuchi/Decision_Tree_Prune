#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from two_error import pae_list,static_error,get_Attribute,backed_up_error,subdata_counts
from split import splitdatasets
import copy
from treePlotter import createPlot
from predict import *

# There are 3 ways to set the value of "m" 
# according the article <on estimating probabilities in tree pruning>-1991

# m初始定义是等效样本数量

#①set m according to maximising the classification accuracy on an independent dataset.
#  m=a list and you should try each value to find the best value of m 
#  pae = apriori  probabilities of class c

#②set m according to the noise in datasets(先实现这一种)
#  m= if you want to prune more ,just increase m please.
#  pae = apriori  probabilities of class c

#③m-probabilities-estimates = Laplace Law of Succession
#  according to page141 of <on estimating probabilities in tree pruning>
#  m = counts of classes of datasets
#  pae = 1.0/m


#Attention that "Laplace's law of succession" is less fit the reality of the whole datasets. 

#----------------ｇｅｔ Attribute list of datasets--------------------------------------------
def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list

#---------------------用来从叶子的ｓｔｒ中提取到分类错误的数据，用来被后面的MEP判据使用---------------------------------------
# data=' 4 (2.0)'
# data='6 (6.0/3.0)'
def brackets_data(data):#从叶子节点中获取错误分类的数据数量，
    if '/' in data:
        leaf_error=data.split('/')[1].split(')')[0]
        leaf_error=float(leaf_error)
    else:
        leaf_error=0
    return leaf_error

#---------------下面的代码是统计当前模型中数据集的总数量---------------------------------------------
# data=' 4 (2.0)'
# data='7 (6.0/3.0)'
#获取总ｉｔｅｍ数量
def leaf_items(data):#获取叶子节点中的数据总条数，入口的data是str类型
    if '/' in data:
        leaf_item_counts=data.split('/')[0].split('(')[1]#如果是上面的data2，那么得到6
        leaf_item_counts=float(leaf_item_counts)
    else:
        leaf_item_counts=data.split('(')[1].split(')')[0]#如果是上面的data1，那么得到2
        leaf_item_counts=float(leaf_item_counts)
    return leaf_item_counts

def items_count(model_in):#统计“当前树”数据集数量
    item_count=0
    if isinstance(model_in,str):
        # print"model=",model
        item_count=item_count+ leaf_items(model_in)
        return item_count

    best_feature=model_in.items()[0][0]#因为当前树的根节点肯定只有一个特征，第一个[0]是获取ｋｅｙ列表，当然，该列表中只有一个元素，所以再使用第二个[0]
    branches=model_in[best_feature]
    for sub_tree in branches:#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
        item_count+=items_count(model_in[best_feature][sub_tree])
    return item_count
#---------------上面的代码是统计当前模型中数据集的总数量---------------------------------------------



import csv
from collections import Counter
import pandas as pd 

def read_data(path):
    source_data=pd.read_csv(path,header=None)#第一行不是标题
    lists=source_data.values.tolist()
    return lists



#find the minimum value of the following "static error"(errors after being pruned)
#Es=(N-ne+(1-pae)m)/(N+m)


#注意搞清楚从上往下剪枝与从下往上剪枝的区别

#从上到下剪枝，那么截止条件是两个：
# 1，当前子树能不能剪枝，如果当前子树剪枝不成功，再for循环检查他的树枝
#如果剪枝成功，那么就返回叶子节点
# 2，当前是叶子节点直接返回。


#从下到上MEP剪枝，
# 1.到达叶子节点最邻近的那个节点以后才开始判断能不能剪枝，
# 2.一开始就要使用for循环
# 3.从下往上剪枝时，如果最底部的子树剪枝失败，那么回溯时仍然需要判断整棵树是否需要剪枝
# 也就是说，ｆｏｒ循环后面不能直接ｒｅｔｕｒｎ，还需要对当前整棵树进行修缮。




def MEP_result(model_input,fea_list,datasets,pae_list,class_list,m):#down->top
    if isinstance(model_input,str):
        
        print"----------------这里进来没---------------"
        return model_input
    best_feature=model_input.items()[0][0]#因为当前树的根节点肯定只有一个特征，第一个[0]是获取ｋｅｙ列表，当然，该列表中只有一个元素，所以再使用第二个[0]
    branches=model_input[best_feature]

# # stopping conditions
#     print"for循环外面的model_input=",type(model_input)
#     for branch in branches:
#         print"for循环里面的model_input=",type(model_input)

#         if isinstance(model_input[best_feature][branch],str):#如果已经是叶子节点的上一个“分割节点”，由于是从下往上剪枝，此时应该考虑该“子树”是否满足剪枝要求并且剪枝
#             model_input=prune_current_tree(model_input,datasets,pae_list,class_list,m,fea_list)
#             break
#     if isinstance(model_input,str):
#         return model_input

#如果还未到达树的最底层的叶子节点的上面一个分割节点，那么继续往下搜索。
    print"最后一个ｆｏｒ循环外面,model_input=",model_input
    for branch in branches:#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
        sub_data=splitdatasets(best_feature,fea_list,branch,datasets)
        print"model_input=",model_input
        print"model_input[best_feature]=",model_input[best_feature]
        model_input[best_feature][branch]=MEP_result(model_input[best_feature][branch],fea_list,sub_data,pae_list,class_list,m)
    #处理完当前树的分支是否剪枝的判断以后，判断该树的子树是不是需要剪枝为一个叶子节点。  
    model_input=prune_current_tree(model_input,datasets,pae_list,class_list,m,fea_list)#如果树枝下面对应的子树都没有剪枝成功，可能存在剪枝的需要
    return model_input

def prune_current_tree(model,sub_datas,pae_list,class_list,m,feature_list):
    Eb=backed_up_error(model,sub_datas,pae_list,class_list,m,feature_list)
    count_list=subdata_counts(sub_datas,class_list)
    Estatic,Es_index,_=static_error(sub_datas,pae_list,count_list,class_list,m)

    if Eb>=Estatic:#如果动态错误≥静态错误
        if len(sub_datas)==count_list[Es_index]:#如果不存在误判的数据
            model=str(class_list[Es_index])+" ("+str(len(sub_datas))+")"
        else:
            model=str(class_list[Es_index])+" ("+str(len(sub_datas))+"/"+str(len(sub_datas)-count_list[Es_index])+")"

    return model


#－－－－－－－－－－－－－－－－递归调用部分－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－

#------------------测试部分----------------------------------
def accuracy_analysis(model,model_pruned,datasets,feature_list,name_path):
    conti_or_discrete=conti_or_discrete_list(name_path)
    count=0
    unpruned_accuracy=0.0
    misjudge_datasets=[]
    for item in copy.deepcopy(datasets):
        predict=classify_C45(model,feature_list,item,conti_or_discrete)
        print "item=",item
        print "predict=",predict
        print "预测结果=",predict.split("(")[0].strip()
        print"实际结果＝",str(item[-1])
        if predict.split("(")[0].strip()==str(item[-1]):
            print"计数增１"
            count+=1
            print"-------------------"
        else:
            misjudge_datasets.append(item)
    accuracy_unprune=float(count)/len(datasets)
    count=0

    for item in datasets:
        predict=classify_C45(model_pruned,feature_list,item,conti_or_discrete)
        if predict.split("(")[0].strip()==str(item[-1]):
            count+=1
    accuracy_prune=float(count)/len(datasets)
    return accuracy_unprune,accuracy_prune,misjudge_datasets

#for abalone datasets from UCI
def abalone_test(m):
    model={'Viscera': {'>0.0145': {'Shell': {'<=0.0345': {'Viscera': {'<=0.0285': ' 5 (50.0/9.0)', '>0.0285': ' 4 (3.0)'}}, '>0.0345': {'Sex': {'=M': ' 6 (6.0/3.0)', '=F': ' 5 (3.0)', '=I': ' 5 (59.0/12.0)'}}}}, '<=0.0145': {'Shucked': {'>0.007': ' 4 (66.0/31.0)', '<=0.007': {'Shucked': {'>0.0045': {'Shucked': {'>0.005': {'Height': {'<=0.02': ' 4 (2.0)', '>0.02': ' 3 (4.0)'}}, '<=0.005': ' 4 (3.0)'}}, '<=0.0045': {'Height': {'<=0.025': ' 1 (2.0/1.0)', '>0.025': ' 3 (2.0)'}}}}}}}}
    path="./abalone_parts.data"
    name_path="./abalone.names"
    fea_list=get_Attribute(name_path)
    datasets=read_data(path)
    pae_dict,class_count=pae_list(path)#不要进入递归，这个是剪枝前就要确定下来，并且在剪枝的过程中不可改变的。

#Attention,if you want to perform Laplace Law of succession,just set:
#pae_list=1.0/m
#m=counts of classes of the whole original datasets

    pae_lists=[pae_dict[key] for key in pae_dict]#获得先验概率列表
    class_list=[key for key in class_count]#获取数据集的类别列表



    model_pruned=MEP_result(copy.deepcopy(model),fea_list,copy.deepcopy(datasets),pae_lists,class_list,m)
    accuracy_unprune,accuracy_prune,misjudge_datasets=accuracy_analysis(model,model_pruned,copy.deepcopy(datasets),fea_list,name_path)
    print"accuracy_unprune=",accuracy_unprune
    print"accuracy_prune=",accuracy_prune
    createPlot(model)
    createPlot(model_pruned)
    print "model=",model
    print"model_pruned=",model_pruned




#for credit_a datasets from UCI
def credit_a_test(m):
    model={'A9': {'=t': {'A15': {'>228': ' + (106.0/2.0)', '<=228': {'A11': {'>3': {'A15': {'>4': {'A15': {'<=5': ' - (2.0)', '>5': {'A7': {'=v': ' + (5.0)', '=z': ' - (1.0)', '=dd': ' + (0.0)', '=ff': ' + (0.0)', '=o': ' + (0.0)', '=n': ' + (0.0)', '=h': ' + (3.0)', '=bb': ' + (1.0)', '=j': ' + (0.0)'}}}}, '<=4': ' + (25.0)'}}, '<=3': {'A4': {'=u': {'A7': {'=v': {'A14': {'<=110': ' + (18.0/1.0)', '>110': {'A15': {'>8': ' + (4.0)', '<=8': {'A6': {'=aa': {'A2': {'<=41': ' - (3.0)', '>41': ' + (2.0)'}}, '=w': {'A12': {'=t': ' - (2.0)', '=f': ' + (3.0)'}}, '=q': {'A12': {'=t': ' + (4.0)', '=f': ' - (2.0)'}}, '=ff': ' - (0.0)', '=r': ' - (0.0)', '=i': ' - (0.0)', '=x': ' - (0.0)', '=e': ' - (0.0)', '=d': ' - (2.0)', '=c': ' - (4.0/1.0)', '=m': {'A13': {'=g': ' + (2.0)', '=p': ' - (0.0)', '=s': ' - (5.0)'}}, '=cc': ' + (2.0/1.0)', '=k': ' - (2.0)', '=j': ' - (0.0)'}}}}}}, '=z': ' + (1.0)', '=bb': {'A14': {'<=164': ' + (3.4/0.4)', '>164': ' - (5.6)'}}, '=ff': ' - (1.0)', '=o': ' + (0.0)', '=n': ' + (0.0)', '=h': ' + (18.0)', '=dd': ' + (0.0)', '=j': ' - (1.0)'}}, '=l': ' + (0.0)', '=y': {'A13': {'=g': {'A14': {'<=204': ' - (16.0/1.0)', '>204': ' + (5.0/1.0)'}}, '=p': ' - (0.0)', '=s': ' + (2.0)'}}, '=t': ' + (0.0)'}}}}}}, '=f': {'A13': {'=g': ' - (204.0/10.0)', '=p': {'A2': {'<=36': ' - (4.0/1.0)', '>36': ' + (2.0)'}}, '=s': {'A4': {'=u': {'A6': {'=aa': ' - (0.0)', '=w': ' - (0.0)', '=q': ' - (1.0)', '=ff': ' - (2.0)', '=r': ' - (0.0)', '=i': ' - (3.0)', '=x': ' + (1.0)', '=e': ' - (0.0)', '=d': ' - (2.0)', '=c': ' - (3.0)', '=m': ' - (3.0)', '=cc': ' - (1.0)', '=k': ' - (4.0)', '=j': ' - (0.0)'}}, '=l': ' + (1.0)', '=y': ' - (8.0/1.0)', '=t': ' - (0.0)'}}}}}}
    path="./crx.data"
    name_path="./crx.names"
    fea_list=get_Attribute(name_path)
    datasets=read_data(path)
    print"刚读入的数据集",datasets


    pae_dict,class_count=pae_list(path)#不要进入递归，这个是剪枝前就要确定下来，并且在剪枝的过程中不可改变的。
    pae_lists=[pae_dict[key] for key in pae_dict]#获得先验概率列表

#Attention,if you want to perform Laplace Law of succession,just set:
#pae_list=1.0/m
#m=counts of classes of the whole original datasets

    class_list=[key for key in class_count]#获取数据集的类别列表
    model_pruned=MEP_result(copy.deepcopy(model),fea_list,copy.deepcopy(datasets),pae_lists,class_list,m)

    print"这里检查下数据集",datasets

    accuracy_unprune,accuracy_prune,misjudge_datasets=accuracy_analysis(model,model_pruned,copy.deepcopy(datasets),fea_list,name_path)
    print"accuracy_unprune=",accuracy_unprune
    print"accuracy_prune=",accuracy_prune
    for item in misjudge_datasets:
        print item

    print "model=",model
    print"model_pruned=",model_pruned
    createPlot(model)
    createPlot(model_pruned)

if __name__ == '__main__':
    m=2
    abalone_test(m)
    # credit_a_test(m)

