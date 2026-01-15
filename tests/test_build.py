from py_binary_search_tree import BinarySearchTree
import weakref

class DummyClass():
    def __init__(self, id: int):
        self.id = id

    def __str__(self):
        return str(self.id)

def test_assigning_and_indexing():
    new_tree = BinarySearchTree()
    new_tree["test1"] = 1
    new_tree["test2"] = 2
    new_tree["test3"] = 3
    assert new_tree["test1"] == 1
    assert new_tree.pop("test2") == 2
    assert new_tree.get("test3") == 3
    assert new_tree.get("test2") == None

def test_items_keys_values():
    new_tree = BinarySearchTree()
    new_tree["0"] = 0
    new_tree["1"] = 1
    new_tree["2"] = 2
    for i, item in enumerate(new_tree.items()):
        assert (str(i), i) == item
    for i, key in enumerate(new_tree.keys()):
        assert key == str(i)
    for i, value in enumerate(new_tree.values()):
        assert value == i


def test_dereferencing():
    new_tree = BinarySearchTree()
    new_tree["test1"] = DummyClass(1)
    new_tree["test2"] = DummyClass(2)
    ref1 = weakref.ref(new_tree["test1"])
    copy_tree = new_tree.copy()
    new_tree.pop("test1")
    assert str(copy_tree["test1"]) == "1"
    del copy_tree["test1"]
    assert ref1() is None
    del copy_tree["test2"]
    ref2 = weakref.ref(new_tree["test2"])
    new_tree["test2"] = DummyClass(3)
    assert ref2() is None


if __name__ == "__main__":
    test_assigning_and_indexing()
    test_items_keys_values()
    test_dereferencing()