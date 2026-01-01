// TODO need to check length before insertion (also need to add in updating length)
// Wrapper for richcompare to change exception on type error
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static const int LEFT = 0;
static const int RIGHT = 1;
static const int ROOT = 2;

// This steals a reference to left_string and right_string, then concats them and returns the new string. Checks both strings for nulls and decrefs both.
// The purpose here is to get rid of repetitive code checking by handling the boilerplate stuff that always needs to happen inside of this function.
static PyObject* PyUnicode_concat_decref(PyObject* left_string, PyObject* right_string)
{
    if(!right_string) { Py_XDECREF(left_string); return NULL; }
    if(!left_string) { Py_DECREF(right_string); return NULL; }
    PyObject* rslt = PyUnicode_Concat(left_string, right_string); if(!rslt) { Py_DECREF(right_string); Py_DECREF(left_string); return NULL; }
    Py_DECREF(right_string);
    Py_DECREF(left_string);
    return rslt;
}

// - - - - - BinarySearchTreeNode - - - - - //

typedef struct BSTNode
{
	PyObject* value;
    struct BSTNode* left;
    struct BSTNode* right;
    int height;
	PyObject* key;
} BSTNode;

static void BSTNode_dealloc(BSTNode* op)
{
    Py_XDECREF(op->value);
    Py_XDECREF(op->key);
    free(op);
}

static void BSTNode_dealloc_chain(BSTNode* op)
{
    Py_XDECREF(op->value);
    Py_XDECREF(op->key);
    if(op->left != NULL) { BSTNode_dealloc_chain(op->left); }
    if(op->right != NULL) { BSTNode_dealloc_chain(op->right); }
    free(op);
}

static BSTNode* BSTNode_new()
{
    BSTNode* self = malloc(sizeof(BSTNode));
    if(!self) { return NULL; }
    self->key = Py_NewRef(Py_None);
    if(!self->key)
    {
        BSTNode_dealloc(self);
        return NULL;
    }
    self->value = Py_NewRef(Py_None);
    if(!self->value)
    {
        BSTNode_dealloc(self);
        return NULL;
    }
    self->left = NULL;
    self->right = NULL;
    self->height = 1;
    return self;
}

static void BSTNode_update_height(BSTNode* op)
{
    int left_height;
    int right_height;
    if(!op->left) { left_height = 0; }
    else { left_height = op->left->height; }
    if(!op->right) { right_height = 0; }
    else { right_height = op->right->height; }
    op->height = MAX(left_height+1, right_height+1);
}

static void BSTNode_right_rotation(BSTNode* op, BSTNode** parent_pointer)
{
    BSTNode* temp = op->left;
    BSTNode* temp2 = temp->right;
    temp->right = op;
    op->left = temp2; 
    BSTNode_update_height(op);
    BSTNode_update_height(temp);
    *parent_pointer = temp;
}

static void BSTNode_left_rotation(BSTNode* op, BSTNode** parent_pointer)
{
    BSTNode* temp = op->right;
    BSTNode* temp2 = temp->left;
    temp->left = op;
    op->right= temp2;
    BSTNode_update_height(op);
    BSTNode_update_height(temp);
    *parent_pointer = temp;
}

static void BSTNode_left_right_rotation(BSTNode* op, BSTNode** parent_pointer)
{
    BSTNode_left_rotation(op->left, &op->left);
    BSTNode_right_rotation(op, parent_pointer);
}

static void BSTNode_right_left_rotation(BSTNode* op, BSTNode** parent_pointer)
{
    BSTNode_right_rotation(op->right, &op->right);
    BSTNode_left_rotation(op, parent_pointer);
}

static int BSTNode_get_balance(BSTNode* op)
{
    int left_height;
    int right_height;
    if(!op->left) { left_height = 0; }
    else { left_height = op->left->height; }
    if(!op->right) { right_height = 0; }
    else { right_height = op->right->height; }
    return left_height - right_height;
}

