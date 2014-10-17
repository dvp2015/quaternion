/*
 * Quaternion type for NumPy
 * Copyright (c) 2011 Martin Ling
 *
 * This code has been expanded (and a few bugs have been corrected) by
 * Michael Boyle.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NumPy Developers nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTERS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define NPY_NO_DEPRECATED_API NPY_API_VERSION

#include <Python.h>
#include <numpy/arrayobject.h>
#include <numpy/npy_math.h>
#include <numpy/ufuncobject.h>
#include "structmember.h"
#include "numpy/npy_3kcompat.h"

#include "quaternion.h"

// The basic python object holding a quaternion
typedef struct {
  PyObject_HEAD;
  quaternion obval;
} PyQuaternion;

static PyTypeObject PyQuaternion_Type;

static NPY_INLINE int
PyQuaternion_Check(PyObject* object) {
  return PyObject_IsInstance(object,(PyObject*)&PyQuaternion_Type);
}

static PyObject*
PyQuaternion_FromQuaternion(quaternion q) {
  PyQuaternion* p = (PyQuaternion*)PyQuaternion_Type.tp_alloc(&PyQuaternion_Type,0);
  if (p) { p->obval = q; }
  return (PyObject*)p;
}

// TODO: Add list/tuple conversions
#define PyQuaternion_AsQuaternion(q, o)         \
  quaternion q = {0};                           \
  if(PyQuaternion_Check(o)) {                   \
    q = ((PyQuaternion*)o)->obval;              \
  } else {                                      \
    return NULL;                                \
  }                                             \


#define UNARY_BOOL_RETURNER(name)                                       \
  static PyObject*                                                      \
  pyquaternion_##name(PyObject* a, PyObject* b) {                       \
    PyQuaternion_AsQuaternion(q, a);                                    \
    return PyBool_FromLong(quaternion_##name(q));                       \
  }
UNARY_BOOL_RETURNER(nonzero)
UNARY_BOOL_RETURNER(isnan)
UNARY_BOOL_RETURNER(isinf)
UNARY_BOOL_RETURNER(isfinite)

#define BINARY_BOOL_RETURNER(name)                                      \
  static PyObject*                                                      \
  pyquaternion_##name(PyObject* a, PyObject* b) {                       \
    PyQuaternion_AsQuaternion(p, a);                                    \
    PyQuaternion_AsQuaternion(q, b);                                    \
    return PyBool_FromLong(quaternion_##name(p,q));                     \
  }
BINARY_BOOL_RETURNER(equal)
BINARY_BOOL_RETURNER(not_equal)
BINARY_BOOL_RETURNER(less)
BINARY_BOOL_RETURNER(greater)
BINARY_BOOL_RETURNER(less_equal)
BINARY_BOOL_RETURNER(greater_equal)

#define UNARY_FLOAT_RETURNER(name)                                      \
  static PyObject*                                                      \
  pyquaternion_##name(PyObject* a, PyObject* b) {                       \
    PyQuaternion_AsQuaternion(q, a);                                    \
    return PyFloat_FromDouble(quaternion_##name(q));                    \
  }
UNARY_FLOAT_RETURNER(absolute)

#define UNARY_QUATERNION_RETURNER(name)                                 \
  static PyObject*                                                      \
  pyquaternion_##name(PyObject* a, PyObject* b) {                       \
    PyQuaternion_AsQuaternion(q, a);                                    \
    return PyQuaternion_FromQuaternion(quaternion_##name(q));           \
  }
UNARY_QUATERNION_RETURNER(negative)
UNARY_QUATERNION_RETURNER(conjugate)
UNARY_QUATERNION_RETURNER(log)
UNARY_QUATERNION_RETURNER(exp)
static PyObject*
pyquaternion_positive(PyObject* self, PyObject* b) {
    Py_INCREF(self);
    return self;
}

#define QQ_BINARY_QUATERNION_RETURNER(name)                             \
  static PyObject*                                                      \
  pyquaternion_##name(PyObject* a, PyObject* b) {                       \
    PyQuaternion_AsQuaternion(p, a);                                    \
    PyQuaternion_AsQuaternion(q, b);                                    \
    return PyQuaternion_FromQuaternion(quaternion_##name(p,q));         \
  }
QQ_BINARY_QUATERNION_RETURNER(add)
QQ_BINARY_QUATERNION_RETURNER(subtract)
QQ_BINARY_QUATERNION_RETURNER(copysign)

#define QQ_QS_SQ_BINARY_QUATERNION_RETURNER(name)                       \
  static PyObject*                                                      \
  pyquaternion_##name(PyObject* a, PyObject* b) {                       \
    if(PyFloat_Check(a)) { return pyquaternion_##name(b,a); }           \
    PyQuaternion_AsQuaternion(p, a);                                    \
    if(PyQuaternion_Check(b)) {                                         \
      quaternion q = ((PyQuaternion*)b)->obval;                         \
      return PyQuaternion_FromQuaternion(quaternion_##name(p,q));       \
    } else if(PyFloat_Check(b)) {                                       \
      double q = PyFloat_AsDouble(b);                                   \
      return PyQuaternion_FromQuaternion(quaternion_##name##_scalar(p,q)); \
    }                                                                   \
    return NULL;                                                        \
  }
QQ_QS_SQ_BINARY_QUATERNION_RETURNER(multiply)
QQ_QS_SQ_BINARY_QUATERNION_RETURNER(divide)
QQ_QS_SQ_BINARY_QUATERNION_RETURNER(power)
// QQ_QS_SQ_BINARY_QUATERNION_RETURNER(copysign)


// This is an array of methods (member functions) that will be
// available to use on the quaternion objects in python.  This is
// packaged up here, and will be used in the `tp_methods` field when
// definining the PyQuaternion_Type below.
PyMethodDef pyquaternion_methods[] = {
  // Unary bool returners
  {"nonzero", pyquaternion_nonzero, METH_NOARGS,
   "True if the quaternion has all zero components"},
  {"isnan", pyquaternion_isnan, METH_NOARGS,
   "True if the quaternion has any NAN components"},
  {"isinf", pyquaternion_isinf, METH_NOARGS,
   "True if the quaternion has any INF components"},
  {"isfinite", pyquaternion_isfinite, METH_NOARGS,
   "True if the quaternion has all finite components"},

  // Binary bool returners
  {"equal", pyquaternion_equal, METH_O,
   "True if the quaternions are PRECISELY equal"},
  {"not_equal", pyquaternion_not_equal, METH_O,
   "True if the quaternions are not PRECISELY equal"},
  {"less", pyquaternion_less, METH_O,
   "Strict dictionary ordering"},
  {"greater", pyquaternion_greater, METH_O,
   "Strict dictionary ordering"},
  {"less_equal", pyquaternion_less_equal, METH_O,
   "Dictionary ordering"},
  {"greater_equal", pyquaternion_greater_equal, METH_O,
   "Dictionary ordering"},

  // Unary float returners
  {"absolute", pyquaternion_absolute, METH_NOARGS,
   "Absolute value of quaternion"},
  {"abs", pyquaternion_absolute, METH_NOARGS,
   "Absolute value of quaternion"},

  // Unary quaternion returners
  // {"negative", pyquaternion_negative, METH_NOARGS,
  //  "Return the negated quaternion"},
  // {"positive", pyquaternion_positive, METH_NOARGS,
  //  "Return the quaternion itself"},
  {"conjugate", pyquaternion_conjugate, METH_NOARGS,
   "Return the complex conjugate of the quaternion"},
  {"conj", pyquaternion_conjugate, METH_NOARGS,
   "Return the complex conjugate of the quaternion"},
  {"log", pyquaternion_log, METH_NOARGS,
   "Return the logarithm (base e) of the quaternion"},
  {"exp", pyquaternion_exp, METH_NOARGS,
   "Return the exponential of the quaternion (e**q)"},

  // Quaternion-quaternion binary quaternion returners
  // {"add", pyquaternion_add, METH_O,
  //  "Componentwise addition"},
  // {"subtract", pyquaternion_subtract, METH_O,
  //  "Componentwise subtraction"},
  {"copysign", pyquaternion_copysign, METH_O,
   "Componentwise copysign"},

  // Quaternion-quaternion or quaternion-scalar binary quaternion returners
  // {"multiply", pyquaternion_multiply, METH_O,
  //  "Standard (geometric) quaternion product"},
  // {"divide", pyquaternion_divide, METH_O,
  //  "Standard (geometric) quaternion division"},
  // {"power", pyquaternion_power, METH_O,
  //  "q.power(p) = (q.log() * p).exp()"},

  {NULL}
};

static PyObject* pyquaternion_num_power(PyObject* a, PyObject* b, PyObject *c) { return pyquaternion_power(a,b); }
static PyObject* pyquaternion_num_negative(PyObject* a) { return pyquaternion_negative(a,NULL); }
static PyObject* pyquaternion_num_positive(PyObject* a) { return pyquaternion_positive(a,NULL); }
static PyObject* pyquaternion_num_absolute(PyObject* a) { return pyquaternion_absolute(a,NULL); }
static int pyquaternion_num_nonzero(PyObject* a) {
  quaternion q = ((PyQuaternion*)a)->obval;
  return quaternion_nonzero(q);
}

static PyNumberMethods pyquaternion_as_number = {
  pyquaternion_add,              // nb_add
  pyquaternion_subtract,         // nb_subtract
  pyquaternion_multiply,         // nb_multiply
  pyquaternion_divide,           // nb_divide
  0,                             // nb_remainder
  0,                             // nb_divmod
  pyquaternion_num_power,        // nb_power
  pyquaternion_num_negative,     // nb_negative
  pyquaternion_num_positive,     // nb_positive
  pyquaternion_num_absolute,     // nb_absolute
  pyquaternion_num_nonzero,      // nb_nonzero
  0,                             // nb_invert
  0,                             // nb_lshift
  0,                             // nb_rshift
  0,                             // nb_and
  0,                             // nb_xor
  0,                             // nb_or
  0,                             // nb_coerce
  0,                             // nb_int
  0,                             // nb_long
  0,                             // nb_float
  0,                             // nb_oct
  0,                             // nb_hex
  0,                             // nb_inplace_add
  0,                             // nb_inplace_subtract
  0,                             // nb_inplace_multiply
  0,                             // nb_inplace_divide
  0,                             // nb_inplace_remainder
  0,                             // nb_inplace_power
  0,                             // nb_inplace_lshift
  0,                             // nb_inplace_rshift
  0,                             // nb_inplace_and
  0,                             // nb_inplace_xor
  0,                             // nb_inplace_or
  pyquaternion_divide,           // nb_floor_divide
  pyquaternion_divide,           // nb_true_divide
  0,                             // nb_inplace_floor_divide
  0,                             // nb_inplace_true_divide
  0,                             // nb_index
};


// This is an array of members (member data) that will be available to
// use on the quaternion objects in python.  This is packaged up here,
// and will be used in the `tp_members` field when definining the
// PyQuaternion_Type below.
PyMemberDef pyquaternion_members[] = {
  {"real", T_DOUBLE, offsetof(PyQuaternion, obval.w), READONLY,
   "The real component of the quaternion"},
  {"w", T_DOUBLE, offsetof(PyQuaternion, obval.w), READONLY,
   "The real component of the quaternion"},
  {"x", T_DOUBLE, offsetof(PyQuaternion, obval.x), READONLY,
   "The first imaginary component of the quaternion"},
  {"y", T_DOUBLE, offsetof(PyQuaternion, obval.y), READONLY,
   "The second imaginary component of the quaternion"},
  {"z", T_DOUBLE, offsetof(PyQuaternion, obval.z), READONLY,
   "The third imaginary component of the quaternion"},
  {NULL}
};

// This will be defined as a member function on the quaternion
// objects, so that calling "components" will return a tuple with the
// components of the quaternion.
static PyObject *
pyquaternion_get_components(PyObject *self, void *closure)
{
  quaternion *q = &((PyQuaternion *)self)->obval;
  PyObject *tuple = PyTuple_New(4);
  PyTuple_SET_ITEM(tuple, 0, PyFloat_FromDouble(q->w));
  PyTuple_SET_ITEM(tuple, 1, PyFloat_FromDouble(q->x));
  PyTuple_SET_ITEM(tuple, 2, PyFloat_FromDouble(q->y));
  PyTuple_SET_ITEM(tuple, 3, PyFloat_FromDouble(q->z));
  return tuple;
}

// This will be defined as a member function on the quaternion
// objects, so that calling "components" will return a tuple with the
// last three components (the vector components) of the quaternion.
static PyObject *
pyquaternion_get_imag(PyObject *self, void *closure)
{
  quaternion *q = &((PyQuaternion *)self)->obval;
  PyObject *tuple = PyTuple_New(3);
  PyTuple_SET_ITEM(tuple, 0, PyFloat_FromDouble(q->x));
  PyTuple_SET_ITEM(tuple, 1, PyFloat_FromDouble(q->y));
  PyTuple_SET_ITEM(tuple, 2, PyFloat_FromDouble(q->z));
  return tuple;
}

// This collects the methods for getting and setting elements of the
// quaternion.  This is packaged up here, and will be used in the
// `tp_getset` field when definining the PyQuaternion_Type
// below.
PyGetSetDef pyquaternion_getset[] = {
  {"components", pyquaternion_get_components, NULL,
   "The components of the quaternion as a (w,x,y,z) tuple", NULL},
  {"imag", pyquaternion_get_imag, NULL,
   "The imaginary part of the quaternion as an (x,y,z) tuple", NULL},
  {NULL}
};

// This establishes the quaternion as a python object (not yet a numpy
// scalar type).  The name may be a little counterintuitive; the idea
// is that this will be a type that can be used as an array dtype.
// Note that many of the slots below will be filled later, after the
// corresponding functions are defined.
static PyTypeObject PyQuaternion_Type = {
  #if defined(NPY_PY3K)
  PyVarObject_HEAD_INIT(NULL, 0)
  #else
  PyObject_HEAD_INIT(NULL)
  0,                                          // ob_size
  #endif
  "quaternion",                               // tp_name
  sizeof(PyQuaternion),                       // tp_basicsize
  0,                                          // tp_itemsize
  0,                                          // tp_dealloc
  0,                                          // tp_print
  0,                                          // tp_getattr
  0,                                          // tp_setattr
  #if defined(NPY_PY3K)
  0,                                          // tp_reserved
  #else
  0,                                          // tp_compare
  #endif
  0,                                          // tp_repr
  &pyquaternion_as_number,                    // tp_as_number
  0,                                          // tp_as_sequence
  0,                                          // tp_as_mapping
  0,                                          // tp_hash
  0,                                          // tp_call
  0,                                          // tp_str
  0,                                          // tp_getattro
  0,                                          // tp_setattro
  0,                                          // tp_as_buffer
  0,                                          // tp_flags
  0,                                          // tp_doc
  0,                                          // tp_traverse
  0,                                          // tp_clear
  0,                                          // tp_richcompare
  0,                                          // tp_weaklistoffset
  0,                                          // tp_iter
  0,                                          // tp_iternext
  pyquaternion_methods,                       // tp_methods
  pyquaternion_members,                       // tp_members
  pyquaternion_getset,                        // tp_getset
  0,                                          // tp_base
  0,                                          // tp_dict
  0,                                          // tp_descr_get
  0,                                          // tp_descr_set
  0,                                          // tp_dictoffset
  0,                                          // tp_init
  0,                                          // tp_alloc
  0,                                          // tp_new
  0,                                          // tp_free
  0,                                          // tp_is_gc
  0,                                          // tp_bases
  0,                                          // tp_mro
  0,                                          // tp_cache
  0,                                          // tp_subclasses
  0,                                          // tp_weaklist
  0,                                          // tp_del
  #if PY_VERSION_HEX >= 0x02060000
  0,                                          // tp_version_tag
  #endif
};

// Functions implementing internal features. Not all of these function
// pointers must be defined for a given type. The required members are
// nonzero, copyswap, copyswapn, setitem, getitem, and cast.
static PyArray_ArrFuncs _PyQuaternion_ArrFuncs;

// This function doesn't seem to do anything...
static PyObject *
QUATERNION_getitem(char *ip, PyArrayObject *ap)
{
  return NULL;

  quaternion q;
  PyObject *tuple;

  if ((ap == NULL) || PyArray_ISBEHAVED_RO(ap)) {
    q = *((quaternion *)ip);
  }
  else {
    PyArray_Descr *descr;
    descr = PyArray_DescrFromType(NPY_DOUBLE);
    descr->f->copyswap(&q.w, ip, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(&q.x, ip+8, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(&q.y, ip+16, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(&q.z, ip+24, !PyArray_ISNOTSWAPPED(ap), NULL);
    Py_DECREF(descr);
  }

  tuple = PyTuple_New(4);
  PyTuple_SET_ITEM(tuple, 0, PyFloat_FromDouble(q.w));
  PyTuple_SET_ITEM(tuple, 1, PyFloat_FromDouble(q.x));
  PyTuple_SET_ITEM(tuple, 2, PyFloat_FromDouble(q.y));
  PyTuple_SET_ITEM(tuple, 3, PyFloat_FromDouble(q.z));

  return tuple;
}

static int QUATERNION_setitem(PyObject *op, char *ov, PyArrayObject *ap)
{
  quaternion q;

  // if (PyArray_IsScalar(op, Quaternion)) {
  if (PyQuaternion_Check(op)) {
    q = ((PyQuaternion *)op)->obval;
  }
  else {
    q.w = PyFloat_AsDouble(PyTuple_GetItem(op, 0));
    q.x = PyFloat_AsDouble(PyTuple_GetItem(op, 1));
    q.y = PyFloat_AsDouble(PyTuple_GetItem(op, 2));
    q.z = PyFloat_AsDouble(PyTuple_GetItem(op, 3));
  }
  if (PyErr_Occurred()) {
    if (PySequence_Check(op)) {
      PyErr_Clear();
      PyErr_SetString(PyExc_ValueError,
                      "setting an array element with a sequence.");
    }
    return -1;
  }
  if (ap == NULL || PyArray_ISBEHAVED(ap))
    *((quaternion *)ov)=q;
  else {
    PyArray_Descr *descr;
    descr = PyArray_DescrFromType(NPY_DOUBLE);
    descr->f->copyswap(ov, &q.w, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(ov+8, &q.x, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(ov+16, &q.y, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(ov+24, &q.z, !PyArray_ISNOTSWAPPED(ap), NULL);
    Py_DECREF(descr);
  }

  return 0;
}

static void
QUATERNION_copyswap(quaternion *dst, quaternion *src,
                    int swap, void *NPY_UNUSED(arr))
{
  PyArray_Descr *descr;
  descr = PyArray_DescrFromType(NPY_DOUBLE);
  descr->f->copyswapn(dst, sizeof(double), src, sizeof(double), 4, swap, NULL);
  Py_DECREF(descr);
}

static void
QUATERNION_copyswapn(quaternion *dst, npy_intp dstride,
                     quaternion *src, npy_intp sstride,
                     npy_intp n, int swap, void *NPY_UNUSED(arr))
{
  PyArray_Descr *descr;
  descr = PyArray_DescrFromType(NPY_DOUBLE);
  descr->f->copyswapn(&dst->w, dstride, &src->w, sstride, n, swap, NULL);
  descr->f->copyswapn(&dst->x, dstride, &src->x, sstride, n, swap, NULL);
  descr->f->copyswapn(&dst->y, dstride, &src->y, sstride, n, swap, NULL);
  descr->f->copyswapn(&dst->z, dstride, &src->z, sstride, n, swap, NULL);
  Py_DECREF(descr);
}

static int
QUATERNION_compare (quaternion *pa, quaternion *pb, PyArrayObject *NPY_UNUSED(ap))
{
  quaternion a = *pa, b = *pb;
  npy_bool anan, bnan;
  int ret;

  anan = quaternion_isnan(a);
  bnan = quaternion_isnan(b);

  if (anan) {
    ret = bnan ? 0 : -1;
  } else if (bnan) {
    ret = 1;
  } else if(quaternion_less(a, b)) {
    ret = -1;
  } else if(quaternion_less(b, a)) {
    ret = 1;
  } else {
    ret = 0;
  }

  return ret;
}

static int
QUATERNION_argmax(quaternion *ip, npy_intp n, npy_intp *max_ind, PyArrayObject *NPY_UNUSED(aip))
{
  npy_intp i;
  quaternion mp = *ip;

  *max_ind = 0;

  if (quaternion_isnan(mp)) {
    // nan encountered; it's maximal
    return 0;
  }

  for (i = 1; i < n; i++) {
    ip++;
    //Propagate nans, similarly as max() and min()
    if (!(quaternion_less_equal(*ip, mp))) {  // negated, for correct nan handling
      mp = *ip;
      *max_ind = i;
      if (quaternion_isnan(mp)) {
        // nan encountered, it's maximal
        break;
      }
    }
  }
  return 0;
}

static npy_bool
QUATERNION_nonzero (char *ip, PyArrayObject *ap)
{
  quaternion q;
  if (ap == NULL || PyArray_ISBEHAVED_RO(ap)) {
    q = *(quaternion *)ip;
  }
  else {
    PyArray_Descr *descr;
    descr = PyArray_DescrFromType(NPY_DOUBLE);
    descr->f->copyswap(&q.w, ip, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(&q.x, ip+8, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(&q.y, ip+16, !PyArray_ISNOTSWAPPED(ap), NULL);
    descr->f->copyswap(&q.z, ip+24, !PyArray_ISNOTSWAPPED(ap), NULL);
    Py_DECREF(descr);
  }
  return (npy_bool) !quaternion_equal(q, (quaternion) {0,0,0,0});
}

static void
QUATERNION_fillwithscalar(quaternion *buffer, npy_intp length, quaternion *value, void *NPY_UNUSED(ignored))
{
  npy_intp i;
  quaternion val = *value;

  for (i = 0; i < length; ++i) {
    buffer[i] = val;
  }
}

// This is a macro (followed by applications of the macro) that cast
// the input types to standard quaternions with only a nonzero scalar
// part.
#define MAKE_T_TO_QUATERNION(TYPE, type)                                \
  static void                                                           \
  TYPE ## _to_quaternion(type *ip, quaternion *op, npy_intp n,          \
                         PyArrayObject *NPY_UNUSED(aip), PyArrayObject *NPY_UNUSED(aop)) \
  {                                                                     \
    while (n--) {                                                       \
      op->w = (double)(*ip++);                                          \
      op->x = 0;                                                        \
      op->y = 0;                                                        \
      op->z = 0;                                                        \
    }                                                                   \
  }
MAKE_T_TO_QUATERNION(FLOAT, npy_float);
MAKE_T_TO_QUATERNION(DOUBLE, npy_double);
MAKE_T_TO_QUATERNION(LONGDOUBLE, npy_longdouble);
MAKE_T_TO_QUATERNION(BOOL, npy_bool);
MAKE_T_TO_QUATERNION(BYTE, npy_byte);
MAKE_T_TO_QUATERNION(UBYTE, npy_ubyte);
MAKE_T_TO_QUATERNION(SHORT, npy_short);
MAKE_T_TO_QUATERNION(USHORT, npy_ushort);
MAKE_T_TO_QUATERNION(INT, npy_int);
MAKE_T_TO_QUATERNION(UINT, npy_uint);
MAKE_T_TO_QUATERNION(LONG, npy_long);
MAKE_T_TO_QUATERNION(ULONG, npy_ulong);
MAKE_T_TO_QUATERNION(LONGLONG, npy_longlong);
MAKE_T_TO_QUATERNION(ULONGLONG, npy_ulonglong);

// This is a macro (followed by applications of the macro) that cast
// the input complex types to standard quaternions with only the first
// two components nonzero.  This doesn't make a whole lot of sense to
// me, and may be removed in the future.
#define MAKE_CT_TO_QUATERNION(TYPE, type)                               \
  static void                                                           \
  TYPE ## _to_quaternion(type *ip, quaternion *op, npy_intp n,          \
                         PyArrayObject *NPY_UNUSED(aip), PyArrayObject *NPY_UNUSED(aop)) \
  {                                                                     \
    while (n--) {                                                       \
      op->w = (double)(*ip++);                                          \
      op->x = (double)(*ip++);                                          \
      op->y = 0;                                                        \
      op->z = 0;                                                        \
    }                                                                   \
  }
MAKE_CT_TO_QUATERNION(CFLOAT, npy_float);
MAKE_CT_TO_QUATERNION(CDOUBLE, npy_double);
MAKE_CT_TO_QUATERNION(CLONGDOUBLE, npy_longdouble);

static void register_cast_function(int sourceType, int destType, PyArray_VectorUnaryFunc *castfunc)
{
  PyArray_Descr *descr = PyArray_DescrFromType(sourceType);
  PyArray_RegisterCastFunc(descr, destType, castfunc);
  PyArray_RegisterCanCast(descr, destType, NPY_NOSCALAR);
  Py_DECREF(descr);
}



// This is the crucial feature that will make a quaternion into a
// built-in numpy data type.  We will describe its features below.
PyArray_Descr* quaternion_descr;

static PyObject *
quaternion_arrtype_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  quaternion q;

  if (!PyArg_ParseTuple(args, "dddd", &q.w, &q.x, &q.y, &q.z))
    return NULL;

  return PyArray_Scalar(&q, quaternion_descr, NULL);
}

static PyObject *
gentype_richcompare(PyObject *self, PyObject *other, int cmp_op)
{
  PyObject *arr, *ret;

  arr = PyArray_FromScalar(self, NULL);
  if (arr == NULL) {
    return NULL;
  }
  ret = Py_TYPE(arr)->tp_richcompare(arr, other, cmp_op);
  Py_DECREF(arr);
  return ret;
}

static long
quaternion_arrtype_hash(PyObject *o)
{
  quaternion q = ((PyQuaternion *)o)->obval;
  long value = 0x456789;
  value = (10000004 * value) ^ _Py_HashDouble(q.w);
  value = (10000004 * value) ^ _Py_HashDouble(q.x);
  value = (10000004 * value) ^ _Py_HashDouble(q.y);
  value = (10000004 * value) ^ _Py_HashDouble(q.z);
  if (value == -1)
    value = -2;
  return value;
}

static PyObject *
quaternion_arrtype_repr(PyObject *o)
{
  char str[128];
  quaternion q = ((PyQuaternion *)o)->obval;
  sprintf(str, "quaternion(%.15g, %.15g, %.15g, %.15g)", q.w, q.x, q.y, q.z);
  return PyUString_FromString(str);
}

static PyObject *
quaternion_arrtype_str(PyObject *o)
{
  char str[128];
  quaternion q = ((PyQuaternion *)o)->obval;
  sprintf(str, "quaternion(%.15g, %.15g, %.15g, %.15g)", q.w, q.x, q.y, q.z);
  return PyUString_FromString(str);
}

// This function generates a view of the quaternion array as a float
// array.  I'm just not sure where to put it...
// {"float_array", pyquaternion_get_float_array, NULL,
//  "The quaternion array, viewed as a float array (with an extra dimension of size 4)", NULL},
// static PyObject *
// pyquaternion_get_float_array(PyObject *self, void *closure)
// {
//   PyArrayObject* array = (PyArrayObject *) self;

//   // Save the dtype of double, because it will be stolen
//   PyArray_Descr* dtype;
//   dtype = PyArray_DescrFromType(NPY_DOUBLE);

//   // Now, make an array of describing the new shape of the output array
//   size_t size = PyArray_NDIM(array);
//   npy_intp* shape_old;
//   shape_old = PyArray_SHAPE(array);
//   PyObject* shape_new = PyList_New(size+1);
//   for (size_t i = 0; i != size; ++i) {
//     PyList_SET_ITEM(shape_new, i, PyInt_FromLong(shape_old[i]));
//   }
//   PyList_SET_ITEM(shape_new, size, PyInt_FromLong(4));

//   // Get the new view
//   PyArrayObject* array_float = (PyArrayObject*) PyArray_View((PyArrayObject *) self, dtype, self->ob_type);

//   // Reshape it, so that the last dimension is split up properly
//   return PyArray_Reshape(array_float, shape_new);
// }


// This is a macro that will be used to define the various basic unary
// quaternion functions, so that they can be applied quickly to a
// numpy array of quaternions.
#define UNARY_UFUNC(name, ret_type)                             \
  static void                                                   \
  quaternion_##name##_ufunc(char** args, npy_intp* dimensions,  \
                            npy_intp* steps, void* data) {      \
    char *ip1 = args[0], *op1 = args[1];                        \
    npy_intp is1 = steps[0], os1 = steps[1];                    \
    npy_intp n = dimensions[0];                                 \
    npy_intp i;                                                 \
    for(i = 0; i < n; i++, ip1 += is1, op1 += os1){             \
      const quaternion in1 = *(quaternion *)ip1;                \
      *((ret_type *)op1) = quaternion_##name(in1);};}
// And these all do the work mentioned above, using the macro
UNARY_UFUNC(isnan, npy_bool)
UNARY_UFUNC(isinf, npy_bool)
UNARY_UFUNC(isfinite, npy_bool)
UNARY_UFUNC(absolute, npy_double)
UNARY_UFUNC(log, quaternion)
UNARY_UFUNC(exp, quaternion)
UNARY_UFUNC(negative, quaternion)
UNARY_UFUNC(conjugate, quaternion)

// This is a macro that will be used to define the various basic binary
// quaternion functions, so that they can be applied quickly to a
// numpy array of quaternions.
#define BINARY_GEN_UFUNC(name, func_name, arg_type, ret_type)           \
  static void                                                           \
  quaternion_##func_name##_ufunc(char** args, npy_intp* dimensions,     \
                                 npy_intp* steps, void* data) {         \
    char *ip1 = args[0], *ip2 = args[1], *op1 = args[2];                \
    npy_intp is1 = steps[0], is2 = steps[1], os1 = steps[2];            \
    npy_intp n = dimensions[0];                                         \
    npy_intp i;                                                         \
    for(i = 0; i < n; i++, ip1 += is1, ip2 += is2, op1 += os1){         \
      const quaternion in1 = *(quaternion *)ip1;                        \
      const arg_type in2 = *(arg_type *)ip2;                            \
      *((ret_type *)op1) = quaternion_##func_name(in1, in2);};};
// A couple special-case versions of the above
#define BINARY_UFUNC(name, ret_type)                    \
  BINARY_GEN_UFUNC(name, name, quaternion, ret_type)
#define BINARY_SCALAR_UFUNC(name, ret_type)                     \
  BINARY_GEN_UFUNC(name, name##_scalar, npy_double, ret_type)
// And these all do the work mentioned above, using the macros
BINARY_UFUNC(add, quaternion)
BINARY_UFUNC(subtract, quaternion)
BINARY_UFUNC(multiply, quaternion)
BINARY_UFUNC(divide, quaternion)
BINARY_UFUNC(power, quaternion)
BINARY_UFUNC(copysign, quaternion)
BINARY_UFUNC(equal, npy_bool)
BINARY_UFUNC(not_equal, npy_bool)
BINARY_UFUNC(less, npy_bool)
BINARY_UFUNC(less_equal, npy_bool)
BINARY_SCALAR_UFUNC(multiply, quaternion)
BINARY_SCALAR_UFUNC(divide, quaternion)
BINARY_SCALAR_UFUNC(power, quaternion)


// This will just be an empty place-holder to start things out
static PyMethodDef QuaternionMethodsPlaceHolder[] = {
  {NULL, NULL, 0, NULL}
};

#if defined(NPY_PY3K)
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "numpy_quaternion",
    NULL,
    -1,
    QuaternionMethodsPlaceHolder,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif

// This is the initialization function that does the setup
#if defined(NPY_PY3K)
PyMODINIT_FUNC PyInit_numpy_quaternion(void) {
#else
PyMODINIT_FUNC initnumpy_quaternion(void) {
#endif

  int quaternionNum;
  int arg_types[3];

  // Initialize a (for now, empty) module
  PyObject *m;
#if defined(NPY_PY3K)
  m = PyModule_Create(&moduledef);
#else
  m = Py_InitModule("numpy_quaternion", QuaternionMethodsPlaceHolder);
#endif
  if (!m) {
#if defined(NPY_PY3K)
    return NULL;
#else
    return;
#endif
  }

  // Initialize numpy
  import_array();
  if (PyErr_Occurred()) {
#if defined(NPY_PY3K)
    return NULL;
#else
    return;
#endif
  }
  import_umath();
  if (PyErr_Occurred()) {
#if defined(NPY_PY3K)
    return NULL;
#else
    return;
#endif
  }
  PyObject* numpy_str = PyString_FromString("numpy");
  if (!numpy_str) {
#if defined(NPY_PY3K)
    return NULL;
#else
    return;
#endif
  }
  PyObject* numpy = PyImport_Import(numpy_str);
  Py_DECREF(numpy_str);
  if (!numpy) {
#if defined(NPY_PY3K)
    return NULL;
#else
    return;
#endif
  }
  PyObject* numpy_dict = PyModule_GetDict(numpy);
  if (!numpy_dict) {
#if defined(NPY_PY3K)
    return NULL;
#else
    return;
#endif
  }

  // Can't set this until we import numpy
  PyQuaternion_Type.tp_base = &PyGenericArrType_Type;

  // Register the quaternion array scalar type
  #if defined(NPY_PY3K)
  PyQuaternion_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
  #else
  PyQuaternion_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES;
  #endif
  PyQuaternion_Type.tp_new = quaternion_arrtype_new;
  PyQuaternion_Type.tp_richcompare = gentype_richcompare;
  PyQuaternion_Type.tp_hash = quaternion_arrtype_hash;
  PyQuaternion_Type.tp_repr = quaternion_arrtype_repr;
  PyQuaternion_Type.tp_str = quaternion_arrtype_str;
  PyQuaternion_Type.tp_base = &PyGenericArrType_Type;
  if (PyType_Ready(&PyQuaternion_Type) < 0) {
    PyErr_Print();
    PyErr_SetString(PyExc_SystemError, "could not initialize PyQuaternion_Type");
#if defined(NPY_PY3K)
    return NULL;
#else
    return;
#endif
  }

  // The array functions
  PyArray_InitArrFuncs(&_PyQuaternion_ArrFuncs);
  _PyQuaternion_ArrFuncs.getitem = (PyArray_GetItemFunc*)QUATERNION_getitem;
  _PyQuaternion_ArrFuncs.setitem = (PyArray_SetItemFunc*)QUATERNION_setitem;
  _PyQuaternion_ArrFuncs.copyswap = (PyArray_CopySwapFunc*)QUATERNION_copyswap;
  _PyQuaternion_ArrFuncs.copyswapn = (PyArray_CopySwapNFunc*)QUATERNION_copyswapn;
  _PyQuaternion_ArrFuncs.compare = (PyArray_CompareFunc*)QUATERNION_compare;
  _PyQuaternion_ArrFuncs.argmax = (PyArray_ArgFunc*)QUATERNION_argmax;
  _PyQuaternion_ArrFuncs.nonzero = (PyArray_NonzeroFunc*)QUATERNION_nonzero;
  _PyQuaternion_ArrFuncs.fillwithscalar = (PyArray_FillWithScalarFunc*)QUATERNION_fillwithscalar;

  // The quaternion array descr
  quaternion_descr = PyObject_New(PyArray_Descr, &PyArrayDescr_Type);
  quaternion_descr->typeobj = &PyQuaternion_Type;
  quaternion_descr->kind = 'q';
  quaternion_descr->type = 'j';
  quaternion_descr->byteorder = '=';
  quaternion_descr->type_num = 0; // assigned at registration
  quaternion_descr->elsize = 8*4;
  quaternion_descr->alignment = 8;
  quaternion_descr->subarray = NULL;
  quaternion_descr->fields = NULL;
  quaternion_descr->names = NULL;
  quaternion_descr->f = &_PyQuaternion_ArrFuncs;

  Py_INCREF(&PyQuaternion_Type);
  quaternionNum = PyArray_RegisterDataType(quaternion_descr);

  if (quaternionNum < 0) {
#if defined(NPY_PY3K)
    return NULL;
#else
    return;
#endif
  }

  register_cast_function(NPY_BOOL, quaternionNum, (PyArray_VectorUnaryFunc*)BOOL_to_quaternion);
  register_cast_function(NPY_BYTE, quaternionNum, (PyArray_VectorUnaryFunc*)BYTE_to_quaternion);
  register_cast_function(NPY_UBYTE, quaternionNum, (PyArray_VectorUnaryFunc*)UBYTE_to_quaternion);
  register_cast_function(NPY_SHORT, quaternionNum, (PyArray_VectorUnaryFunc*)SHORT_to_quaternion);
  register_cast_function(NPY_USHORT, quaternionNum, (PyArray_VectorUnaryFunc*)USHORT_to_quaternion);
  register_cast_function(NPY_INT, quaternionNum, (PyArray_VectorUnaryFunc*)INT_to_quaternion);
  register_cast_function(NPY_UINT, quaternionNum, (PyArray_VectorUnaryFunc*)UINT_to_quaternion);
  register_cast_function(NPY_LONG, quaternionNum, (PyArray_VectorUnaryFunc*)LONG_to_quaternion);
  register_cast_function(NPY_ULONG, quaternionNum, (PyArray_VectorUnaryFunc*)ULONG_to_quaternion);
  register_cast_function(NPY_LONGLONG, quaternionNum, (PyArray_VectorUnaryFunc*)LONGLONG_to_quaternion);
  register_cast_function(NPY_ULONGLONG, quaternionNum, (PyArray_VectorUnaryFunc*)ULONGLONG_to_quaternion);
  register_cast_function(NPY_FLOAT, quaternionNum, (PyArray_VectorUnaryFunc*)FLOAT_to_quaternion);
  register_cast_function(NPY_DOUBLE, quaternionNum, (PyArray_VectorUnaryFunc*)DOUBLE_to_quaternion);
  register_cast_function(NPY_LONGDOUBLE, quaternionNum, (PyArray_VectorUnaryFunc*)LONGDOUBLE_to_quaternion);
  register_cast_function(NPY_CFLOAT, quaternionNum, (PyArray_VectorUnaryFunc*)CFLOAT_to_quaternion);
  register_cast_function(NPY_CDOUBLE, quaternionNum, (PyArray_VectorUnaryFunc*)CDOUBLE_to_quaternion);
  register_cast_function(NPY_CLONGDOUBLE, quaternionNum, (PyArray_VectorUnaryFunc*)CLONGDOUBLE_to_quaternion);


  // These macros will be used below
  #define REGISTER_UFUNC(name)                                          \
    PyUFunc_RegisterLoopForType((PyUFuncObject *)PyDict_GetItemString(numpy_dict, #name), \
    quaternion_descr->type_num, quaternion_##name##_ufunc, arg_types, NULL)
  #define REGISTER_SCALAR_UFUNC(name)                                   \
    PyUFunc_RegisterLoopForType((PyUFuncObject *)PyDict_GetItemString(numpy_dict, #name), \
    quaternion_descr->type_num, quaternion_##name##_scalar_ufunc, arg_types, NULL)


  // quat -> bool
  arg_types[0] = quaternion_descr->type_num;
  arg_types[1] = NPY_BOOL;
  REGISTER_UFUNC(isnan);
  REGISTER_UFUNC(isinf);
  REGISTER_UFUNC(isfinite);

  // quat -> double
  arg_types[1] = NPY_DOUBLE;
  REGISTER_UFUNC(absolute);

  // quat -> quat
  arg_types[1] = quaternion_descr->type_num;
  REGISTER_UFUNC(log);
  REGISTER_UFUNC(exp);
  REGISTER_UFUNC(negative);
  REGISTER_UFUNC(conjugate);

  // quat, quat -> bool
  arg_types[2] = NPY_BOOL;
  REGISTER_UFUNC(equal);
  REGISTER_UFUNC(not_equal);
  REGISTER_UFUNC(less);
  REGISTER_UFUNC(less_equal);

  // quat, double -> quat
  arg_types[1] = NPY_DOUBLE;
  arg_types[2] = quaternion_descr->type_num;
  REGISTER_SCALAR_UFUNC(multiply);
  REGISTER_SCALAR_UFUNC(divide);
  REGISTER_SCALAR_UFUNC(power);

  // quat, quat -> quat
  arg_types[1] = quaternion_descr->type_num;
  REGISTER_UFUNC(add);
  REGISTER_UFUNC(subtract);
  REGISTER_UFUNC(multiply);
  REGISTER_UFUNC(divide);
  REGISTER_UFUNC(power);
  REGISTER_UFUNC(copysign);

  // Finally, add this quaternion object to the numpy_quaternion module itself
  PyModule_AddObject(m, "quaternion", (PyObject *)&PyQuaternion_Type);

#if defined(NPY_PY3K)
    return m;
#else
    return;
#endif
}
