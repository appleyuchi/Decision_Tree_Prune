#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from sklearn.tree import _tree
from sklearn import tree
import collections
import drawtree
import os,copy
from sklearn.tree._tree import TREE_LEAF
from sklearn.datasets import load_boston
import json


from sklearn.tree import _tree
from sklearn.model_selection import train_test_split
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.tree import DecisionTreeRegressor
from sklearn.metrics import r2_score,mean_absolute_error,mean_squared_error
from sklearn2json import draw_file,rules,get_Attribute,read_data_for_split
import csv



#totally 506 items boston 
#１３个属性，１个特征




#T0->T1
#根据g(t)最小获得ｐｒｕｎｅｄ_parts（也就是要裁剪的部分），然后对当前模型进行剪枝。
def T1_create(model,gt_list,prune_parts,prune_gt_index):#完成,这个函数遍历了每一个节点
    if 'children' not in model:#如果是叶子节点
        return
    if nodes_all_count(model,0)==3:
        gt_list.append(gt_compute(model))
        prune_parts.append(model)

        if len(prune_parts)==prune_gt_index+1:
            del model["children"]
    else:
        children=model["children"]
        for child in children:
            T1_create(child,gt_list,prune_parts,prune_gt_index)


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


def prune_sklearn_model(sklearn_model,index,json_model):
    if "children" not in json_model:#json_model is the noly node of the tree.
        sklearn_model.children_left[index]=TREE_LEAF
        sklearn_model.children_right[index]=TREE_LEAF
    else:
        prune_sklearn_model(sklearn_model,sklearn_model.children_left[index],json_model["children"][0])
        prune_sklearn_model(sklearn_model,sklearn_model.children_right[index],json_model["children"][1])


def gt_with_tree(model,gt_list,prune_parts):#完成,这个函数遍历了每一个节点
    if 'children' not in model:#如果是叶子节点
        return

    #如果不是叶子节点
#如果不是最底层的节点（因为ＥＣＰ算法要求每次只能剪去两个叶子节点，而不能减去一个子树）
    if nodes_all_count(model,0)==3:#算法规定一次只能裁掉两个叶子
#加上当前节点的gt值以及保存当前“假设要裁掉”的树部分
        gt_list.append(gt_compute(model))#这个函数中的ｍｏｄｅｌ是要被裁剪掉的部分的后选项。
        prune_parts.append(model)
    children=model["children"]
    for child in children:
        if nodes_all_count(child,0)==3:#算法规定一次只能裁掉两个叶子
#加上当前节点的gt值以及保存当前“假设要裁掉”的树部分
            gt_list.append(gt_compute(child))#这个函数中的ｍｏｄｅｌ是要被裁剪掉的部分的后选项。
            prune_parts.append(child)
        else:
            gt_with_tree(child,gt_list,prune_parts)





def Tt_count(model,count):#because the tree can only be pruned with two leaves once
# Reference:<classification and regression trees>-Leo Breiman page 235th:
# "The pruning process in regression trees usually takes off only two terminal nodes at a time."
    return 1



def Rt_compute(model):#R(t)    
    print"再看看这里－－－－－－－－－－－－"
    return model["mse"]


def gt_compute(model):
    print"再看看这里－－－－－－－－－－－－"
    return (Rt_compute(model)-RTt_compute(model,0))*1.0
#             R(t)-R(Tt)
#g(t)=-----------------=MSE(t)-MSE(Tt)
#               |Tt|-1
# T0->a0
# T1->a1
# T2->a2
# T3->a3
# ···


def RTt_compute(model,RTt_MSE_pruned_parts):#剪枝前
#R(Tt)
#this function is modified from the above function "Tt_count"
    if "children"not in model:
        return Rt_compute(model)
    children=model["children"]
    N=0

    for child in children:
        N+=child['samples']*1.0

    RTt_MSE_parts=0.0
    for child in children:
        RTt_MSE_pruned_parts+=child['samples']*1.0/N*RTt_compute(child,0)

    return RTt_MSE_pruned_parts



def model_gtmin_Tt(dtr,model,feature_names,Tt_name):#还没修改完成
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

    print "gt_list=",gt_list
    print"prune_parts=",prune_parts
    print"len(prune_parts)=",len(prune_parts)
    # print"---------------------------------------------------------"
    # print len(gt_list)
    # print_list(prune_parts)
    # print"---------------------------------------------------------"
    # print len(prune_parts)
    print"model=",model
    alpha=min(gt_list)
    prune_gt_index=gt_list.index(alpha)
    # print"prune_gt_index=",prune_gt_index
    prune_for_minimum_gt=prune_parts[prune_gt_index]
    # print"prune_for_minimum_gt=\n",prune_for_minimum_gt
