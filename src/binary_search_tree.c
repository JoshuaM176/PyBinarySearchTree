#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static const int LEFT = 0;
static const int RIGHT = 1;
static const int ROOT = 2;

// - - - - - BinarySearchTreeNode - - - - - //

typedef struct BSTNode{
	PyObject* value;
    struct BSTNode* left;
    struct BSTNode* right;
    int height;
	PyObject* key;
} BSTNode;

static void
BSTNode_dealloc(BSTNode* op)
{
    Py_XDECREF(op->value);
    Py_XDECREF(op->key);
    free(op);
}

static void
BSTNode_dealloc_chain(BSTNode* op) {
    Py_XDECREF(op->value);
    if(op->left != NULL) {
        BSTNode_dealloc_chain(op->left);
    }
    if(op->right != NULL) {
        BSTNode_dealloc_chain(op->right);
    }
    free(op);
}

static BSTNode* BSTNode_new() {
    BSTNode* self = malloc(sizeof(BSTNode));
    if(!self) {
        return NULL;
    }
    self->key = Py_NewRef(Py_None);
    if(!self->key){
        BSTNode_dealloc(self);
        return NULL;
    }
    self->value = Py_NewRef(Py_None);
    if(!self->value) {
        BSTNode_dealloc(self);
        return NULL;
    }
    self->left = NULL;
    self->right = NULL;
    self->height = 1;
    return self;
}

static void BSTNode_update_height(BSTNode* op) {
    int left_height;
    int right_height;
    if(!op->left) {
        left_height = 0;
    }
    else {
        left_height = op->left->height;
    }
    if(!op->right) {
        right_height = 0;
    }
    else {
        right_height = op->right->height;
    }
    op->height = MAX(left_height+1, right_height+1);
}

static void BSTNode_right_rotation(BSTNode* op, BSTNode** parent_pointer) {
    BSTNode* temp = op->left;
    BSTNode* temp2 = temp->right;
    temp->right = op;
    op->left = temp2; 
    BSTNode_update_height(op);
    BSTNode_update_height(temp);
    *parent_pointer = temp;
}

static void BSTNode_left_rotation(BSTNode* op, BSTNode** parent_pointer) {
    BSTNode* temp = op->right;
    BSTNode* temp2 = temp->left;
    temp->left = op;
    op->right= temp2;
    BSTNode_update_height(op);
    BSTNode_update_height(temp);
    *parent_pointer = temp;
}

static void BSTNode_left_right_rotation(BSTNode* op, BSTNode** parent_pointer) {
    BSTNode_left_rotation(op->left, &op->left);
    BSTNode_right_rotation(op, parent_pointer);
}
static void BSTNode_right_left_rotation(BSTNode* op, BSTNode** parent_pointer) {
    BSTNode_right_rotation(op->right, &op->right);
    BSTNode_left_rotation(op, parent_pointer);
}

static int BSTNode_get_balance(BSTNode* op) {
    int left_height;
    int right_height;
    if(!op->left) {
        left_height = 0;
    }
    else {
        left_height = op->left->height;
    }
    if(!op->right) {
        right_height = 0;
    }
    else {
        right_height = op->right->height;
    }
    return left_height - right_height;
}

static void BSTNode_update(BSTNode* op, BSTNode** parent_pointer) {
    int balance = BSTNode_get_balance(op);
    if(balance == -2) {
        if(BSTNode_get_balance(op->right) == -1) {
            BSTNode_left_rotation(op, parent_pointer);
        }
        else{
            BSTNode_right_left_rotation(op, parent_pointer);
        }
    }
    else if(balance == 2) {
        if(BSTNode_get_balance(op->left) == 1) {
            BSTNode_right_rotation(op, parent_pointer);
        }
        else {
            BSTNode_left_right_rotation(op, parent_pointer);
        }
    }
    else {BSTNode_update_height(op);}
}

// __Methods__

static PyObject* BSTNode_str(BSTNode* op){
    BSTNode* self = op;
    PyObject* rtn = PyUnicode_FromFormat("%S", self->value); if(!rtn) {return NULL;} //TODO switch to dictionary format
    return rtn;
}

// - - - - - BinarySearchTree - - - - - //

typedef struct {
	PyObject_HEAD
    BSTNode* root;
    Py_ssize_t length;
    PyObject* type;
} BinarySearchTree;

// Initialization and deallocation

