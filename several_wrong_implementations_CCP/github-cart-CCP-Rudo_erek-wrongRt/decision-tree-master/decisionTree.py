#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')


from treeNode import TreeNode
import pandas as pd
import numpy as np


class DecisionTree:

    def __init__(self, X, Y):
        self.X_train = X
        self.Y_train = Y
        self.root_node = TreeNode(None, None, None, None, self.X_train, self.Y_train)
        self.features = self.get_features(self.X_train)
        self.tree_generate(self.root_node)

    def get_features(self, X_train_data):
        features = dict()
        for i in range(len(X_train_data.columns)):
            feature = X_train_data.columns[i]
            features[feature] = list(X_train_data[feature].value_counts().keys())

        return features

    def tree_generate(self, tree_node):
        X_data = tree_node.X_data
        Y_data = tree_node.Y_data
        # get all features of the data set
        features = list(X_data.columns)
        # 如果Y_data中的实例属于同一类，则置为单结点，并将该类作为该结点的类
        if len(list(Y_data.value_counts())) == 1:
            tree_node.category = Y_data.iloc[0]
            tree_node.children = None
            return
        # 如果特征集为空，则置为单结点，并将Y_data中最大的类作为该结点的类
        elif len(features) == 0:
            tree_node.category = Y_data.value_counts(ascending=False).keys()[0]
            tree_node.children = None
            return
        # 否则，计算各特征的信息增益，选择信息增益最大的特征
        else:
            ent_d = self.compute_entropy(Y_data)
            XY_data = pd.concat([X_data, Y_data], axis=1)
            d_nums = XY_data.shape[0]
            max_gain_ratio = 0
            feature = None

            for i in range(len(features)):
                v = self.features.get(features[i])
                Ga = ent_d
                for j in v:
                    dv = XY_data[XY_data[features[i]] == j]
                    dv_nums = dv.shape[0]
                    ent_dv = self.compute_entropy(dv[dv.columns[-1]])
                    Ga -= dv_nums/d_nums*ent_dv

                if Ga > max_gain_ratio:
                    max_gain_ratio = Ga
                    feature = features[i]

            # 信息增益低于阈值0
            if feature is None:
                tree_node.category = Y_data.value_counts(ascending=False).keys()[0]
                tree_node.children = None
                return
            tree_node.feature = feature

            # get all kinds of values of the current partition feature
            branches = self.features.get(feature)
            # branches = list(XY_data[feature].value_counts().keys())
            tree_node.children = dict()
            for i in range(len(branches)):
                X_data = XY_data[XY_data[feature] == branches[i]]
                if len(X_data) == 0:
                    category = XY_data[XY_data.columns[-1]].value_counts(ascending=False).keys()[0]
                    childNode = TreeNode(tree_node, None, None, category, None, None)
                    tree_node.children[branches[i]] = childNode
                    # return
                    # error, not should return, but continue
                    continue

                Y_data = X_data[X_data.columns[-1]]
                X_data.drop(X_data.columns[-1], axis=1, inplace=True)
                X_data.drop(feature, axis=1, inplace=True)
                childNode = TreeNode(tree_node, None, None, None, X_data, Y_data)
                tree_node.children[branches[i]] = childNode
                # print("feature: " + str(tree_node.feature) + " branch: " + str(branches[i]) + "\n")
                self.tree_generate(childNode)

            return

    def compute_entropy(self, Y):
        ent = 0;
        for cate in Y.value_counts(1):
            ent -= cate*np.log2(cate);
        return ent




