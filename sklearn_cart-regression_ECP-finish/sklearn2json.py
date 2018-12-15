#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from sklearn.datasets import load_iris
from sklearn.tree import DecisionTreeClassifier
import json
import copy
import xlwt
import csv
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


def str2float(X_train):
    for line in X_train:
        for index,item in enumerate(line):
            # print"item=",item
            try:
                line[index]=float(line[index])
            except:
                pass

def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list
    
def draw_file(model,dot_file,png_file,feature_names):
    dot_data = tree.export_graphviz(model, out_file =dot_file ,feature_names=feature_names, filled   = True, rounded  = True, special_characters = True)
    graph = pydotplus.graph_from_dot_file(dot_file)  
    thisIsTheImage = Image(graph.create_png())
    display(thisIsTheImage)
    #print(dt.tree_.feature)
    from subprocess import check_call
    check_call(['dot','-Tsvg',dot_file,'-o',png_file])



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
        # count_labels = zip(clf.tree_.value[node_index, 0], labels)
        count_labels=clf.tree_.value[node_index,0]
        # node['value']=[count for count,label in count_labels]#add by appleyuchi
        node['value']=[count for count in count_labels][0]
        node['mse']=clf.tree_.impurity[node_index]
        node['samples']=clf.tree_.n_node_samples[node_index]
    else:#非叶子节点
        # count_labels = zip(clf.tree_.value[node_index, 0], labels)#add by appleyuchi
        # node['value']=[count for count,label in count_labels]#add by appleyuchi
        count_labels=clf.tree_.value[node_index,0]
        node['value']=[count for count in count_labels][0]
        node['mse']=clf.tree_.impurity[node_index]
        feature = features[clf.tree_.feature[node_index]]
        threshold = clf.tree_.threshold[node_index]
        left_index = clf.tree_.children_right[node_index] 
        right_index =clf.tree_.children_left[node_index]
        node['name'] = '{} <= {}'.format(feature, threshold)
        node['children'] = [rules(clf, features, labels, right_index),rules(clf, features, labels, left_index)]
        node['samples']=clf.tree_.n_node_samples[node_index]
    return node


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
if __name__ == '__main__':
    data_path="./data/housing.data"
    x_list, y_list= read_data_for_split(data_path,n=0,label=1)
    print"x_list=",len(x_list)
    print"y_list=",len(y_list)