static void BSTNode_update(BSTNode* op, BSTNode** parent_pointer)
{
    int balance = BSTNode_get_balance(op);
    if(balance == -2)
    {
        if(BSTNode_get_balance(op->right) == -1) { BSTNode_left_rotation(op, parent_pointer); }
        else{ BSTNode_right_left_rotation(op, parent_pointer); }
    }
    else if(balance == 2)
    {
        if(BSTNode_get_balance(op->left) == 1) { BSTNode_right_rotation(op, parent_pointer); }
        else { BSTNode_left_right_rotation(op, parent_pointer); }
    }
    else { BSTNode_update_height(op); }
}

// __Methods__

static PyObject* BSTNode_str(BSTNode* op)
{
    BSTNode* self = op;
    PyObject* rtn = PyUnicode_FromFormat("%S", self->value); if(!rtn) { return NULL; } //TODO switch to dictionary format
    return rtn;
}

// - - - - - BinarySearchTree - - - - - //

typedef struct
{
	PyObject_HEAD
    BSTNode* root;
    Py_ssize_t length;
    Py_ssize_t change_id; // Used by iterators to see when changes occur so they can raise an error.
} BinarySearchTree;

// Forward Declarations

typedef struct BSTIterator BSTIterator;
typedef struct BinarySearchTreeItems BinarySearchTreeItems;
static PyTypeObject BSTIteratorType;
static PyTypeObject BinarySearchTreeItemsType;
static PyTypeObject BinarySearchTreeType;
static PyObject* BSTIterator_new(PyTypeObject* op, PyObject* args);
static PyObject* BinarySearchTreeItems_new(PyTypeObject* op, PyObject* args, PyObject* kwds);
static PyObject* BinarySearchTree_remove(PyObject* op, PyObject* key);
static PyObject* BinarySearchTree_subscript(PyObject* op, PyObject* key);

// Initialization and deallocation

static void BinarySearchTree_dealloc(PyObject *op)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    if(self->root != NULL) { BSTNode_dealloc_chain(self->root); }
    Py_TYPE(self)->tp_free(self);
}

static PyObject* BinarySearchTree_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    BinarySearchTree *self;
    self = (BinarySearchTree*)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->root = NULL;
        self->length = 0;
        self->change_id = 0;
    }
    return (PyObject*)self;
}

// Methods

static PyObject* BinarySearchTree_clear_method(PyObject* op)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    if(self->root) { BSTNode_dealloc_chain(self->root); }
    self->length = 0;
    self->root = NULL;
    ++self->change_id;
    return Py_NewRef(Py_None);
}

static PyObject* BinarySearchTree_display_tree(PyObject* op)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    if(!self->root) { printf("Tree is empty\n"); return Py_NewRef(Py_None); }
    BSTNode* current[(int)pow(2,self->root->height)];
    BSTNode* next[(int)pow(2,self->root->height)];
    PyObject* string = PyUnicode_FromFormat("");
    if(!self->root) { return string; } 
    next[0] = self->root;
    int height = 0;
    PyObject* null_string = PyUnicode_FromFormat("NULL ");
    for(int i = self->root->height; i > 0;  i--)
    {
        int num_nodes = pow(2,height);
        for(int j = 0; j < num_nodes; j++)
        {
            current[j] = next[j];
        }
        for(int h = 0; h < num_nodes; h++)
        {
            BSTNode* temp = current[h];
            if(!temp)
            {
                PyObject* temp_string2 = string;
                string = PyUnicode_Concat(string, null_string);
                printf("%s\n", PyUnicode_AsUTF8(string));
                Py_DECREF(temp_string2);
                next[h*2] = NULL;
                next[h*2+1] = NULL;
            }
            else
            {
                PyObject* temp_string = PyUnicode_FromFormat("%S ", temp->key);
                PyObject* temp_string2 = string;
                string = PyUnicode_Concat(string, temp_string);
                printf("%s\n", PyUnicode_AsUTF8(string));
                Py_DECREF(temp_string); Py_DECREF(temp_string2);
                next[h*2] = temp->left;
                next[h*2+1] = temp->right;
            }
        }
        height++;
    }
    return string; 
}

