from typing import Iterable, Any, overload

class BinarySearchTreeItems:
    def __init__(self, tree: BinarySearchTree) -> None: ...
    def __iter__(self) -> Iterable: ... #TODO

class BinarySearchTree:
    def __init__(self) -> None: ...
    def clear(self) -> None:
        """Removes all items from tree."""
        ...
    def items(self) -> Any:
        """Returns a view of this tree's key-value pairs that can be iterated over."""
    @overload
    def pop(self, key: Any):
        """Pop the given key off and return its value. Raise KeyError exception if key is not found."""
        ...
    @overload
    def pop(self, key: Any, default: Any):
        """Pop the given key off and return its value. Return the default if key is not found."""
    def __iter__(self): ... #TODO
