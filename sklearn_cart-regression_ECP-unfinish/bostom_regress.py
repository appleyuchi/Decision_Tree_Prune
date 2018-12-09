#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from sklearn.tree import _tree
from sklearn import tree
import collections
import drawtree
import os  
from sklearn.tree._tree import TREE_LEAF
from sklearn.datasets import load_boston


from sklearn.tree import _tree
from sklearn.model_selection import train_test_split
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.tree import DecisionTreeRegressor
from sklearn.metrics import r2_score,mean_absolute_error,mean_squared_error
#这里有MSE,到时候查一下，还有哪些指标是sklearn提供的！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

#totally 506 items boston 


boston = load_boston()
# print dir(boston)
 # DESCR,feature_names,filename,target






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




def model2json():
    #第二步：波士顿房价数据分割
    
    X_train,X_test,y_train,y_test = train_test_split(boston.data,boston.target,test_size=0.25,random_state=33)
    
    print"-----------------------看一下这些数据------------------------"
    # print"X_train=",X_train
    #analyse the differece among the target value.
    # print 'The max target value is ',np.max(boston.target)
    # print 'The min target value is ',np.min(boston.target)
    # print 'The average target value is ',np.mean(boston.target)
    
    #第三步：训练数据和测试数据标准化处理
    
    #分别初始化对特征值和目标值的标准化器
    ss_X = StandardScaler()
    ss_y = StandardScaler()
    #训练数据都是数值型，所以要标准化处理
    X_train = ss_X.fit_transform(X_train)
    X_test = ss_X.transform(X_test)
    #目标数据（房价预测值）也是数值型，所以也要标准化处理
    #说明一下：fit_transform与transform都要求操作2D数据，而此时的y_train与y_test都是1D的，因此需要调用reshape(-1,1)，例如：[1,2,3]变成[[1],[2],[3]]
    y_train = ss_y.fit_transform(y_train.reshape(-1,1))
    y_test = ss_y.transform(y_test.reshape(-1,1))

    
    
    
    #第四步：使用单一回归树模型进行训练，并且对测试数据进行预测
    
    #1.初始化k近邻回归器，并且调整配置，使得预测方式为平均回归：weights = 'uniform'
    dtr = DecisionTreeRegressor(max_depth=2)
    dtr.fit(X_train,y_train)
    result = rules(dtr,boston.feature_names,boston.target, node_index=0)

    print"json model of CART regression tree=",result
    print"---------------the information we can get from sklearn---------------"
    print dir(dtr.tree_)
    dtr_y_predict = dtr.predict(X_test)
    
    #第五步：对单一回归树模型在测试集下进行性能评估
    #使用R-squared、MSE、MAE指标评估
    
    
    # print 'R-squared value of DecisionTreeRegressor is',dtr.score(X_test,y_test)
    # print 'the MSE of DecisionTreeRegressor is',mean_squared_error(ss_y.inverse_transform(y_test),ss_y.inverse_transform(dtr_y_predict))
    # print 'the MAE of DecisionTreeRegressor is',mean_absolute_error(ss_y.inverse_transform(y_test),ss_y.inverse_transform(dtr_y_predict))
    


if __name__ == '__main__':
    model2json()