static PyObject* BinarySearchTree_items(PyObject* op)
{
    PyObject* args = PyTuple_Pack(1, op);
    BinarySearchTreeItems* items = (BinarySearchTreeItems*)BinarySearchTreeItems_new(&BinarySearchTreeItemsType, args, NULL); if(!items) { Py_DECREF(args); return NULL; }
    Py_DECREF(args);
    return (PyObject*)items;
}

static PyObject* BinarySearchTree_get(PyObject* op, PyObject* args, PyObject* kwds)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    static char* kwlist[] = {"key", "default", NULL};
    PyObject* key; PyObject* default_value = Py_NewRef(Py_None);
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &key, &default_value)) { return NULL; }
    PyObject* rslt = BinarySearchTree_subscript(op, key);
    if(rslt) {
        return rslt;
    }
    if(PyErr_Occurred()) {
        if(PyErr_ExceptionMatches(PyExc_KeyError)) 
        {
            PyErr_Clear();   
            return Py_NewRef(default_value); 
        }
        return NULL;
    }
    return NULL;
}

static PyObject* BinarySearchTree_pop(PyObject* op, PyObject* args, PyObject* kwds)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    static char* kwlist[] = {"key", "default", NULL};
    PyObject* key = NULL; PyObject* default_value = NULL ;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &key, &default_value)) { return NULL; }
    PyObject* rslt = BinarySearchTree_remove(op, key);
    if(!rslt)
    {
        if(default_value && PyErr_Occurred())
        {
            if(PyErr_ExceptionMatches(PyExc_KeyError)) { PyErr_Clear(); return Py_NewRef(default_value); }
        }
        return NULL;
    }
    return rslt;
}

static PyObject* BinarySearchTree_copy(PyObject* op)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    BinarySearchTree* new = BinarySearchTree_new(&BinarySearchTreeType, NULL, NULL); if(!new) { return NULL; }
    if(self->length == 0)
    {
        return (PyObject*)new; 
    }
    new->length = self->length;
    BSTNode* og_stack[sizeof(Py_ssize_t)];
    BSTNode* new_stack[sizeof(Py_ssize_t)];
    int stack_index = 0;
    BSTNode* og_temp = self->root;
    BSTNode* new_temp = BSTNode_new(); if(!new_temp) { Py_DECREF(new); return NULL; }
    new->root = new_temp;
    new_temp->key = Py_NewRef(og_temp->key);
    new_temp->value = Py_NewRef(og_temp->value);
    new_temp->height = og_temp->height;
    while(1)
    {
        if(og_temp->right)
        {
            og_stack[stack_index] = og_temp;
            new_stack[stack_index] = new_temp;
            ++stack_index;
        }
        if(og_temp->left)
        {
            new_temp->left = BSTNode_new(); if(!new_temp->left) { Py_DECREF(new); return NULL; }
            new_temp = new_temp->left;
            og_temp = og_temp->left;
            new_temp->key = Py_NewRef(og_temp->key);
            new_temp->value = Py_NewRef(og_temp->value);
            new_temp->height = og_temp->height;
        }
        else
        {
            --stack_index;
            if(stack_index < 0)
            {
                break;
            }
            og_temp = og_stack[stack_index]->right;
            new_temp = new_stack[stack_index];
            new_temp->right = BSTNode_new(); if(!new_temp->right) { Py_DECREF(new); return NULL; }
            new_temp = new_temp->right;
            new_temp->key = Py_NewRef(og_temp->key);
            new_temp->value = Py_NewRef(og_temp->value);
            new_temp->height = og_temp->height;
        }
    }
    return (PyObject*)new;
}

// Internal Methods

