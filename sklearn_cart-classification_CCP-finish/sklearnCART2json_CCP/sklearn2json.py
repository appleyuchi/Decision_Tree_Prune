#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from sklearn.datasets import load_iris
from sklearn.tree import DecisionTreeClassifier
import json
import copy
import xlwt
import numpy as np
import pandas as pd
from sklearn.datasets import make_classification
from sklearn.ensemble import RandomForestClassifier
from sklearn.tree import DecisionTreeClassifier
from IPython.display import display, Image
import pydotplus
from sklearn import tree
from sklearn.tree import _tree
from sklearn import tree
import collections
import drawtree
import os  
from sklearn.tree._tree import TREE_LEAF
from pandas import DataFrame


from sklearn.model_selection import train_test_split
import csv

# 注意，使用时，默认最后一列是类别标签
# 并且假定第0行开始就是数据集


def str2float(X_train):
    for line in X_train:
        for index,item in enumerate(line):
            # print"item=",item
            try:
                line[index]=float(line[index])
            except:
                pass

def read_data_for_split(test_data='abalone.data',n=0,label=1):
    '''
    加载数据的功能
    n:特征数据起始位
    label：是否是监督样本数据
    '''
    csv_reader=csv.reader(open(test_data))
    data_list=[]
    for one_line in csv_reader:
        data_list.append(one_line)
    x_list=[]
    y_list=[]
    for one_line in data_list[n:]:
        if label==1:#有监督数据
            y_list.append(one_line[-1])   #标志位
            one_list=[o for o in one_line[n:-1]]
            x_list.append(one_list)
        else:#无监督数据
            one_list=[float(o) for o in one_line[n:]]
            x_list.append(one_list)
    str2float(x_list)
    return x_list, y_list

def split_data(data_list, y_list, ratio=0.30):
    '''
    按照指定的比例，划分样本数据集
    ratio: 测试数据的比率
    '''
    X_train, X_test, y_train, y_test = train_test_split(data_list, y_list, test_size=ratio, random_state=0)
    print '--------------------------------split_data shape-----------------------------------'
    print "训练集长度:",len(X_train), len(y_train)
    print "测试集长度：",len(X_test), len(y_test)
    str2float(X_train)
    str2float(X_test)
    print"分割后的情况：",type(X_train)
    X_train_class=copy.deepcopy(X_train)
    X_test_class=copy.deepcopy(X_test)
    X_train_class['class']=y_train
    X_test_class['class']=y_test

    class_names=X_train_class["class"].unique()
    class_names.sort()

    # print"X_train_class[class].unique()=",X_train_class["class"].unique()
    # print"X_test_class[class].unique()=",X_test_class["class"].unique()
    X_train_class.to_excel('./data_after_splits/X_train_class.xls')
    X_test_class.to_excel('./data_after_splits/X_test_class.xls')

    return X_train, X_test, y_train, y_test,class_names


def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list
def get_classname(path):
    class_names=[]
    for line in open(path):
        if"| classes" in line:
            class_names=line.split("| class")[0].split(",")
        break
    return class_names



def rules(clf, features, labels, node_index=0):
    """Structure of rules in a fit decision tree classifier

    Parameters
    ----------
    clf : DecisionTreeClassifier
        A tree that has already been fit.

    features, labels : lists of str
        The names of the features and labels, respectively.

    """
    node = {}
    if clf.tree_.children_left[node_index] == -1:  # 叶子节点
        count_labels = zip(clf.tree_.value[node_index, 0], labels)
        # print"labels=",labels
        # print"count_labels=",count_labels
        # print"clf.tree_.value=",clf.tree_.value[1]
        node['name'] = ', '.join(('{} of {}'.format(int(count), label)#所谓的class_name其实就是在这种地方用到了，这个class_names其实可以理解为类别的取值
                                  for count, label in count_labels))

        node['value']=[count for count,label in count_labels]#add by appleyuchi
        # print"node[value]=",node['value']
    else:

        count_labels = zip(clf.tree_.value[node_index, 0], labels)#add by appleyuchi
        node['value']=[count for count,label in count_labels]#add by appleyuchi

        feature = features[clf.tree_.feature[node_index]]
        threshold = clf.tree_.threshold[node_index]
        node['name'] = '{} <= {}'.format(feature, threshold)
        left_index = clf.tree_.children_right[node_index] 
        right_index =clf.tree_.children_left[node_index]
        node['children'] = [rules(clf, features, labels, right_index),
                            rules(clf, features, labels, left_index)]
    return node



