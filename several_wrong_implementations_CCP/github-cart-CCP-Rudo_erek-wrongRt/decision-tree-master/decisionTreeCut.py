#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

import pandas as pd
import time

from copy import deepcopy

from decisionTreeCART import DecisionTreeCART
from plotTree import PlotTree
from main import test_decision_treeCART


def BFS(tree):
    nodes = list()
    if tree.children is None:
        return None
    for child in tree.children:
        nodes.append(tree.children.get(child))
    return nodes

def compute_gini(Y):
    gini = 1
    for cate in Y.value_counts(1):
        gini -= cate * cate
    return gini

def tree_compute_gini(root_node):
    tree_gini = 0
    leaf_nodes = 0
    if root_node.feature is None:
        tree_gini = compute_gini(root_node.Y_data)
        leaf_nodes = 1
        return tree_gini, leaf_nodes
    else:
        for child in root_node.children:
            a, b = tree_compute_gini(root_node.children.get(child))
            tree_gini += a
            leaf_nodes += b
        return tree_gini, leaf_nodes


# 先由CART得到一颗决策树
# 西瓜数据集
# train_data = pd.read_csv('data/watermelon_data.csv')
# X_data = train_data.drop('haogua', axis=1)
# Y_data = train_data['haogua']

# Titanic 数据集
train_data_1 = pd.read_csv('data/CV-3/train_dealt_1.csv')
train_data_2 = pd.read_csv('data/CV-3/train_dealt_2.csv')
train_data = pd.concat([train_data_1, train_data_2], axis=0)
Y_data = train_data['Survived']
X_data = train_data.drop('Survived', axis=1)

# 计算运行时间
start = time.clock()

tree = DecisionTreeCART(X_data, Y_data).root_node

print"tree=",tree

pt = PlotTree()
pt.createPlot(tree)







# # 剪枝算法
# k = 0
# child_tree_list = list()
# T = deepcopy(tree)

# # 循环计算 alpha
# while True:
# # for j in range(2):
#     # 设置 alpha 为无穷大
#     alpha = float('inf')

#     # 宽度优先搜索，遍历所有的结点
#     BFS_nodes = list()
#     BFS_nodes.append(T)
#     i = 0
#     while len(BFS_nodes) > i:
#         child_nodes = BFS(BFS_nodes[i])
#         if child_nodes is not None:
#             BFS_nodes += child_nodes
#         i += 1

#     # 自下而上对各内部结点计算 g(t) 以及更新 alpha 的值
#     # g_t_list 记录各结点的 g(t) 值
#     nodes_num = len(BFS_nodes)
#     g_t_list = [float('inf') for i in range(nodes_num)]
#     for i in range(1, nodes_num):
#         if BFS_nodes[-i].feature is None or BFS_nodes[-i].X_data is None or len(BFS_nodes[-i].X_data)==0:#如果是叶子节点，就继续
#             continue
#         else:
#             curr_root_node = BFS_nodes[-i]#这个大概是倒数第几个节点的意思
#             C_t = compute_gini(curr_root_node.Y_data)
#             C_Tt, abs_Tt = tree_compute_gini(curr_root_node)
#             g_t = (C_t - C_Tt) / (abs_Tt - 1)
#             g_t_list[-i] = g_t
#             alpha = min(alpha, g_t)#

#     # 对 g(t) = alpha 的内部结点 t 进行剪枝，并对结点 t 以多数表决法决定其类，得到树 T
#     for i in range(1, nodes_num):
#         if g_t_list[-i] == alpha:
#             curr_node = BFS_nodes[-i]
#             curr_node.feature = None
#             curr_node.children = None
#             curr_node.category = curr_node.Y_data.value_counts(ascending=False).keys()[0]

#     # 把 T 放入子树列表
#     child_tree_list.append(T)

#     T = deepcopy(T)
#     T_children = T.children
#     keys = list(T_children.keys())
#     if T_children.get(keys[0]).children is None and T_children.get(keys[1]).children is None:
#         break
# #---------------------------------------------------------------

# # 验证
# best_tree = child_tree_list[0]
# best_good_value = 0
# for T in child_tree_list:
#     train_data = pd.read_csv("./data/CV-3/train_dealt_3.csv")
#     Y_data = train_data['Survived']
#     X_data = train_data.drop('Survived', axis=1)

#     # 计算回归值
#     # Y_test = pd.read_csv("./data/CV-3/train_dealt_test.csv")
#     Y_test = pd.read_csv("./data/train_dealt_test.csv")
#     test_decision_treeCART(Y_test, T)
#     Y_data_predict = Y_test['Survived']
#     count = 0
#     for i in range(len(Y_data)):
#         if Y_data.loc[i] == Y_data_predict.loc[i]:
#             count += 1
#     # with open("out/text/Titanic_CART_Cut.txt", "a") as f:
#     #     f.write("\n-----------------------------------------------------------------------\n")
#     #     f.write("相同个数: %d    总个数: %d\n" % (count, len(Y_data)))
#     #     f.write("回归值: %f\n" % (count/len(Y_data)))
#     #     f.write("-----------------------------------------------------------------------")
#     # f.close()
#     # print(count)
#     # print(count/len(Y_data))
#     if count/len(Y_data) >= best_good_value:
#         best_tree = T
#         best_good_value = count/len(Y_data)

# # 运行结束
# elapsed = (time.clock() - start)
# print("Time used:", elapsed)
# # 做预测
# Y_test = pd.read_csv("./data/test_dealt.csv")
# test_decision_treeCART(Y_test, best_tree)
# Y_test = Y_test[['PassengerId', 'Survived']]
# Y_test.to_csv("./test_dealt_out_CART_Cut_1.csv", index=False)


# pt = PlotTree()
# pt.createPlot(best_tree)
