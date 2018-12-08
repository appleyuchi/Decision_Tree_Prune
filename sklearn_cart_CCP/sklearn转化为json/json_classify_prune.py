# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-12-06 13:29:13
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-12-08 16:54:03
from sklearn2json import model_json,draw_file
import pandas as pd
import numpy as np
import re
import copy
from sklearn.tree._tree import TREE_LEAF



def classify(json_model,feature_names_list,test_data):
    # print"current model=",model
    if "children" not in json_model:
        return json_model["value"]#到达叶子节点，完成测试

    bestfeature=json_model["name"].split("<=")[0].strip()
    threshold=float(json_model["name"].split(bestfeature+" <= ")[1].strip())
    test_best_feature_value=test_data[feature_names_list.index(bestfeature)]
    # print"test_best_feature_value=",test_best_feature_value
    if  float(test_best_feature_value)<=threshold:
        child=json_model["children"][0]
        # print"child=",child["name"]

        result=classify(child,feature_names_list,test_data)
    else:
        child=json_model["children"][1]
        result=classify(child,feature_names_list,test_data)

    return result


# ['1.45', '5', '6.45', '8.82']

def predict(json_model,feature_names,class_names,test_item):
    # print"进入predict时的测试数据",test_item
    leaf_value=classify(json_model,feature_names,test_item)
    # print"最终叶子节点是=",leaf_value
    # result,class_names_index=major(result,class_names)
    class_names_index=leaf_value.index(max(leaf_value))
    # print"class_names=",class_names
    result=class_names[class_names_index]
    return result,class_names_index

def precision_compute(json_model,X_test,y,feature_names,class_names):
    count_right=0.0
    # print"进入precision_compute的数据是=",X_test
    # print type(X_test)
#---------Dataframe->list--------------
    X_test = np.array(X_test)
    X_test=X_test.tolist()
#-----------------------
    for index,item in enumerate(X_test):
        # print"测试数据=",item
        predict_result,class_name_index=predict(json_model,feature_names,class_names,item)
        # print"predict_result=",predict_result
        # print"label[class_name_index]=",y[class_name_index]
        if class_names[class_name_index]==str(y[index]):
            count_right+=1
    #     else:
    #         print"测试失败数据为：",item
    #         print"实际结果为",y[index]
    #         print"预测结果为",class_names[class_name_index]
    # print"测试准确的有:",count_right
    return count_right/len(X_test)

##############################################################
def iris_model_unprune_analysis():
    #list[0] True
    #list[0] False
    clf,model,X_train,feature_names,label,class_names=model_json()
    
    accuracy=precision_compute(model,X_train,label,feature_names,class_names)
    print"当前模型准确度=",accuracy

    print"--------test one item-----------------------------"
    #feature_names= ['sepal length (cm)', 'sepal width (cm)', 'petal length (cm)', 'petal width (cm)']
    test_data=[4.6,3.6,3,0.2]
    # model={"name": "petal length (cm) > 2.45000004768", "children": [{"name": "petal width (cm) > 1.75", "children": [{"name": "petal length (cm) > 4.85000038147", "children": [{"name": "0 of setosa, 0 of versicolor, 43 of virginica"}, {"name": "0 of setosa, 1 of versicolor, 2 of virginica"}]}, {"name": "petal length (cm) > 4.94999980927", "children": [{"name": "0 of setosa, 2 of versicolor, 4 of virginica"}, {"name": "0 of setosa, 47 of versicolor, 1 of virginica"}]}]}, {"name": "50 of setosa, 0 of versicolor, 0 of virginica"}]}
    result,class_names_index=predict(model,feature_names,class_names,test_data)
    print"预测结果result=",result

    
#########################parameters for CCP#####################################


def Tt_count(model,count):#|Tt|
    if "children" not in model:
        return 1
    children=model["children"]
    for child in children:
        count+=Tt_count(child,0)
    return count

def Rt_compute(model):#R(t)
    Rt=(sum(model['value'])-max(model['value']))
    return Rt

def RTt_compute(model,leaves_error_count):
#R(Tt)
#this function is modified from the above function "Tt_count"
    if "children"not in model:
        return Rt_compute(model)
    children=model["children"]
    for child in children:
        leaves_error_count+=RTt_compute(child,0)
    return leaves_error_count



def gt_compute(model):
    return (Rt_compute(model)-RTt_compute(model,0)*1.0)/(Tt_count(model,0)-1)