def model_json(data_path,name_path,cart_max_depth):

    feature_names=get_Attribute(name_path)

#------------------------------------------
    x_list, y_list=read_data_for_split(data_path,n=0,label=1)
#------------------------------------------


    feature_names=get_Attribute(name_path)
    print "x_list[0]=",x_list[0]
    x_list_dummies = pd.get_dummies(DataFrame(x_list,columns=feature_names))
    # because CART can NOT deal with string ,so you need to pre-process here
    print"x_list_dummies=",x_list_dummies

    X_train,X_test,y_train,y_test,class_names=split_data(x_list_dummies,y_list)

    # print X_train
    feature_list=X_train.columns.values.tolist()

    clf = DecisionTreeClassifier(max_depth=cart_max_depth,criterion='entropy',random_state=0)
    print"now training,wait please.........."
    clf.fit(X_train, y_train)
    print"train finished"
    result = rules(clf, feature_list, class_names)
    with open('structure.json', 'w') as f:
        f.write(json.dumps(result))
    print"The json-style model has been stored in structur.json"

    print"now I'm drawing the CART tree,wait please............"
    # print dir(data)
    dot_file="./visualization/T0.dot"
    png_file="./visualization/T0.svg"
    # draw_file(clf,dot_file,png_file,X_train,feature_list)
    draw_file(clf,dot_file,png_file,feature_list)
    print"CART tree has been drawn in "+png_file
    return clf,result,X_train,y_train,X_test,y_test,feature_list,class_names


# def draw_file(model,dot_file,png_file,X_train,feature_names):
def draw_file(model,dot_file,png_file,feature_names):
    dot_data = tree.export_graphviz(model, out_file =dot_file ,
                                     feature_names=feature_names, filled   = True
                                        , rounded  = True
                                        , special_characters = True)
    
    graph = pydotplus.graph_from_dot_file(dot_file)  
    
    thisIsTheImage = Image(graph.create_png())
    display(thisIsTheImage)
    #print(dt.tree_.feature)
    
    from subprocess import check_call
    check_call(['dot','-Tsvg',dot_file,'-o',png_file])



def rules_test():
    data=load_iris()
    clf = DecisionTreeClassifier(max_depth=3,random_state=0)
    clf.fit(data.data, data.target)
    rules(clf,data.feature_names, data.target_names)
    dot_file="test.dot"
    png_file="test.png"
    draw_file(clf,dot_file,png_file,data.data,data.feature_names)
def test_abalone():
    name_path="./data/abalone.names"
    data_path="./data/abalone.data"
    cart_max_depth=4#don't be too large please.the ability of graphviz is limited.


    class_names=get_classname(name_path)
    print class_names

    model,models_json,X_train,y_train,X_test,y_test,feature_names,class_names=model_json(data_path,name_path,cart_max_depth)
    print"X_train=",X_train
    print"y_train=",y_train
    print"models_json=",models_json
    # print"model_json=",model_json

def test_credit_a():
    name_path="./data/crx.names"
    data_path="./data/crx_fix_unKnown.data"
    cart_max_depth=4#don't be too large please.the ability of graphviz is limited.


    class_names=get_classname(name_path)
    print class_names

    model,models_json,X_train,y_train,X_test,y_test,feature_names,class_names=model_json(data_path,name_path,cart_max_depth)
    print"X_train=",X_train
    print"y_train=",y_train
    print"models_json=",models_json
    # print"model_json=",model_json

if __name__ == '__main__':
    test_credit_a()


