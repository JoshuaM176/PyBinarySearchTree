from py_binary_search_tree import BinarySearchTree
new_tree = BinarySearchTree()
new_tree[10] = 5
new_tree[11] = 6
new_tree[9] = 6
new_tree[12] = 5
new_tree[13] = 5
new_tree[14] = 5
new_tree[15] = 5
new_tree[17] = 5
new_tree.display_tree()
del new_tree[17]
new_tree.display_tree()
del new_tree[10]
new_tree.display_tree()
for item in new_tree:
    print(item)
print(new_tree.pop(10, "default"))
new_tree.display_tree()
new_tree.clear()
new_tree.display_tree()