#             R(t)-R(Tt)
#g(t)=-----------------
#               |Tt|-1
# T0->a0
# T1->a1
# T2->a2
# T3->a3
# ···


#model=T0
#for example,to get T1,we need to know which node of T0 has the minimum g(t)


def nodes_all_count(model,count):#完成,这个函数遍历了每一个节点
    if 'children' not in model:
        count+=1
        return count
    else:
        count+=1
    children=model["children"]
    for child in children:
        print"child=",child
        count+=nodes_all_count(child,0)
    return count


#     1
#    /\
#   2 3
# /\  /\
# 45 6 7
#for example,if the above is the original T0
#then "prune_parts" can be:
# 3
# /\
# 6 7

def gt_with_tree(model,gt_list,prune_parts):#完成,这个函数遍历了每一个节点
    if 'children' not in model:#如果是叶子节点
        return
    else:#加上当前节点的gt值以及保存当前“假设要裁掉”的树部分
        gt_list.append(gt_compute(model))
        prune_parts.append(model)
        children=model["children"]
        for child in children:
            gt_with_tree(child,gt_list,prune_parts)

# XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


#T0->T1
def T1_create(model,gt_list,prune_parts,prune_gt_index):#完成,这个函数遍历了每一个节点
    if 'children' not in model:#如果是叶子节点
        return
    else:#加上当前节点的gt值以及保存当前“假设要裁掉”的树部分
        gt_list.append(gt_compute(model))
        prune_parts.append(model)
        children=model["children"]
        if len(prune_parts)==prune_gt_index+1:
            del model["children"]
        for child in children:
            T1_create(child,gt_list,prune_parts,prune_gt_index)



#☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆modify sklearn-model synchronized with json-model☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
def prune_sklearn_model(sklearn_model,index,json_model):
    if "children" not in json_model:#json_model is the noly node of the tree.
        sklearn_model.children_left[index]=TREE_LEAF
        sklearn_model.children_right[index]=TREE_LEAF
    else:
        prune_sklearn_model(sklearn_model,sklearn_model.children_left[index],json_model["children"][0])
        prune_sklearn_model(sklearn_model,sklearn_model.children_right[index],json_model["children"][1])


def print_list(lists):
    for item in lists:
        print item
        print '\n'

#------------------------获取数据和sklearn、json模型--------------------------------


# Tt_name需要在每次循环中叠加
# 这个函数的目的是产生下一个alpha，以及下一个T
# def model_gtmin_Tt(clf,model,X_train,feature_names,class_names,Tt_name):
def model_gtmin_Tt(clf,model,feature_names,class_names,Tt_name):
    # print"model=",model
    Tt=Tt_count(model,0)#|Tt|
    # print"|Tt|=",Tt
    Rt=Rt_compute(model)
    # print"R(t)=",Rt

    RTt=RTt_compute(model,0)
    # print"R(Tt)=",RTt

    # leaves=nodes_all_count(model,0)#no use now
    # print"all nodes=",leaves
    gt_list=[]
    prune_parts=[]
    gt_with_tree(model,gt_list,prune_parts)

    # print gt_list
    # print"---------------------------------------------------------"
    # print len(gt_list)
    # print_list(prune_parts)
    # print"---------------------------------------------------------"
    # print len(prune_parts)
    alpha=min(gt_list)
    prune_gt_index=gt_list.index(alpha)
    # print"prune_gt_index=",prune_gt_index
    prune_for_minimum_gt=prune_parts[prune_gt_index]
    # print"prune_for_minimum_gt=\n",prune_for_minimum_gt
#------------------------------
    T0=copy.deepcopy(model)
    T1=copy.deepcopy(model)#here T1 means Ti
    gt_list=[]#这里必须复位清零
    prune_parts=[]#这里必须复位清零
    T1_create(T1,gt_list,prune_parts,prune_gt_index)#from T0(original model) to get T1
    # print"\nT0=",model
    # print "\nT1=",T1

    index=0#never change this value！！！
    sklearn_model=copy.deepcopy(clf)
    prune_sklearn_model(sklearn_model.tree_,index,T1)
    dot_file="./visualization/T"+Tt_name+".dot"
    png_file="./visualization/T"+Tt_name+".svg"
    # draw_file(sklearn_model,dot_file,png_file,X_train,feature_names)
    draw_file(sklearn_model,dot_file,png_file,feature_names)
    return sklearn_model,T1,alpha


