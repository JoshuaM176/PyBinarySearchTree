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
for key, value in new_tree.items():
    print(key, value)
print(new_tree)

new_test = BinarySearchTree()
new_test["Hello"] = 5
print(new_test)
print("test")
new_test["double"] = 5
test_items_print = new_test.items()
print(test_items_print)
print()
print(new_test.get("Helo", "banana"))