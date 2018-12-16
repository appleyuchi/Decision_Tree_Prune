class TreeNode:

    def __init__(self, parent, children, feature, category, X_data, Y_data):
        self.parent = parent
        self.children = children
        self.feature = feature
        self.category = category
        self.X_data = X_data
        self.Y_data = Y_data

    def get_parent(self):
        return self.parent

    def get_children(self):
        return self.children
