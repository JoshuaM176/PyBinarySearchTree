import cProfile
import pstats
from py_binary_search_tree import BinarySearchTree
import sys
import random

random_list = [str(random.randint(1, 10000000)) for i in range(10_000_000)]

profiler = cProfile.Profile()
profiler.enable()

def test_bst():
    tree = BinarySearchTree()
    for i in random_list:
        tree[i] = i

def test_dict():
    d = dict()
    for i in random_list:
        d[i] = i


t = test_bst()
d = test_dict()
profiler.disable()
profiler.print_stats()