static void
BinarySearchTree_dealloc(PyObject *op)
{
    BinarySearchTree* self = (BinarySearchTree* )op;
    if(self->root != NULL){
    BSTNode_dealloc_chain(self->root);}
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
BinarySearchTree_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    BinarySearchTree *self;
    self = (BinarySearchTree *) type->tp_alloc(type, 0);
    self->type = Py_NewRef(Py_None);
    if(!self->type) {
        Py_DECREF(self);
        return NULL;
    }
    if (self != NULL) {
        self->root = NULL;
        self->length = 0;
    }
    return (PyObject *) self;
}

static int
BinarySearchTree_init(PyObject* op, PyObject *args)
{
    BinarySearchTree* self = (BinarySearchTree* )op;
    if (!PyArg_ParseTuple(args, ""))
        return -1;
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
    for(int i = self->root->height; i > 0;  i--) {
        int num_nodes = pow(2,height);
        for(int j = 0; j < num_nodes; j++){
            current[j] = next[j];
        }
        for(int h = 0; h < num_nodes; h++) {
            BSTNode* temp = current[h];
            if(!temp) {
                PyObject* temp_string2 = string;
                string = PyUnicode_Concat(string, null_string);
                printf("%s\n", PyUnicode_AsUTF8(string));
                Py_DECREF(temp_string2);
                next[h*2] = NULL;
                next[h*2+1] = NULL;
            }
            else {
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

// Mapping Methods

static int BinarySearchTree_remove(PyObject* op, PyObject* key) {
    BinarySearchTree* self = (BinarySearchTree*)op;
    if(!self->root) {
        PyObject* err_format = PyUnicode_FromFormat("Key %S not found.",key); if(!err_format) { return -1; }
        const char* err_str = PyUnicode_AsUTF8(err_format); if(!err_str) { Py_DECREF(err_format); return -1;}
        PyErr_SetString(PyExc_KeyError, err_str);
        return -1; 
    }
    BSTNode* temp = self->root;
    BSTNode* stack[sizeof(Py_ssize_t)];
    int dir_stack[sizeof(Py_ssize_t)];
    dir_stack[0] = ROOT;
    int stack_index = 0;
    while(1) {
        if(!temp) {
            //TODO throw error
            return -1;
        }
        stack[stack_index] = temp;
        int rslt = PyObject_RichCompareBool(temp->key, key, Py_GT); if(rslt == -1) {return -1;}
        if(rslt == 1) {
            temp = temp->left;
            ++stack_index;
            dir_stack[stack_index] = LEFT;
            continue;
        }
        rslt = PyObject_RichCompareBool(temp->key, key, Py_LT); if(rslt == -1) {return -1;}
        if(rslt == 1) {
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
    BSTNode* delete_node = temp; // stack[stack_index]
    int deleted_index = stack_index;
    if(delete_node->left && delete_node->right) {
        stack_index++;
        temp = delete_node->right;
        stack[stack_index] = temp;
        while(1) {
            if(!temp->left) {
                break;
            }
            temp = temp->left;
            ++stack_index;
            stack[stack_index] = temp;
        }
        reassign_node = temp;
        stack[deleted_index] = reassign_node; //Replace the deleted node with the reassigned one
        reassign_node->left = delete_node->left;
        if(stack_index != deleted_index+1) {
            reassign_node->right = delete_node->right;}
    }
    else {
        if(delete_node->left) {
            reassign_node = delete_node->left;
        }
        else if(delete_node->right) {
            reassign_node = delete_node->right;
        }
    }
    BSTNode_dealloc(delete_node);
    if(reassign_direction == ROOT) {
        self->root = reassign_node;
    }
    else {
        if(reassign_direction == LEFT) {
            parent_node->left = reassign_node;
        }
        else {
            parent_node->right = reassign_node;
        }
    }
    stack_index -= 1;
    for(int i = stack_index; i >= 0; i--) {
        switch(dir_stack[i]) {
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
    return 0;
}

static int BinarySearchTree_ass_subscript(PyObject* op, PyObject* key, PyObject* value){
    //TODO check key type
    if(value == NULL) {
        return BinarySearchTree_remove(op, key);
    }
    BinarySearchTree* self = (BinarySearchTree*) op;
    if(!self->root) {
        self->root = BSTNode_new(); if(!self->root) { return -1; }
        self->root->key = Py_NewRef(key);
        self->root->value = Py_NewRef(value);
        return 0;
    }
    BSTNode* temp = self->root;
    BSTNode* stack[sizeof(Py_ssize_t)];
    int dir_stack[sizeof(Py_ssize_t)];
    dir_stack[0] = ROOT;
    int stack_index = 0; // Also tracks height of current node
    while(1) {
        if(!temp) {
            stack_index--;
            temp = stack[stack_index];
            BSTNode* new_node = BSTNode_new(); if(!new_node) { return -1; }
            new_node->key = key; new_node->value = value;
            if(dir_stack[stack_index+1] == LEFT) {
                temp->left = new_node;
            } 
            else{
                temp->right = new_node;
            }
            break; }
        stack[stack_index] = temp;
        stack_index++;
        int rslt = PyObject_RichCompareBool(temp->key, key, Py_GT); if(rslt == -1) {return -1;}
        if(rslt == 1) {
            temp = temp->left;
            dir_stack[stack_index] = LEFT;
            continue;
        }
        rslt = PyObject_RichCompareBool(temp->key, key, Py_LT); if(rslt == -1) {return -1;}
        if(rslt == 1) {
            temp = temp->right;
            dir_stack[stack_index] = RIGHT;
            continue;
        }
        temp->value = value;
        return 0;
    }

    for(int i = stack_index; i >= 0; i--) {
        switch(dir_stack[i]) {
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
    self->length++;
    return 0;
}

static PyObject* BinarySearchTree_subscript(PyObject* op, PyObject* key) {
    BinarySearchTree* self = (BinarySearchTree*)op;
    BSTNode* temp = self->root;
    while(1) {
        if(!temp) {
            PyObject* err_format = PyUnicode_FromFormat("Key %S not found.", key); if(!err_format) { return NULL;}
            const char* err_str = PyUnicode_AsUTF8(err_format); if(!err_str) { Py_DECREF(err_format); return NULL;}
            PyErr_SetString(PyExc_KeyError, err_str);
            return NULL;
        }
        int rslt = PyObject_RichCompareBool(temp->key, key, Py_GT); if(rslt == -1) {return NULL;}
        if(rslt == 1) {
            temp = temp->left;
            continue;
        }
        rslt = PyObject_RichCompareBool(temp->key, key, Py_LT); if(rslt == -1) {return NULL;}
        if(rslt == 1) {
            temp = temp->right;
            continue;
        }
        return temp->value;
    }
}

static PyMethodDef BinarySearchTree_methods[] = {
    {"display_tree", (PyCFunction)BinarySearchTree_display_tree, METH_NOARGS, "display tree"},
    {NULL, NULL, 0, NULL}
};

static PyMappingMethods BinarySearchTree_map = {
    .mp_ass_subscript = (objobjargproc)BinarySearchTree_ass_subscript,
    .mp_subscript = BinarySearchTree_subscript
};

static PySequenceMethods BinarySearchTree_sequence = {
};

static PyTypeObject BinarySearchTreeType = {
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
    .tp_as_mapping = &BinarySearchTree_map
};

static int binary_search_tree_module_exec(PyObject *m)
{
    if (PyType_Ready(&BinarySearchTreeType) < 0) {return -1;}
    Py_INCREF(&BinarySearchTreeType);
    if (PyModule_AddObject(m, "BinarySearchTree", (PyObject *) &BinarySearchTreeType) < 0) {
        Py_DECREF(&BinarySearchTreeType);
        Py_DECREF(m);
        return -1;
    }
    return 0;
}

#if PY_MINOR_VERSION >= 12

static PyModuleDef_Slot py_binary_search_tree_module_slots[] = {
    {Py_mod_exec, binary_search_tree_module_exec},
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    {0, NULL}
};

#else

static PyModuleDef_Slot py_binary_search_tree_module_slots[] = {
    {Py_mod_exec, binary_search_tree_module_exec},
    {0, NULL}
};

#endif

static struct PyModuleDef py_binary_search_tree_module = {
	PyModuleDef_HEAD_INIT,
	"py_binary_search_tree.binary_search_tree",
	"A library implementing a binary search tree for python",
	.m_slots = py_binary_search_tree_module_slots
};

PyMODINIT_FUNC PyInit_binary_search_tree(void) {
	return PyModuleDef_Init(&py_binary_search_tree_module);
}
