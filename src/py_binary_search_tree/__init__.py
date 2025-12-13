from .binary_search_tree import BinarySearchTree, BSTItems

class BinarySearchTree(BinarySearchTree):

    def __str__(self):
        string = ""
        for key, value in self.items():
            string += f"{repr(key)}: {repr(value)}, "
        string = string.removesuffix(", ")
        string = "{" + string + "}"
        return string