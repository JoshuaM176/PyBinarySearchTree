// TODO need to check length before insertion (also need to add in updating length)
// Wrapper for richcompare to change exception on type error
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static const int LEFT = 0;
static const int RIGHT = 1;
static const int ROOT = 2;

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

typedef struct BinarySearchTreeIterator BinarySearchTreeIterator;
static PyTypeObject BinarySearchTreeIteratorType;
static PyObject* BinarySearchTreeIterator_new(PyTypeObject* op, PyObject* args, PyObject* kwds);
static int BinarySearchTreeIterator_init(PyObject* op, PyObject* args, PyObject* kwds);

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

static int BinarySearchTree_init(PyObject* op, PyObject *args)
{
    BinarySearchTree* self = (BinarySearchTree*)op;
    if(!PyArg_ParseTuple(args, "")) { return -1; }
    return 0;
}

// Methods

static PyObject* BinarySearchTree_display_tree(PyObject* op) {
    BinarySearchTree* self = (BinarySearchTree*)op;
    BSTNode* current[(int)pow(2,self->root->height)];
    BSTNode* next[(int)pow(2,self->root->height)];
    PyObject* string = PyUnicode_FromFormat("");
    if(!self->root) { return string; } 
    next[0] = self->root;
    int height = 0;
    PyObject* null_string = PyUnicode_FromFormat("NULL");
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
                PyObject* temp_string = PyUnicode_FromFormat("%S", temp->key);
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
        self->root->key = Py_NewRef(key);
        self->root->value = Py_NewRef(value);
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
        return temp->value;
    }
}

// __Methods__

static PyObject* BinarySearchTree_iter(PyObject* op)
{
    BinarySearchTreeIterator* iterator = (BinarySearchTreeIterator*)BinarySearchTreeIterator_new(&BinarySearchTreeIteratorType, NULL, NULL); if(!iterator) { return NULL; }
    PyObject* init_args = PyTuple_Pack(1, op); if(!init_args) { return NULL; }
    if(BinarySearchTreeIterator_init((PyObject*)iterator, init_args, NULL)) { return NULL; }
    return (PyObject*)iterator;
}



static PyMethodDef BinarySearchTree_methods[] =
{
    {"display_tree", (PyCFunction)BinarySearchTree_display_tree, METH_NOARGS, "display tree"},
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
    .tp_basicsize = sizeof(BinarySearchTree),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = (newfunc)BinarySearchTree_new,
    .tp_init = (initproc)BinarySearchTree_init,
    .tp_dealloc = (destructor)BinarySearchTree_dealloc,
    .tp_methods = BinarySearchTree_methods,
    .tp_as_sequence = &BinarySearchTree_sequence,
    .tp_as_mapping = &BinarySearchTree_map,
    .tp_iter = (getiterfunc)BinarySearchTree_iter
};

static int binary_search_tree_module_exec(PyObject *m)
{
    if (PyType_Ready(&BinarySearchTreeType) < 0) { return -1; }
    Py_INCREF(&BinarySearchTreeType);
    if (PyModule_AddObject(m, "BinarySearchTree", (PyObject *) &BinarySearchTreeType) < 0)
    {
        Py_DECREF(&BinarySearchTreeType);
        Py_DECREF(m);
        return -1;
    }
    return 0;
}

// - - - - - BinarySearchTreeIterator - - - - - //

typedef struct BinarySearchTreeIterator
{
    PyObject_HEAD
    PyObject* tree;
    Py_ssize_t change_id;
    BSTNode* stack[sizeof(Py_ssize_t)];
    int stack_index;
} BinarySearchTreeIterator;

// Forward declarations

static int BinarySearchTreeIterator_add(PyObject* op, BSTNode* node);

// Initialization and deallocation

static void
BinarySearchTreeIterator_dealloc(PyObject *op)
{
    BinarySearchTreeIterator* self = (BinarySearchTreeIterator* )op;
    Py_XDECREF(self->tree);
    Py_TYPE(self)->tp_free(self);
}