static PyObject* BinarySearchTree_remove(PyObject* op, PyObject* key)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    BSTNode* temp = self->root;
    BSTNode* stack[sizeof(Py_ssize_t)];
    int dir_stack[sizeof(Py_ssize_t)];
    dir_stack[0] = ROOT;
    int stack_index = 0;
    while(1)
    {
        if(!temp)
        {
            PyObject* err_format = PyUnicode_FromFormat("Key %S not found.", key); if(!err_format) { return NULL; }
            const char* err_str = PyUnicode_AsUTF8(err_format); if(!err_str) { Py_DECREF(err_format); return NULL; }
            PyErr_SetString(PyExc_KeyError, err_str);
            return NULL;
        }
        stack[stack_index] = temp;
        int rslt = PyObject_RichCompareBool(temp->key, key, Py_GT); if(rslt == -1) { return NULL; }
        if(rslt == 1)
        {
            temp = temp->left;
            ++stack_index;
            dir_stack[stack_index] = LEFT;
            continue;
        }
        rslt = PyObject_RichCompareBool(temp->key, key, Py_LT); if(rslt == -1) { return NULL; }
        if(rslt == 1)
        {
            temp = temp->right;
            ++stack_index;
            dir_stack[stack_index] = RIGHT;
            continue;
        }
        break;
    }
    BSTNode* parent_node = stack[stack_index-1];
    BSTNode* reassign_node = NULL;
    int reassign_direction = dir_stack[stack_index];
    BSTNode* delete_node = temp;
    int deleted_index = stack_index;
    PyObject* return_value = Py_NewRef(delete_node->value);
    if(delete_node->left && delete_node->right)
    {
        ++stack_index;
        temp = delete_node->right;
        stack[stack_index] = temp;
        while(1)
        {
            if(!temp->left) { break; }
            temp = temp->left;
            ++stack_index;
            stack[stack_index] = temp;
        }
        reassign_node = temp;
        stack[deleted_index] = reassign_node;
        reassign_node->left = delete_node->left;
        if(stack_index != deleted_index+1)
        {
            stack[stack_index-1]->left = reassign_node->right;
            reassign_node->right = delete_node->right;
        }
    }
    else
    {
        if(delete_node->left) { reassign_node = delete_node->left; }
        else if(delete_node->right) { reassign_node = delete_node->right; }
    }
    BSTNode_dealloc(delete_node);
    if(reassign_direction == ROOT) { self->root = reassign_node; }
    else
    {
        if(reassign_direction == LEFT) { parent_node->left = reassign_node; }
        else { parent_node->right = reassign_node; }
    }
    stack_index -= 1;
    for(int i = stack_index; i >= 0; i--)
    {
        switch(dir_stack[i])
        {
            case LEFT:
                BSTNode_update(stack[i], &(stack[i-1]->left));
                break;
            case RIGHT:
                BSTNode_update(stack[i], &(stack[i-1]->right));
                break;
            case ROOT:
                BSTNode_update(stack[i], &(self->root));
                break;
        }
    }
    --self->length;
    ++self->change_id;
    return return_value;
}

static int BinarySearchTree_assign(PyObject* op, PyObject* key, PyObject* value)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    if(!self->root)
    {
        self->root = BSTNode_new(); if(!self->root) { return -1; }
        Py_SETREF(self->root->key, Py_NewRef(key));
        Py_SETREF(self->root->value, Py_NewRef(value));
        ++self->length;
        ++self->change_id;
        return 0;
    }
    BSTNode* temp = self->root;
    BSTNode* stack[sizeof(Py_ssize_t)];
    int dir_stack[sizeof(Py_ssize_t)];
    dir_stack[0] = ROOT;
    int stack_index = 0;
    while(1)
    {
        if(!temp)
        {
            --stack_index;
            temp = stack[stack_index];
            BSTNode* new_node = BSTNode_new(); if(!new_node) { return -1; }
            new_node->key = Py_NewRef(key); new_node->value = Py_NewRef(value);
            if(dir_stack[stack_index+1] == LEFT) { temp->left = new_node; } 
            else { temp->right = new_node; }
            break; 
        }
        stack[stack_index] = temp;
        ++stack_index;
        int rslt = PyObject_RichCompareBool(temp->key, key, Py_GT); if(rslt == -1) { return -1; }
        if(rslt == 1)
        {
            temp = temp->left;
            dir_stack[stack_index] = LEFT;
            continue;
        }
        rslt = PyObject_RichCompareBool(temp->key, key, Py_LT); if(rslt == -1) { return -1; }
        if(rslt == 1)
        {
            temp = temp->right;
            dir_stack[stack_index] = RIGHT;
            continue;
        }
        Py_SETREF(temp->value, value);
        return 0;
    }

    for(int i = stack_index; i >= 0; i--)
    {
        switch(dir_stack[i])
        {
            case LEFT:
                BSTNode_update(stack[i], &(stack[i-1]->left));
                break;
            case RIGHT:
                BSTNode_update(stack[i], &(stack[i-1]->right));
                break;
            case ROOT:
                BSTNode_update(stack[i], &(self->root));
        }
    }
    ++self->length;
    ++self->change_id;
    return 0;
}