# 遍历当前树，获取Ti,然后返回裁剪后的树，进行下一轮的Ti-1



#------------------------------
def CCP_TreeCandidate(clf,current_model,feature_names,class_names,alpha_list,Ti_list):#get the tree Sets with each minimum "g(t)"
    Flag=True
    alpha=0
    Tt_name=0
    scikit_model=copy.deepcopy(clf)
    current_json_model=copy.deepcopy(current_model)
    print"current_json_model=",current_json_model


    while Flag:
        alpha_list.append(alpha)
        Ti_list.append(current_json_model)
        print"We have gotten the final T"+str(Tt_name)+", wait please......."

        Tt_name=Tt_name+1
        scikit_model,current_json_model,alpha=model_gtmin_Tt(scikit_model,current_json_model,feature_names,class_names,str(Tt_name))

        if "children" not in current_json_model:#only root node
            print"截止条件中的current_json_model=",current_json_model
            Ti_list.append(copy.deepcopy(current_json_model))
            alpha_list.append(alpha)
            Flag=False
            print"We have gotten the final Ti"

    return alpha_list,Ti_list
#------------------------------
#the final step.
def CCP_cross_validation(TreeSets,alpha_list,X_test,y_test,feature_names,class_names,sklearn_model):
    precision_list=[]
    progress_length=len(TreeSets)
    # print"------------------------------检查下这里------------------------------"
    # print"X_test,y_test=",X_test
    # print y_test
    for index,item in enumerate(TreeSets):
        Ti_precision=precision_compute(item,X_test,y_test,feature_names,class_names)
        print"Ti_precision=",Ti_precision
        precision_list.append(Ti_precision)
        print"the T"+str(index)+" has been validated, "+str(progress_length-index-1)+" Trees left, wait please....."

    pruned_precision=max(precision_list)
    index=precision_list.index(pruned_precision)
    best_alpha=alpha_list[index]
    Best_tree=TreeSets[index]
    dot_file="./visualization/Best_tree.dot"
    svg_file="./visualization/Best_tree.svg"
    #画一画树
    # draw_file(sklearn_model,dot_file,svg_file,X_train,feature_names)
    draw_file(sklearn_model,dot_file,svg_file,feature_names)
    return Best_tree,best_alpha,pruned_precision


def CCP_top(name_path,data_path,max_depth,prune=True):
    # name_path="./data/abalone.names"
    # data_path="./data/abalone.data"
    clf,json_model,X_train,y_train,X_test,y_test,feature_list,class_names=model_json(data_path,name_path,max_depth)
    print"sklearn model has been transformed to json-style model=\n"
    if prune==False:
        return json_model,0,'you can calculate it with sklearn'

    else:
        alpha_list=[]
        Ti_list=[]    
        print"unpruned model=\n",json_model
        print"We are trying to get the Tree Sets,wait please.........."
        alpha_list,Ti_list=CCP_TreeCandidate(copy.deepcopy(clf),copy.deepcopy(json_model),feature_list,class_names,alpha_list,Ti_list)
        print"We have gotten all the Tree Sets.cross-validation is coming,wait please..............."
        # print"##########################################################"
        # print"alpha_list=\n"
        # print_list(alpha_list)
        # print alpha_list
        # print"##########################################################"
        # print"Ti_list=\n"
        # print_list(Ti_list)
        Best_tree,best_alpha,pruned_precision=CCP_cross_validation(Ti_list,alpha_list,X_test,y_test,feature_list,class_names,copy.deepcopy(clf))
        print"\n"
        print"Best_tree=",Best_tree
        print"\n"
        print"best_alpha=",best_alpha
        print"\n"
        print"pruned_precision=",pruned_precision
        return Best_tree,best_alpha,pruned_precision


def test_abalone():
    name_path="./data/abalone.names"
    data_path="./data/abalone.data"
    max_depth=2
    Best_tree,best_alpha,pruned_precision=CCP_top(name_path,data_path,max_depth,prune=True)
def test_credit_a():
    name_path="./data/crx.names"
    data_path="./data/crx.data"
    max_depth=2
    Best_tree,best_alpha,pruned_precision=CCP_top(name_path,data_path,max_depth,prune=True)

if __name__ == '__main__':
    # test_abalone()
    # Note:cart on abalone has low precision,because this task is preferred to be dealed with regression tree,
    # Not classification Tree.
    # To improve the precision ,you can set the "max_depth" larger
    test_credit_a()