static PyObject* BinarySearchTreeIterator_new(PyTypeObject* type, PyObject* args, PyObject* kwds) 
{
    BinarySearchTreeIterator* self;
    self = (BinarySearchTreeIterator*)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->tree = NULL;
        self->change_id = 0;
        self->stack_index = 0;
    }
    return (PyObject*) self; 
}

static int BinarySearchTreeIterator_init(PyObject* op, PyObject* args, PyObject* kwds) 
{
    BinarySearchTreeIterator* self = (BinarySearchTreeIterator*)op;
    if(self->tree) { PyErr_SetString(PyExc_TypeError, "init method called twice"); return -1; }
    static char* kwlist[] = {"tree", NULL};
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &self->tree)) { return -1; }
    Py_INCREF(self->tree);
    self->change_id = ((BinarySearchTree*)self->tree)->change_id;
    BinarySearchTreeIterator_add(op, ((BinarySearchTree*)self->tree)->root);
    return 0;
}

// Internal methods

static int BinarySearchTreeIterator_add(PyObject* op, BSTNode* node) 
{
    BinarySearchTreeIterator* self = (BinarySearchTreeIterator*)op;
    BSTNode* temp = node;
    while(1)
    {
        if(!temp) { break; }
        self->stack[self->stack_index] = temp;
        ++self->stack_index;
        temp = temp->left;
    }
    return 0;
}

static PyObject* BinarySearchTreeIterator_remove(PyObject* op) 
{
    BinarySearchTreeIterator* self = (BinarySearchTreeIterator*)op;
    --self->stack_index;
    BSTNode* temp = self->stack[self->stack_index];
    PyObject* value = temp->key;
    self->stack[self->stack_index] = NULL;
    BinarySearchTreeIterator_add(op, temp->right);
    return value;
}

// __Methods__

static PyObject* BinarySearchTreeIterator_next(PyObject* op) 
{
    BinarySearchTreeIterator* self = (BinarySearchTreeIterator*)op;
    if(((BinarySearchTree*)self->tree)->change_id != self->change_id) { PyErr_SetString(PyExc_RuntimeError, "Tree was modified during iteration."); return NULL; }
    if(self->stack_index <= 0) { return NULL; }
    return BinarySearchTreeIterator_remove(op);
}

static PyObject* BinarySearchTreeIterator_iter(PyObject* op)
{
    return op;
}

static PyTypeObject BinarySearchTreeIteratorType = 
{
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_binary_search_tree.binary_search_tree.BinarySearchTreeIterator",
    .tp_doc = PyDoc_STR("Binary Search Tree Iterator"),
    .tp_basicsize = sizeof(BinarySearchTreeIterator),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = (newfunc)BinarySearchTreeIterator_new,
    .tp_init = (initproc)BinarySearchTree_init,
    .tp_dealloc = (destructor)BinarySearchTreeIterator_dealloc,
    .tp_iter = (getiterfunc)BinarySearchTreeIterator_iter,
    .tp_iternext = (iternextfunc)BinarySearchTreeIterator_next
};

static int binary_search_tree_iterator_module_exec(PyObject *m)
{
    if (PyType_Ready(&BinarySearchTreeIteratorType) < 0) {return -1;}
    Py_INCREF(&BinarySearchTreeIteratorType);
    if (PyModule_AddObject(m, "BinarySearchTreeIterator", (PyObject *) &BinarySearchTreeIteratorType) < 0) {
        Py_DECREF(&BinarySearchTreeIteratorType);
        Py_DECREF(m);
        return -1;
    }
    return 0;
}

#if PY_MINOR_VERSION >= 12

static PyModuleDef_Slot py_binary_search_tree_module_slots[] = 
{
    {Py_mod_exec, binary_search_tree_module_exec},
    {Py_mod_exec, binary_search_tree_iterator_module_exec},
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    {0, NULL}
};

#else

static PyModuleDef_Slot py_binary_search_tree_module_slots[] = 
{
    {Py_mod_exec, binary_search_tree_module_exec},
    {Py_mod_exec, binary_search_tree_iterator_module_exec},
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