// Mapping Methods

static int BinarySearchTree_ass_subscript(PyObject* op, PyObject* key, PyObject* value)
{
    if(value == NULL) {
        PyObject* rslt = BinarySearchTree_remove(op, key); if(!rslt) { return -1; }
        Py_DECREF(rslt);
        return 0;
    }
    return BinarySearchTree_assign(op, key, value);
}

static PyObject* BinarySearchTree_subscript(PyObject* op, PyObject* key)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    BSTNode* temp = self->root;
    while(1)
    {
        if(!temp)
        {
            PyObject* err_format = PyUnicode_FromFormat("Key %S not found.", key); if(!err_format) { return NULL;}
            const char* err_str = PyUnicode_AsUTF8(err_format); if(!err_str) { Py_DECREF(err_format); return NULL; }
            PyErr_SetString(PyExc_KeyError, err_str);
            return NULL;
        }
        int rslt = PyObject_RichCompareBool(temp->key, key, Py_GT); if(rslt == -1) { return NULL; }
        if(rslt == 1)
        {
            temp = temp->left;
            continue;
        }
        rslt = PyObject_RichCompareBool(temp->key, key, Py_LT); if(rslt == -1) { return NULL; }
        if(rslt == 1)
        {
            temp = temp->right;
            continue;
        }
        return Py_NewRef(temp->value);
    }
}

// __Methods__

static PyObject* BinarySearchTree_iter(PyObject* op);
static PyObject* BSTItemsIterator_next(PyObject* op);

static PyObject* BinarySearchTree_str(PyObject* op)
{
    PyObject* iterator = BinarySearchTree_iter(op);
    PyObject* tmp;
    PyObject* string = PyUnicode_FromString("{");
    while((tmp = BSTItemsIterator_next(iterator)))
    {
        string = PyUnicode_concat_decref(string, PyUnicode_FromFormat("%R: %R, ", PyTuple_GET_ITEM(tmp, 0), PyTuple_GET_ITEM(tmp, 1)));
        Py_DECREF(tmp);
        if(!string) { return NULL; }
    }
    if(PyErr_Occurred())
    {
        Py_XDECREF(iterator);
        Py_XDECREF(string);
        return NULL;
    }
    Py_DECREF(iterator);
    PyObject* tmp2 = PyObject_CallMethod(string, "rstrip", "s", ", "); Py_DECREF(string);
    string = PyUnicode_concat_decref(tmp2, PyUnicode_FromString("}")); if(!string) { return NULL; }
    return string;
}

static PyObject* BinarySearchTree_iter(PyObject* op)
{
    PyObject* args = PyTuple_Pack(1, op); if(!args) { return NULL; }
    BSTIterator* iterator = (BSTIterator*)BSTIterator_new(&BSTIteratorType, args); if(!iterator) { Py_DECREF(args); return NULL; }
    Py_DECREF(args);
    return (PyObject*)iterator;
}

