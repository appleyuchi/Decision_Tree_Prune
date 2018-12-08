#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
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


def train():
    X, y = make_classification(n_samples=1000,
                               n_features=6,
                               n_informative=3,
                               n_classes=2,
                               random_state=0,
                               shuffle=False)
    print"y=",y
    # Creating a dataFrame
    df = pd.DataFrame({'Feature 1':X[:,0],
                           'Feature 2':X[:,1],
                           'Feature 3':X[:,2],
                           'Feature 4':X[:,3],
                           'Feature 5':X[:,4],
                           'Feature 6':X[:,5],
                           'Class':y})
    y_train = df['Class']
    X_train = df.drop('Class',axis = 1)
    
    

    
    dt = DecisionTreeClassifier( random_state=42)                
    dt.fit(X_train, y_train)
    return dt,X_train
#------------------上面是生成决策树模型-----------------------------------
# os.environ["PATH"] += os.pathsep + 'C:\\Anaconda3\\Library\\bin\\graphviz'
def draw_file(model,dot_file,png_file,X_train):
    dot_data = tree.export_graphviz(model, out_file =dot_file ,
                                     feature_names=X_train.columns, filled   = True
                                        , rounded  = True
                                        , special_characters = True)
    
    graph = pydotplus.graph_from_dot_file(dot_file)  
    
    thisIsTheImage = Image(graph.create_png())
    display(thisIsTheImage)
    #print(dt.tree_.feature)
    
    from subprocess import check_call
    check_call(['dot','-Tpng',dot_file,'-o',png_file])

# 剪枝函数（这里使用的不是著名的CCP剪枝，而是根据的当前的子树剩余的样本数是否超过阈值，如果小于阈值，就进行剪枝）
def prune_index(inner_tree, index, threshold):
    if inner_tree.value[index].min() < threshold:
        # turn node into a leaf by "unlinking" its children
        inner_tree.children_left[index] = TREE_LEAF#对左子树进行剪枝操作
        inner_tree.children_right[index] = TREE_LEAF#对右子树进行剪枝操作
    # if there are shildren, visit them as well
    if inner_tree.children_left[index] != TREE_LEAF:
        prune_index(inner_tree, inner_tree.children_left[index], threshold)#对左子树进行递归
        prune_index(inner_tree, inner_tree.children_right[index], threshold)#对右子树进行递归

#***************************************************************



if __name__ == '__main__':
    model,X_train=train()
    dot_file='unprunedtree.dot'
    png_file='unprunedtree.png'
    draw_file(model,dot_file,png_file,X_train)
    
    print(sum(model.tree_.children_left < 0))
    print"************************************************"
    print model.tree_.value
    print type(model.tree_)
    print dir(model)
    print dir(model.tree_)
    # prune_index(model.tree_, 0, 5)
    # dot_file='prunedtree.dot'
    # png_file='prunedtree.png'
    # print"当前的model是",model
    # draw_file(model,dot_file,png_file,X_train)
    # print sum(model.tree_.children_left < 0)