#------------------------------
    T0=copy.deepcopy(model)
    T1=copy.deepcopy(model)#here T1 means Ti
    gt_list=[]#这里必须复位清零
    pruned_parts=[]#这里必须复位清零
    T1_create(T1,gt_list,pruned_parts,prune_gt_index)
    print"pruned_parts=",pruned_parts

    index=0#never change this value！！！
    sklearn_model=copy.deepcopy(dtr)
    prune_sklearn_model(sklearn_model.tree_,index,T1)
    dot_file="./visualization/T"+Tt_name+".dot"
    png_file="./visualization/T"+Tt_name+".svg"
    # draw_file(sklearn_model,dot_file,png_file,X_train,feature_names)
    draw_file(sklearn_model,dot_file,png_file,feature_names)
    return sklearn_model,T1,alpha


def model_json(data_path,name_path,cart_max_depth):
##########################################################
    feature_names=get_Attribute(name_path)
    print"data_path=",data_path
#------------------------------------------
    x_list, y_list=read_data_for_split(data_path,n=0,label=1)#把数据和类别标签列分开。
    print"x_list=",x_list
    print"y_list=",y_list
#------------------------------------------
    X_train,X_test,y_train,y_test = train_test_split(x_list, y_list,test_size=0.25,random_state=0)
    print"X_train=",X_train
    #分别初始化对特征值和目标值的标准化器
    ss_X = StandardScaler()
    ss_y = StandardScaler()
    #训练数据都是数值型，所以要标准化处理
    X_train=np.array(X_train)
    print"X_train=",X_train
    X_train = ss_X.fit_transform(np.array(X_train))
    X_test=np.array(X_test)
    X_test = ss_X.transform(np.array(X_test))
    
    y_train=np.array(y_train)
    y_test=np.array(y_test)
    #目标数据（房价预测值）也是数值型，所以也要标准化处理
    #说明一下：fit_transform与transform都要求操作2D数据，而此时的y_train与y_test都是1D的，因此需要调用reshape(-1,1)，例如：[1,2,3]变成[[1],[2],[3]]
    y_train = ss_y.fit_transform(y_train.reshape(-1,1))
    y_test = ss_y.transform(y_test.reshape(-1,1))

    # print X_train
    feature_list=get_Attribute(name_path)

    dtr = DecisionTreeRegressor(max_depth=cart_max_depth,criterion='mse',random_state=0)
    print"now training,wait please.........."
    dtr.fit(X_train, y_train)
    print"train finished"
    class_names=''#因为是回归，所以不需要分类名
    result = rules(dtr, feature_list, class_names)
    print"result=",result
    with open('structure.json', 'w') as f:
        f.write(json.dumps(result))
    print"The json-style model has been stored in structure.json"

    print"now I'm drawing the CART Regression tree,wait please............"
    # print dir(data)
    dot_file="./visualization/T0.dot"
    png_file="./visualization/T0.svg"
    # draw_file(dtr,dot_file,png_file,X_train,feature_list)
    draw_file(dtr,dot_file,png_file,feature_list)
    print"CART tree has been drawn in "+png_file
    return dtr,result,X_train,y_train,X_test,y_test,feature_list

def ECP_TreeCandidate(clf,current_model,feature_names,alpha_list,Ti_list):#get the tree Sets with each minimum "g(t)"
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
        scikit_model,current_json_model,alpha=model_gtmin_Tt(scikit_model,current_json_model,feature_names,str(Tt_name))

        if "children" not in current_json_model:#only root node
            print"截止条件中的current_json_model=",current_json_model
            Ti_list.append(copy.deepcopy(current_json_model))
            alpha_list.append(alpha)
            Flag=False
            print"We have gotten the final Ti"

    return alpha_list,Ti_list


def ECP_top(name_path,data_path,max_depth,prune=True):
    clf,json_model,X_train,y_train,X_test,y_test,feature_list=model_json(data_path,name_path,max_depth)
    print"sklearn model has been transformed to json-style model=\n"
    if prune==False:
        return json_model,0,'you can calculate it with sklearn'

    else:
        alpha_list=[]
        Ti_list=[]    
        print"unpruned model=\n",json_model
        print"We are trying to get the Tree Sets,wait please.........."
        alpha_list,Ti_list=ECP_TreeCandidate(copy.deepcopy(clf),copy.deepcopy(json_model),feature_list,class_names,alpha_list,Ti_list)
        print"We have gotten all the Tree Sets.cross-validation is coming,wait please..............."
        # print"##########################################################"
        print"alpha_list=\n"
        print "alpha_list=",alpha_list
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
if __name__ == '__main__':
    data_path="./data/housing.data"
    name_path="./data/housing.names"
    cart_max_depth=3
    dtr,result,X_train,y_train,X_test,y_test,feature_list=model_json(data_path,name_path,cart_max_depth)
    alpha_list=[]
    Ti_list=[]    

    alpha_list,Ti_list=ECP_TreeCandidate(dtr,result,feature_list,alpha_list,Ti_list)
    print"alpha_list=",alpha_list
    print"Ti_list=",Ti_list

######################################################