static PyMethodDef BinarySearchTree_methods[] =
{
    {"clear", (PyCFunction)BinarySearchTree_clear_method, METH_NOARGS,
    "Clear all items from the tree."},
    {"display_tree", (PyCFunction)BinarySearchTree_display_tree, METH_NOARGS, "display tree"},
    {"items", (PyCFunction)BinarySearchTree_items, METH_NOARGS,
    "Returns a view of this tree's key-value pairs that can be iterated over."},
    {"pop", (PyCFunction)BinarySearchTree_pop, METH_VARARGS|METH_KEYWORDS,
    "Pop the given key off and return its value. If a default is provided an the key isn't found then return default, otherwise raise an exception."},
    {"get", (PyCFunction)BinarySearchTree_get, METH_VARARGS|METH_KEYWORDS,
    "Get the value of the given key, return default if key is not found."},
    {"copy", (PyCFunction)BinarySearchTree_copy, METH_NOARGS,
    "Return a shallow copy of the tree."},
    {NULL, NULL, 0, NULL}
};

static PyMappingMethods BinarySearchTree_map =
{
    .mp_ass_subscript = (objobjargproc)BinarySearchTree_ass_subscript,
    .mp_subscript = BinarySearchTree_subscript
};

static PySequenceMethods BinarySearchTree_sequence =
{
};

static PyTypeObject BinarySearchTreeType =
{
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_binary_search_tree.binary_search_tree.BinarySearchTree",
    .tp_doc = PyDoc_STR("Binary Search Tree"),
    .tp_basicsize = sizeof(BinarySearchTree) + 16,
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc)BinarySearchTree_new,
    .tp_dealloc = (destructor)BinarySearchTree_dealloc,
    .tp_methods = BinarySearchTree_methods,
    .tp_as_sequence = &BinarySearchTree_sequence,
    .tp_as_mapping = &BinarySearchTree_map,
    .tp_iter = (getiterfunc)BinarySearchTree_iter,
    .tp_str = (reprfunc)BinarySearchTree_str,
    .tp_repr = (reprfunc)BinarySearchTree_str
};

static int binary_search_tree_module_exec(PyObject* m)
{
    if (PyType_Ready(&BinarySearchTreeType) < 0) { return -1; }
    Py_INCREF(&BinarySearchTreeType);
    if (PyModule_AddObject(m, "BinarySearchTree", (PyObject*) &BinarySearchTreeType) < 0)
    {
        Py_DECREF(&BinarySearchTreeType);
        Py_DECREF(m);
        return -1;
    }
    return 0;
}

// - - - - - BinarySearchTreeItems - - - - - //

typedef struct BinarySearchTreeItems
{
    PyObject_HEAD
    PyObject* tree;
} BinarySearchTreeItems;

// Forward declarations

static PyTypeObject BSTItemsIteratorType;
static PyObject* BinarySearchTreeItems_iter(PyObject* op);

// Initialization and deallocation

static void BinarySearchTreeItems_dealloc(PyObject* op)
{
    BinarySearchTreeItems* self = (BinarySearchTreeItems*)op;
    Py_XDECREF(self->tree);
    Py_TYPE(self)->tp_free(self);
}

static PyObject* BinarySearchTreeItems_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    BinarySearchTreeItems* self;
    self = (BinarySearchTreeItems*)type->tp_alloc(type, 0);
    static char* kwlist[] = {"tree", NULL};
    if(self != NULL)
    {
        if(!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &self->tree)) { Py_DECREF(self); return NULL; }
    }
    return (PyObject*)self;
}


// __Methods__

static PyObject* BinarySearchTreeItems_str(PyObject* op)
{
    PyObject* iterator = BinarySearchTreeItems_iter(op);
    PyObject* tmp;
    PyObject* string = PyUnicode_FromString("binary_search_tree_items([");
    while((tmp = BSTItemsIterator_next(iterator)))
    {
        string = PyUnicode_concat_decref(string, PyUnicode_FromFormat("(%R, %R), ", PyTuple_GET_ITEM(tmp, 0), PyTuple_GET_ITEM(tmp, 1)));
        Py_DECREF(tmp);
        if(!string) { return NULL; }
    }
    if(PyErr_Occurred())
    {
        Py_XDECREF(iterator);
        Py_XDECREF(string);
        return NULL;
    }
    Py_DECREF(iterator);
    PyObject* tmp2 = PyObject_CallMethod(string, "rstrip", "s", ", "); Py_DECREF(string);
    string = PyUnicode_concat_decref(tmp2, PyUnicode_FromString("])")); if(!string) { return NULL; }
    return string;
}

