#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

from decisionTree import DecisionTree
from decisionTreeC45 import DecisionTreeC45
from decisionTreeCART import DecisionTreeCART

import pandas as pd

import random as rnd

import time

import traceback

def inorder_traversal(treeNode):
    childNodes = treeNode.children
    if childNodes is None or len(childNodes) == 0:
        print(treeNode.category + '\n')
        return
    for key in childNodes:
        if childNodes[key].feature is None:
            print("key:" + key + " category:" + str(childNodes[key].category) + "\n")
        else:
            print("key:" + key + " feature:" + childNodes[key].feature + "\n")
            inorder_traversal(childNodes[key])
    return

def test_decision_tree(Y_test, tree):
    count = 0
    Y_test['Survived'] = None
    nums = Y_test.shape[0]
    for i in range(nums):
        row = Y_test.loc[i]
        find_category(row, tree)
        Y_test.loc[i, 'Survived'] = row['Survived']
        # count += 1
        # print(count)
    print('done')

def find_category(row, treeNode):
    childNodes = treeNode.children
    if treeNode.feature is None:
        row['Survived'] = treeNode.category
        return
    else:
        node = childNodes.get(row[treeNode.feature])
        if node is None:
            row['Survived'] = rnd.randint(0, 1)
            return
        else:
            find_category(row, node)
            return

def test_decision_treeCART(Y_test, tree):
    count = 0
    Y_test['Survived'] = None
    nums = Y_test.shape[0]
    for i in range(nums):
        row = Y_test.loc[i]
        find_categoryCART(row, tree)
        Y_test.loc[i, 'Survived'] = row['Survived']
        # count += 1
        # print(count)
    print('done')

def find_categoryCART(row, treeNode):
    childNodes = treeNode.children
    if treeNode.feature is None:
        row['Survived'] = treeNode.category
        return
    else:
        node = childNodes.get(row[treeNode.feature])
        if node is None:
            node = childNodes.get(list(childNodes.keys())[1])
        find_categoryCART(row, node)
        return

if __name__ == '__main__':
    # 西瓜数据集
    # X_data = train_data.drop('haogua', axis=1)
    # Y_data = train_data['haogua']

    # Titanic训练数据
    train_data = pd.read_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\data\train_dealt.csv')
    Y_data = train_data['Survived']
    X_data = train_data.drop('Survived', axis=1)

    # 计算运行时间
    start = time.clock()
    # ID3算法
    # tree = DecisionTree(X_data, Y_data).root_node
    # 计算回归值
    # Y_test = pd.read_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\data\train_dealt_test.csv')
    # test_decision_tree(Y_test, tree)
    # Y_data_predict = Y_test['Survived']
    # count = 0
    # for i in range(len(Y_data)):
    #     if Y_data.loc[i] == Y_data_predict.loc[i]:
    #         count += 1
    # epsilon = str(0)
    # with open("out/text/Titanic_ID3.txt", "a") as f:
    #     f.write("\n-----------------------------------------------------------------------\n")
    #     f.write("阈值epsilon: %s\n" % (epsilon))
    #     f.write("相同个数: %d    总个数: %d\n" % (count, len(Y_data)))
    #     f.write("回归值: %f\n" % (count/len(Y_data)))
    #     f.write("-----------------------------------------------------------------------")
    # f.close()
    # print(count)
    # print(count/len(Y_data))
    # 做预测
    # Y_test = pd.read_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\data\test_dealt.csv')
    # test_decision_tree(Y_test, tree)
    # Y_test = Y_test[['PassengerId', 'Survived']]
    # Y_test.to_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\out\data\test_dealt_out_ID3_modi.csv', index=False)


    # C4.5算法
    # tree = DecisionTreeC45(X_data, Y_data).root_node
    #计算回归值
    # Y_test = pd.read_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\data\train_dealt_test.csv')
    # test_decision_tree(Y_test, tree)
    # Y_data_predict = Y_test['Survived']
    # count = 0
    # for i in range(len(Y_data)):
    #     if Y_data.loc[i] == Y_data_predict.loc[i]:
    #         count += 1
    # epsilon = str(0.1)
    # with open("out/text/Titanic_C45.txt", "a") as f:
    #     f.write("\n-----------------------------------------------------------------------\n")
    #     f.write("阈值epsilon: %s\n" % (epsilon))
    #     f.write("相同个数: %d    总个数: %d\n" % (count, len(Y_data)))
    #     f.write("回归值: %f\n" % (count/len(Y_data)))
    #     f.write("-----------------------------------------------------------------------")
    # f.close()
    # print(count)
    # print(count/len(Y_data))
    # 做预测
    # Y_test = pd.read_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\data\test_dealt.csv')
    # test_decision_tree(Y_test, tree)
    # Y_test = Y_test[['PassengerId', 'Survived']]
    # Y_test.to_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\out\data\test_dealt_out_C45_modi.csv', index=False)

    # CART算法
    tree = DecisionTreeCART(X_data, Y_data).root_node
    # 计算回归值
    # Y_test = pd.read_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\data\train_dealt_test.csv')
    # test_decision_treeCART(Y_test, tree)
    # Y_data_predict = Y_test['Survived']
    # count = 0
    # for i in range(len(Y_data)):
    #     if Y_data.loc[i] == Y_data_predict.loc[i]:
    #         count += 1
    # # epsilon = str(0.1)
    # with open("out/text/Titanic_CART.txt", "a") as f:
    #     f.write("\n-----------------------------------------------------------------------\n")
    #     # f.write("阈值epsilon: %s\n" % (epsilon))
    #     f.write("相同个数: %d    总个数: %d\n" % (count, len(Y_data)))
    #     f.write("回归值: %f\n" % (count/len(Y_data)))
    #     f.write("-----------------------------------------------------------------------")
    # f.close()
    # print(count)
    # print(count/len(Y_data))
    # 做预测
    # Y_test = pd.read_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\data\test_dealt.csv')
    # test_decision_treeCART(Y_test, tree)
    # Y_test = Y_test[['PassengerId', 'Survived']]
    # Y_test.to_csv(r'E:\learn\Machine_Learning_course\paper&code\decision_tree\out\data\test_dealt_out_CART_modi.csv', index=False)

    # 算法结束
    elapsed = (time.clock() - start)
    print("Time used:", elapsed)