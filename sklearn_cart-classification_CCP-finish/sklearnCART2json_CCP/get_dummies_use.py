# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-12-07 16:49:49
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-12-07 17:32:18
from sklearn import *
import pydotplus
from IPython.display import display, Image
# pip install --force-reinstall scikit-learn==0.20rc1


'''

S现在主要的问题就是get_dummies前后补充的数据的位置
以及feature_names的增加
以及class_name是否有必要
'''
from sklearn.datasets import load_iris
from sklearn.tree import DecisionTreeClassifier
import numpy as np
import pandas as pd


def draw_file(model,dot_file,png_file,X_train,feature_names):
    dot_data = tree.export_graphviz(model, out_file =dot_file ,
                                     feature_names=feature_names, filled   = True
                                        , rounded  = True
                                        , special_characters = True)
    
    graph = pydotplus.graph_from_dot_file(dot_file)  
    
    thisIsTheImage = Image(graph.create_png())
    display(thisIsTheImage)
    #print(dt.tree_.feature)
    
    from subprocess import check_call
    check_call(['dot','-Tpng',dot_file,'-o',png_file])


def read_data(path):
    source_data=pd.read_csv(path,header=None)#第一行不是标题
    lists=source_data.values.tolist()
    return lists
def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list



from pandas.core.frame import DataFrame
name_path="./data/abalone.names"
data_path="./data/abalone.data"
# 想办法处理这个数据，并且绘制出出图形
feature_list=get_Attribute(name_path)
print "feature_list=",feature_list
data=read_data(data_path)
y=[item[-1]for item in data]
for item in data:
    # print type(item[0])
    item.pop(-1)

print len(data[0])

print "type(data)=",type(data)
print"data[0]=",data[0]
data = pd.get_dummies(DataFrame(data,columns=feature_list))


# feature_list=data.columns.values.tolist()
# # print dir(data)
# print "feature_list=",feature_list

# clf = DecisionTreeClassifier(max_depth=100, random_state=0) #定义最大深度和确定随机种子
# print"y=",y
# clf.fit(data, y)  #训练
# dot_file="test.dot"
# png_file="test.png"
# draw_file(clf,dot_file,png_file,data,feature_list)