static PyObject* BinarySearchTreeItems_iter(PyObject* op)
{
    BinarySearchTreeItems* self = (BinarySearchTreeItems*)op;
    PyObject* args = PyTuple_Pack(1, self->tree); if(!args) { return NULL; }
    BSTIterator* iterator = (BSTIterator*)BSTIterator_new(&BSTItemsIteratorType, args); if(!iterator) { Py_DECREF(args); return NULL; }
    return (PyObject*)iterator;
}

static PyTypeObject BinarySearchTreeItemsType = 
{
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_binary_search_tree.binary_search_tree.BinarySearchTreeItems",
    .tp_doc = PyDoc_STR("Binary Search Tree Items View"),
    .tp_basicsize = sizeof(BinarySearchTreeItems),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc)BinarySearchTreeItems_new,
    .tp_dealloc = (destructor)BinarySearchTreeItems_dealloc,
    .tp_iter = (getiterfunc)BinarySearchTreeItems_iter,
    .tp_str = (reprfunc)BinarySearchTreeItems_str,
    .tp_repr = (reprfunc)BinarySearchTreeItems_str
};

static int binary_search_tree_items_module_exec(PyObject *m)
{
    if (PyType_Ready(&BinarySearchTreeItemsType) < 0) {return -1;}
    Py_INCREF(&BinarySearchTreeItemsType);
    if (PyModule_AddObject(m, "BinarySearchTreeItems", (PyObject *) &BinarySearchTreeItemsType) < 0) {
        Py_DECREF(&BinarySearchTreeItemsType);
        Py_DECREF(m);
        return -1;
    }
    return 0;
}

// - - - - - BSTIterator - - - - - //

typedef struct BSTIterator
{
    PyObject_HEAD
    PyObject* tree;
    Py_ssize_t change_id;
    BSTNode* stack[sizeof(Py_ssize_t)];
    int stack_index;
} BSTIterator;

// Forward declarations

static void BSTIterator_add(PyObject* op, BSTNode* node);

// Initialization and deallocation

static void BSTIterator_dealloc(PyObject *op)
{
    BSTIterator* self = (BSTIterator* )op;
    Py_XDECREF(self->tree);
    Py_TYPE(self)->tp_free(self);
}

static PyObject* BSTIterator_new(PyTypeObject* type, PyObject* args) 
{
    BSTIterator* self;
    self = (BSTIterator*)type->tp_alloc(type, 0);
    if(self != NULL)
    {
        if(!PyArg_ParseTuple(args, "O", &self->tree)) { Py_DECREF(self); return NULL; }
        Py_INCREF(self->tree);
        self->change_id = ((BinarySearchTree*)self->tree)->change_id;
        self->stack_index = 0;
        BSTIterator_add((PyObject*)self, ((BinarySearchTree*)self->tree)->root);
    }
    return (PyObject*)self; 
}

// Internal methods

static void BSTIterator_add(PyObject* op, BSTNode* node) 
{
    BSTIterator* self = (BSTIterator*)op;
    BSTNode* temp = node;
    while(1)
    {
        if(!temp) { break; }
        self->stack[self->stack_index] = temp;
        ++self->stack_index;
        temp = temp->left;
    }
}

static PyObject* BSTIterator_remove(PyObject* op) 
{
    BSTIterator* self = (BSTIterator*)op;
    --self->stack_index;
    BSTNode* temp = self->stack[self->stack_index];
    PyObject* value = temp->key;
    self->stack[self->stack_index] = NULL;
    BSTIterator_add(op, temp->right);
    return value;
}

