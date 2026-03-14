import cProfile
import pstats
from py_binary_search_tree import BinarySearchTree
import sys

profiler = cProfile.Profile()
profiler.enable()

def test_bst():
    for i in range(10_000_000):
        tree = BinarySearchTree()
        for i in range(10):
            tree[str(i)] = i

def test_dict():
    for i in range(10_000_000):
        d = dict()
        for i in range(10):
            d[i] = i


t = test_bst()
#d = test_dict()
profiler.disable()
profiler.print_stats()