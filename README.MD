# py-binary-search-tree
## Description
This library provides an implementation of an AVL Binary Search Tree. It behaves similarly to a dictionary, using key-value pairs. Since a Binary Search Tree works by comparison, all keys must be comparable with each other. > and < operators are required.
## Methods
- get  
Return the value for the key if the key is in the tree, otherwise return default.
```python
binary_search_tree.get(key: Any, default: Any = None)
```
- pop  
Remove the specified key and return the corresponding value. If key is not found return the default if provided, else raise KeyError exception."
```python
binary_search_tree.pop(key: Any)
binary_search_tree.pop(key: Any, default: Any)
```
- update  
Update the tree using key-value pairs defined in a map.
```python
binary_search_tree.update(m: SupportsKeysAndGetItems)
```
- clear  
Clear all key-value pairs from the tree.
```python
binary_search_tree.clear()
```
- fromkeys  
Class Method  
Create a new tree using the keys from an iterable and setting the values to the value provided.
```python
binary_search_tree.fromkeys(iterable: Iterable, value: Any = None)
```

# License
Copyright (c) 2026 Joshua A. Morningstar

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.