static PyObject* BSTItemsIterator_remove(PyObject* op) 
{
    BSTIterator* self = (BSTIterator*)op;
    --self->stack_index;
    BSTNode* temp = self->stack[self->stack_index];
    PyObject* value = PyTuple_Pack(2, temp->key, temp->value);
    self->stack[self->stack_index] = NULL;
    BSTIterator_add(op, temp->right);
    return value;
}

// __Methods__

static PyObject* BSTIterator_next(PyObject* op) 
{
    BSTIterator* self = (BSTIterator*)op;
    if(((BinarySearchTree*)self->tree)->change_id != self->change_id) { PyErr_SetString(PyExc_RuntimeError, "Tree size changed during iteration."); return NULL; }
    if(self->stack_index <= 0) { return NULL; }
    return BSTIterator_remove(op);
}

static PyObject* BSTItemsIterator_next(PyObject* op) 
{
    BSTIterator* self = (BSTIterator*)op;
    if(((BinarySearchTree*)self->tree)->change_id != self->change_id) { PyErr_SetString(PyExc_RuntimeError, "Tree size changed during iteration."); return NULL; }
    if(self->stack_index <= 0) { return NULL; }
    return BSTItemsIterator_remove(op);
}

static PyTypeObject BSTIteratorType = 
{
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_binary_search_tree.binary_search_tree.BSTIterator",
    .tp_doc = PyDoc_STR("Binary Search Tree Iterator"),
    .tp_basicsize = sizeof(BSTIterator),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor)BSTIterator_dealloc,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc)BSTIterator_next
};

static PyTypeObject BSTItemsIteratorType =
{
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_binary_search_tree.binary_search_tree.BSTItemsIterator",
    .tp_doc = PyDoc_STR("Binary Search Tree Iterator"),
    .tp_basicsize = sizeof(BSTIterator),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor)BSTIterator_dealloc,
    .tp_iter = PyObject_SelfIter, 
    .tp_iternext = (iternextfunc)BSTItemsIterator_next
};

static int bst_iterator_module_exec(PyObject *m)
{
    if (PyType_Ready(&BSTIteratorType) < 0) {return -1;}
    Py_INCREF(&BSTIteratorType);
    if (PyModule_AddObject(m, "BSTIterator", (PyObject *) &BSTIteratorType) < 0) {
        Py_DECREF(&BSTIteratorType);
        Py_DECREF(m);
        return -1;
    }
    return 0;
}

static int bst_items_iterator_module_exec(PyObject *m)
{
    if (PyType_Ready(&BSTItemsIteratorType) < 0) {return -1;}
    Py_INCREF(&BSTItemsIteratorType);
    if (PyModule_AddObject(m, "BSTItemsIterator", (PyObject *) &BSTItemsIteratorType) < 0) {
        Py_DECREF(&BSTItemsIteratorType);
        Py_DECREF(m);
        return -1;
    }
    return 0;
}

#if PY_MINOR_VERSION >= 12

static PyModuleDef_Slot py_binary_search_tree_module_slots[] = 
{
    {Py_mod_exec, binary_search_tree_module_exec},
    {Py_mod_exec, binary_search_tree_items_module_exec},
    {Py_mod_exec, bst_iterator_module_exec},
    {Py_mod_exec, bst_items_iterator_module_exec},
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    {0, NULL}
};

#else

static PyModuleDef_Slot py_binary_search_tree_module_slots[] = 
{
    {Py_mod_exec, binary_search_tree_module_exec},
    {Py_mod_exec, binary_search_tree_items_module_exec},
    {Py_mod_exec, bst_iterator_module_exec},
    {Py_mod_exec, bst_items_iterator_module_exec},
    {0, NULL}
};

#endif


static struct PyModuleDef py_binary_search_tree_module =
{
	PyModuleDef_HEAD_INIT,
	"py_binary_search_tree.binary_search_tree",
	"A library implementing a binary search tree for python",
	.m_slots = py_binary_search_tree_module_slots
};

PyMODINIT_FUNC PyInit_binary_search_tree(void)
{
	return PyModuleDef_Init(&py_binary_search_tree_module);
}
