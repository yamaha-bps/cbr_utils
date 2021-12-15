#pragma once

// Python headers must be included before any system headers, since
// they define _POSIX_C_SOURCE
#include <Python.h>

#include <cbr_utils/utils.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>


#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#if PY_MAJOR_VERSION >= 3
#define PyString_FromString PyUnicode_FromString
#define PyInt_FromLong PyLong_FromLong
#define PyString_FromString PyUnicode_FromString
#endif

// Forward declare
namespace Eigen
{
template<typename Derived>
class DenseBase;
}

namespace matplotlibcpp
{
namespace detail
{

static std::string s_backend;

struct _interpreter
{
  PyObject * s_python_colormap;
  PyObject * s_python_empty_tuple;
  PyObject * s_python_function_annotate;
  PyObject * s_python_function_arrow;
  PyObject * s_python_function_axhline;
  PyObject * s_python_function_axis;
  PyObject * s_python_function_axvline;
  PyObject * s_python_function_axvspan;
  PyObject * s_python_function_bar;
  PyObject * s_python_function_barh;
  PyObject * s_python_function_boxplot;
  PyObject * s_python_function_cla;
  PyObject * s_python_function_clf;
  PyObject * s_python_function_close;
  PyObject * s_python_function_colorbar;
  PyObject * s_python_function_contour;
  PyObject * s_python_function_draw;
  PyObject * s_python_function_errorbar;
  PyObject * s_python_function_fignum_exists;
  PyObject * s_python_function_figure;
  PyObject * s_python_function_fill;
  PyObject * s_python_function_fill_between;
  PyObject * s_python_function_gca;
  PyObject * s_python_function_ginput;
  PyObject * s_python_function_grid;
  PyObject * s_python_function_hist;
  PyObject * s_python_function_imshow;
  PyObject * s_python_function_ion;
  PyObject * s_python_function_legend;
  PyObject * s_python_function_loglog;
  PyObject * s_python_function_margins;
  PyObject * s_python_function_pause;
  PyObject * s_python_function_plot;
  PyObject * s_python_function_quiver;
  PyObject * s_python_function_rcparams;
  PyObject * s_python_function_save;
  PyObject * s_python_function_scatter;
  PyObject * s_python_function_semilogx;
  PyObject * s_python_function_semilogy;
  PyObject * s_python_function_show;
  PyObject * s_python_function_spy;
  PyObject * s_python_function_stem;
  PyObject * s_python_function_subplot2grid;
  PyObject * s_python_function_subplot;
  PyObject * s_python_function_subplots_adjust;
  PyObject * s_python_function_suptitle;
  PyObject * s_python_function_text;
  PyObject * s_python_function_tick_params;
  PyObject * s_python_function_tight_layout;
  PyObject * s_python_function_title;
  PyObject * s_python_function_xkcd;
  PyObject * s_python_function_xlabel;
  PyObject * s_python_function_xlim;
  PyObject * s_python_function_xticks;
  PyObject * s_python_function_ylabel;
  PyObject * s_python_function_ylim;
  PyObject * s_python_function_yticks;

  /* For now, _interpreter is implemented as a singleton since its currently not possible to have
     multiple independent embedded python interpreters without patching the python source code
     or starting a separate process for each. [1]
     Furthermore, many python objects expect that they are destructed in the same thread as they
     were constructed. [2] So for advanced usage, a `kill()` function is provided so that library
     users can manually ensure that the interpreter is constructed and destroyed within the
     same thread.

       1:
     http://bytes.com/topic/python/answers/793370-multiple-independent-python-interpreters-c-c-program
       2: https://github.com/lava/matplotlib-cpp/pull/202#issue-436220256
     */

  static _interpreter & get() {return interkeeper(false);}

  static _interpreter & kill() {return interkeeper(true);}

  // Stores the actual singleton object referenced by `get()` and `kill()`.
  static _interpreter & interkeeper(bool should_kill)
  {
    static _interpreter ctx;
    if (should_kill) {ctx.~_interpreter();}
    return ctx;
  }

  PyObject * safe_import(PyObject * module, std::string fname)
  {
    PyObject * fn = PyObject_GetAttrString(module, fname.c_str());

    if (!fn) {throw std::runtime_error(std::string("Couldn't find required function: ") + fname);}

    if (!PyFunction_Check(fn)) {
      throw std::runtime_error(fname + std::string(" is unexpectedly not a PyFunction."));
    }

    return fn;
  }

private:
#if PY_MAJOR_VERSION >= 3

  void * import_numpy()
  {
    import_array();  // initialize C-API
    return NULL;
  }

#else

  void import_numpy()
  {
    import_array();  // initialize C-API
  }

#endif

  _interpreter()
  {

    // optional but recommended
#if PY_MAJOR_VERSION >= 3
    wchar_t name[] = L"plotting";
#else
    char name[] = "plotting";
#endif
    Py_SetProgramName(name);
    Py_Initialize();

    wchar_t const * dummy_args[] = {
      L"Python", NULL};  // const is needed because literals must not be modified
    wchar_t const ** argv = dummy_args;
    int argc = sizeof(dummy_args) / sizeof(dummy_args[0]) - 1;

#if PY_MAJOR_VERSION >= 3
    PySys_SetArgv(argc, const_cast<wchar_t **>(argv));
#else
    PySys_SetArgv(argc, (char **)(argv));
#endif

    import_numpy();  // initialize numpy C-API

    PyObject * matplotlibname = PyString_FromString("matplotlib");
    PyObject * pyplotname = PyString_FromString("matplotlib.pyplot");
    PyObject * cmname = PyString_FromString("matplotlib.cm");
    PyObject * pylabname = PyString_FromString("pylab");
    if (!pyplotname || !pylabname || !matplotlibname || !cmname) {
      throw std::runtime_error("couldnt create string");
    }

    PyObject * matplotlib = PyImport_Import(matplotlibname);

    Py_DECREF(matplotlibname);
    if (!matplotlib) {
      PyErr_Print();
      throw std::runtime_error("Error loading module matplotlib!");
    }

    // matplotlib.use() must be called *before* pylab, matplotlib.pyplot,
    // or matplotlib.backends is imported for the first time
    if (!s_backend.empty()) {
      PyObject_CallMethod(
        matplotlib, const_cast<char *>("use"), const_cast<char *>("s"), s_backend.c_str());
    }

    PyObject * pymod = PyImport_Import(pyplotname);
    Py_DECREF(pyplotname);
    if (!pymod) {throw std::runtime_error("Error loading module matplotlib.pyplot!");}

    s_python_colormap = PyImport_Import(cmname);
    Py_DECREF(cmname);
    if (!s_python_colormap) {throw std::runtime_error("Error loading module matplotlib.cm!");}

    PyObject * pylabmod = PyImport_Import(pylabname);
    Py_DECREF(pylabname);
    if (!pylabmod) {throw std::runtime_error("Error loading module pylab!");}

    s_python_empty_tuple = PyTuple_New(0);

    s_python_function_annotate = safe_import(pymod, "annotate");
    s_python_function_arrow = safe_import(pymod, "arrow");
    s_python_function_axhline = safe_import(pymod, "axhline");
    s_python_function_axis = safe_import(pymod, "axis");
    s_python_function_axvline = safe_import(pymod, "axvline");
    s_python_function_axvspan = safe_import(pymod, "axvspan");
    s_python_function_bar = safe_import(pymod, "bar");
    s_python_function_barh = safe_import(pymod, "barh");
    s_python_function_boxplot = safe_import(pymod, "boxplot");
    s_python_function_cla = safe_import(pymod, "cla");
    s_python_function_clf = safe_import(pymod, "clf");
    s_python_function_close = safe_import(pymod, "close");
    s_python_function_colorbar = PyObject_GetAttrString(pymod, "colorbar");
    s_python_function_contour = safe_import(pymod, "contour");
    s_python_function_draw = safe_import(pymod, "draw");
    s_python_function_errorbar = safe_import(pymod, "errorbar");
    s_python_function_fignum_exists = safe_import(pymod, "fignum_exists");
    s_python_function_figure = safe_import(pymod, "figure");
    s_python_function_fill = safe_import(pymod, "fill");
    s_python_function_fill_between = safe_import(pymod, "fill_between");
    s_python_function_gca = safe_import(pymod, "gca");
    s_python_function_ginput = safe_import(pymod, "ginput");
    s_python_function_grid = safe_import(pymod, "grid");
    s_python_function_hist = safe_import(pymod, "hist");
    s_python_function_imshow = safe_import(pymod, "imshow");
    s_python_function_ion = safe_import(pymod, "ion");
    s_python_function_legend = safe_import(pymod, "legend");
    s_python_function_loglog = safe_import(pymod, "loglog");
    s_python_function_margins = safe_import(pymod, "margins");
    s_python_function_pause = safe_import(pymod, "pause");
    s_python_function_plot = safe_import(pymod, "plot");
    s_python_function_quiver = safe_import(pymod, "quiver");
    s_python_function_rcparams = PyObject_GetAttrString(pymod, "rcParams");
    s_python_function_save = safe_import(pylabmod, "savefig");
    s_python_function_scatter = safe_import(pymod, "scatter");
    s_python_function_semilogx = safe_import(pymod, "semilogx");
    s_python_function_semilogy = safe_import(pymod, "semilogy");
    s_python_function_show = safe_import(pymod, "show");
    s_python_function_spy = PyObject_GetAttrString(pymod, "spy");
    s_python_function_stem = safe_import(pymod, "stem");
    s_python_function_subplot = safe_import(pymod, "subplot");
    s_python_function_subplot2grid = safe_import(pymod, "subplot2grid");
    s_python_function_subplots_adjust = safe_import(pymod, "subplots_adjust");
    s_python_function_suptitle = safe_import(pymod, "suptitle");
    s_python_function_text = safe_import(pymod, "text");
    s_python_function_tick_params = safe_import(pymod, "tick_params");
    s_python_function_tight_layout = safe_import(pymod, "tight_layout");
    s_python_function_title = safe_import(pymod, "title");
    s_python_function_xkcd = safe_import(pymod, "xkcd");
    s_python_function_xlabel = safe_import(pymod, "xlabel");
    s_python_function_xlim = safe_import(pymod, "xlim");
    s_python_function_xticks = safe_import(pymod, "xticks");
    s_python_function_ylabel = safe_import(pymod, "ylabel");
    s_python_function_ylim = safe_import(pymod, "ylim");
    s_python_function_yticks = safe_import(pymod, "yticks");
  }

  ~_interpreter() {Py_Finalize();}
};

}  // end namespace detail

/// Select the backend
///
/// **NOTE:** This must be called before the first plot command to have
/// any effect.
///
/// Mainly useful to select the non-interactive 'Agg' backend when running
/// matplotlibcpp in headless mode, for example on a machine with no display.
///
/// See also: https://matplotlib.org/2.0.2/api/matplotlib_configuration_api.html#matplotlib.use
inline void backend(const std::string & name) {detail::s_backend = name;}

namespace detail
{

// Type selector for numpy array conversion
template<typename T>
struct select_npy_type
{
  constexpr static NPY_TYPES type = NPY_NOTYPE;
};  // Default
template<>
struct select_npy_type<double>
{
  constexpr static NPY_TYPES type = NPY_DOUBLE;
};
template<>
struct select_npy_type<float>
{
  constexpr static NPY_TYPES type = NPY_FLOAT;
};
template<>
struct select_npy_type<bool>
{
  constexpr static NPY_TYPES type = NPY_BOOL;
};
template<>
struct select_npy_type<int8_t>
{
  constexpr static NPY_TYPES type = NPY_INT8;
};
template<>
struct select_npy_type<int16_t>
{
  constexpr static NPY_TYPES type = NPY_SHORT;
};
template<>
struct select_npy_type<int32_t>
{
  constexpr static NPY_TYPES type = NPY_INT;
};
template<>
struct select_npy_type<int64_t>
{
  constexpr static NPY_TYPES type = NPY_INT64;
};
template<>
struct select_npy_type<uint8_t>
{
  constexpr static NPY_TYPES type = NPY_UINT8;
};
template<>
struct select_npy_type<uint16_t>
{
  constexpr static NPY_TYPES type = NPY_USHORT;
};
template<>
struct select_npy_type<uint32_t>
{
  constexpr static NPY_TYPES type = NPY_ULONG;
};
template<>
struct select_npy_type<uint64_t>
{
  constexpr static NPY_TYPES type = NPY_UINT64;
};

// Sanity checks; comment them out or change the numpy type below if you're compiling on
// a platform where they don't apply
static_assert(sizeof(long long) == 8);
template<>
struct select_npy_type<long long>
{
  constexpr static NPY_TYPES type = NPY_INT64;
};
static_assert(sizeof(unsigned long long) == 8);
template<>
struct select_npy_type<unsigned long long>
{
  constexpr static NPY_TYPES type = NPY_UINT64;
};

// A couple concepts


template<class T>
concept numpy_type = (select_npy_type<T>::type != NPY_TYPES::NPY_NOTYPE);

template<class T>
concept not_numpy_type = not numpy_type<T>;

template<class T>
concept numpy_range = std::ranges::range<T>&& numpy_type<std::ranges::range_value_t<T>>;

template<class T>
concept numpy_matrix =
  std::ranges::range<T> and std::ranges::range<std::ranges::range_value_t<T>> and
  numpy_type<std::ranges::range_value_t<std::ranges::range_value_t<T>>>;

template<class T>
concept numeric_range = std::ranges::range<T>&& requires
{
  static_cast<double>(std::ranges::range_value_t<T>{});
};

template<class T>
concept numeric_matrix =
  std::ranges::range<T>&& std::ranges::range<std::ranges::range_value_t<T>>&& requires
{
  static_cast<double>(std::ranges::range_value_t<std::ranges::range_value_t<T>>{});
};

template<class T>
concept string_range =
  std::ranges::range<T>&& std::same_as<std::ranges::range_value_t<T>, std::string>;

template<class T>
concept has_data = requires(T v)
{
  v.data();
};

// Array converters
template<numpy_range T>
PyObject * get_array(const T & vs)
{
  using value_t = std::ranges::range_value_t<T>;
  constexpr NPY_TYPES type = select_npy_type<value_t>::type;

  const auto size = std::ranges::size(vs);
  npy_intp vsize = size;

  PyObject * varray = PyArray_SimpleNew(2, vsize, type);
  value_t * it = static_cast<value_t *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(varray)));

  for (const auto & v : vs) {
    *it = v;
    it++;
  }

  return varray;
}

template<numpy_range T>
requires(has_data<T>)
PyObject * get_array(const T & vs)
{
  constexpr NPY_TYPES type = select_npy_type<std::ranges::range_value_t<T>>::type;

  npy_intp vsize = std::ranges::size(vs);

  return PyArray_SimpleNewFromData(1, &vsize, type, (void *)(vs.data()));
}

template<numeric_range T>
requires not_numpy_type<std::ranges::range_value_t<T>>
PyObject * get_array(const T & vs)
{
  const auto size = std::ranges::size(vs);
  npy_intp vsize = size;

  PyObject * varray = PyArray_SimpleNew(1, &vsize, NPY_DOUBLE);
  double * it = static_cast<double *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(varray)));

  for (const auto & v : vs) {
    *it = static_cast<double>(v);
    it++;
  }

  return varray;
}


template<numpy_matrix T>
PyObject * get_2darray(const T & mat)
{
  if (std::ranges::size(mat) < 1) {throw std::runtime_error("get_2d_array v too small");}

  using value_t = std::ranges::range_value_t<std::ranges::range_value_t<T>>;
  constexpr NPY_TYPES type = select_npy_type<value_t>::type;

  const auto nRows = std::ranges::size(mat);
  const auto nCols = std::ranges::size(*std::ranges::begin(mat));
  npy_intp vsize[2] = {static_cast<npy_intp>(nRows), static_cast<npy_intp>(nCols)};

  PyObject * varray = PyArray_SimpleNew(2, vsize, type);

  value_t * it = static_cast<value_t *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(varray)));

  for (const auto & row : mat) {
    if (std::ranges::size(row) != nCols) {throw std::runtime_error("Missmatched array size");}
    for (const auto & v : row) {
      *it = v;
      it++;
    }
  }

  return varray;
}

template<numpy_matrix T>
requires(has_data<T>)
PyObject * get_2darray(const T & mat)
{
  if (std::ranges::size(mat) < 1) {throw std::runtime_error("get_2d_array v too small");}

  using value_t = std::ranges::range_value_t<std::ranges::range_value_t<T>>;
  constexpr NPY_TYPES type = select_npy_type<value_t>::type;

  const auto nRows = std::ranges::size(mat);
  const auto nCols = std::ranges::size(*std::ranges::begin(mat));
  npy_intp vsize[2] = {static_cast<npy_intp>(nRows), static_cast<npy_intp>(nCols)};

  return PyArray_SimpleNewFromData(2, vsize, type, (void *)(mat.data()));
}

template<numeric_matrix T>
requires not_numpy_type<std::ranges::range_value_t<std::ranges::range_value_t<T>>>
PyObject * get_2darray(const T & mat)
{
  if (std::ranges::size(mat) < 1) {throw std::runtime_error("get_2d_array v too small");}

  const auto nRows = std::ranges::size(mat);
  const auto nCols = std::ranges::size(*std::ranges::begin(mat));
  npy_intp vsize[2] = {static_cast<npy_intp>(nRows), static_cast<npy_intp>(nCols)};

  PyObject * varray = PyArray_SimpleNew(2, vsize, NPY_DOUBLE);

  double * it = static_cast<double *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(varray)));

  for (const auto & row : mat) {
    if (std::ranges::size(row) != nCols) {throw std::runtime_error("Missmatched array size");}
    for (const auto & v : row) {
      *it = static_cast<double>(v);
      it++;
    }
  }

  return varray;
}

// sometimes, for labels and such, we need string arrays
template<string_range T>
PyObject * get_array(const T & strings)
{
  PyObject * list = PyList_New(std::ranges::size(strings));
  std::size_t i = 0;
  for (const auto & s : strings) {
    PyList_SetItem(list, i++, PyString_FromString(s.c_str()));
  }
  return list;
}

// not all matplotlib need 2d arrays, some prefer lists of lists
template<numeric_matrix T>
PyObject * get_listlist(const T & ll)
{
  PyObject * listlist = PyList_New(std::ranges::size(ll));
  std::size_t i = 0;
  for (const auto & l : ll) {
    PyList_SetItem(listlist, i++, get_array(l));
  }
  return listlist;
}

// Converter of C++ type to PyObject type
struct PyObjectConvertor
{
  PyObject * operator()(const std::string & v) const {return PyString_FromString(v.c_str());}
  PyObject * operator()(const double v) const {return PyFloat_FromDouble(v);}
  PyObject * operator()(const bool v) const {return PyBool_FromLong(static_cast<long>(v));}
  PyObject * operator()(const long unsigned int v) const {return PyLong_FromLong(v);}

  template<std::ranges::range T>
  PyObject * operator()(const T & vs) const
  {
    PyObject * list = PyList_New(std::ranges::size(vs));
    std::size_t i = 0;
    for (const auto & v : vs) {
      PyList_SetItem(list, i, PyObjectConvertor{} (v));
      i++;
    }
    return list;
  }
};

// Handle kwargs
using varargin_value_t = typename std::variant<std::string, double, bool, std::vector<double>>;
using varargin_t = typename std::map<std::string, varargin_value_t>;

PyObject * to_PyObject(const varargin_value_t & var)
{
  return std::visit(PyObjectConvertor{}, var);
}

void update_kwargs(PyObject * kwargs, const detail::varargin_t & keywords)
{
  for (const auto & [key, val] : keywords) {
    PyDict_SetItemString(kwargs, key.c_str(), to_PyObject(val));
  }
}

PyObject * create_kwargs(const detail::varargin_t & keywords)
{
  PyObject * kwargs = PyDict_New();
  update_kwargs(kwargs, keywords);
  return kwargs;
}

}  // namespace detail

inline bool annotate(std::string annotation, double x, double y)
{
  detail::_interpreter::get();

  PyObject * xy = PyTuple_New(2);
  PyObject * str = PyString_FromString(annotation.c_str());

  PyTuple_SetItem(xy, 0, PyFloat_FromDouble(x));
  PyTuple_SetItem(xy, 1, PyFloat_FromDouble(y));

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "xy", xy);

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, str);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_annotate, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);

  if (res) {Py_DECREF(res);}

  return res;
}


/// Plot a line through the given x and y data points..
///
/// See: https://matplotlib.org/3.2.1/api/_as_gen/matplotlib.pyplot.plot.html
template<typename T1, typename T2>
bool plot(const T1 & x, const T2 & y, const detail::varargin_t & keywords)
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  // using numpy arrays
  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  // construct positional args
  PyObject * args = PyTuple_New(2);
  PyTuple_SetItem(args, 0, xarray);
  PyTuple_SetItem(args, 1, yarray);

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);
  PyObject * res = PyObject_Call(detail::_interpreter::get().s_python_function_plot, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

// TODO - it should be possible to make this work by implementing
// a non-numpy alternative for `detail::get_2darray()`.
template<typename T1, typename T2, typename T3>
void plot_surface(
  const T1 & x,
  const T2 & y,
  const T3 & z,
  const detail::varargin_t & keywords = {},
  const long fig_number = 0)
{
  detail::_interpreter::get();

  // We lazily load the modules here the first time this function is called
  // because I'm not sure that we can assume "matplotlib installed" implies
  // "mpl_toolkits installed" on all platforms, and we don't want to require
  // it for people who don't need 3d plots.
  static PyObject * mpl_toolkitsmod = nullptr, * axis3dmod = nullptr;
  if (!mpl_toolkitsmod) {
    detail::_interpreter::get();

    PyObject * mpl_toolkits = PyString_FromString("mpl_toolkits");
    PyObject * axis3d = PyString_FromString("mpl_toolkits.mplot3d");
    if (!mpl_toolkits || !axis3d) {throw std::runtime_error("couldnt create string");}

    mpl_toolkitsmod = PyImport_Import(mpl_toolkits);
    Py_DECREF(mpl_toolkits);
    if (!mpl_toolkitsmod) {throw std::runtime_error("Error loading module mpl_toolkits!");}

    axis3dmod = PyImport_Import(axis3d);
    Py_DECREF(axis3d);
    if (!axis3dmod) {throw std::runtime_error("Error loading module mpl_toolkits.mplot3d!");}
  }

  assert(x.size() == y.size());
  assert(y.size() == z.size());

  // using numpy arrays
  PyObject * xarray = detail::get_2darray(x);
  PyObject * yarray = detail::get_2darray(y);
  PyObject * zarray = detail::get_2darray(z);

  // construct positional args
  PyObject * args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, xarray);
  PyTuple_SetItem(args, 1, yarray);
  PyTuple_SetItem(args, 2, zarray);

  // Build up the kw args.
  PyObject * kwargs = detail::create_kwargs(keywords);
  PyDict_SetItemString(kwargs, "rstride", PyInt_FromLong(1));
  PyDict_SetItemString(kwargs, "cstride", PyInt_FromLong(1));

  PyObject * python_colormap_coolwarm =
    PyObject_GetAttrString(detail::_interpreter::get().s_python_colormap, "coolwarm");

  PyDict_SetItemString(kwargs, "cmap", python_colormap_coolwarm);

  PyObject * fig_args = PyTuple_New(1);
  PyObject * fig = nullptr;
  PyTuple_SetItem(fig_args, 0, PyLong_FromLong(fig_number));
  PyObject * fig_exists =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_fignum_exists, fig_args);
  if (!PyObject_IsTrue(fig_exists)) {
    fig = PyObject_CallObject(
      detail::_interpreter::get().s_python_function_figure,
      detail::_interpreter::get().s_python_empty_tuple);
  } else {
    fig = PyObject_CallObject(detail::_interpreter::get().s_python_function_figure, fig_args);
  }
  Py_DECREF(fig_exists);
  if (!fig) {throw std::runtime_error("Call to figure() failed.");}

  PyObject * gca_kwargs = PyDict_New();
  PyDict_SetItemString(gca_kwargs, "projection", PyString_FromString("3d"));

  PyObject * gca = PyObject_GetAttrString(fig, "gca");
  if (!gca) {throw std::runtime_error("No gca");}
  Py_INCREF(gca);
  PyObject * axis =
    PyObject_Call(gca, detail::_interpreter::get().s_python_empty_tuple, gca_kwargs);

  if (!axis) {throw std::runtime_error("No axis");}
  Py_INCREF(axis);

  Py_DECREF(gca);
  Py_DECREF(gca_kwargs);

  PyObject * plot_surface = PyObject_GetAttrString(axis, "plot_surface");
  if (!plot_surface) {throw std::runtime_error("No surface");}
  Py_INCREF(plot_surface);
  PyObject * res = PyObject_Call(plot_surface, args, kwargs);
  if (!res) {throw std::runtime_error("failed surface");}
  Py_DECREF(plot_surface);

  Py_DECREF(axis);
  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}
}

template<typename T1, typename T2, typename T3, typename T4>
void contour(
  const T1 & x, const T2 & y, const T3 & z, const T4 & c, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  // using numpy arrays
  PyObject * xarray = detail::get_2darray(x);
  PyObject * yarray = detail::get_2darray(y);
  PyObject * zarray = detail::get_2darray(z);
  PyObject * carray = detail::get_array(c);

  // construct positional args
  PyObject * args = PyTuple_New(4);
  PyTuple_SetItem(args, 0, xarray);
  PyTuple_SetItem(args, 1, yarray);
  PyTuple_SetItem(args, 2, zarray);
  PyTuple_SetItem(args, 3, carray);

  // Build up the kw args.
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * python_colormap_coolwarm =
    PyObject_GetAttrString(detail::_interpreter::get().s_python_colormap, "coolwarm");

  if (!keywords.contains("colors")) {
    PyDict_SetItemString(kwargs, "cmap", python_colormap_coolwarm);
  }

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_contour, args, kwargs);
  if (!res) {throw std::runtime_error("failed contour");}

  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}
}

template<typename T>
void spy(const T & x, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * xarray = detail::get_2darray(x);

  PyObject * plot_args = PyTuple_New(1);
  PyTuple_SetItem(plot_args, 0, xarray);

  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_spy, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}
}

template<typename T1, typename T2, typename T3>
void plot3(
  const T1 & x,
  const T2 & y,
  const T3 & z,
  const detail::varargin_t & keywords = {},
  const long fig_number = 0)
{
  detail::_interpreter::get();

  // Same as with plot_surface: We lazily load the modules here the first time
  // this function is called because I'm not sure that we can assume "matplotlib
  // installed" implies "mpl_toolkits installed" on all platforms, and we don't
  // want to require it for people who don't need 3d plots.
  static PyObject * mpl_toolkitsmod = nullptr, * axis3dmod = nullptr;
  if (!mpl_toolkitsmod) {
    detail::_interpreter::get();

    PyObject * mpl_toolkits = PyString_FromString("mpl_toolkits");
    PyObject * axis3d = PyString_FromString("mpl_toolkits.mplot3d");
    if (!mpl_toolkits || !axis3d) {throw std::runtime_error("couldnt create string");}

    mpl_toolkitsmod = PyImport_Import(mpl_toolkits);
    Py_DECREF(mpl_toolkits);
    if (!mpl_toolkitsmod) {throw std::runtime_error("Error loading module mpl_toolkits!");}

    axis3dmod = PyImport_Import(axis3d);
    Py_DECREF(axis3d);
    if (!axis3dmod) {throw std::runtime_error("Error loading module mpl_toolkits.mplot3d!");}
  }

  assert(x.size() == y.size());
  assert(y.size() == z.size());

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);
  PyObject * zarray = detail::get_array(z);

  // construct positional args
  PyObject * args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, xarray);
  PyTuple_SetItem(args, 1, yarray);
  PyTuple_SetItem(args, 2, zarray);

  // Build up the kw args.
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * fig_args = PyTuple_New(1);
  PyObject * fig = nullptr;
  PyTuple_SetItem(fig_args, 0, PyLong_FromLong(fig_number));
  PyObject * fig_exists =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_fignum_exists, fig_args);
  if (!PyObject_IsTrue(fig_exists)) {
    fig = PyObject_CallObject(
      detail::_interpreter::get().s_python_function_figure,
      detail::_interpreter::get().s_python_empty_tuple);
  } else {
    fig = PyObject_CallObject(detail::_interpreter::get().s_python_function_figure, fig_args);
  }
  if (!fig) {throw std::runtime_error("Call to figure() failed.");}

  PyObject * gca_kwargs = PyDict_New();
  PyDict_SetItemString(gca_kwargs, "projection", PyString_FromString("3d"));

  PyObject * gca = PyObject_GetAttrString(fig, "gca");
  if (!gca) {throw std::runtime_error("No gca");}
  Py_INCREF(gca);
  PyObject * axis =
    PyObject_Call(gca, detail::_interpreter::get().s_python_empty_tuple, gca_kwargs);

  if (!axis) {throw std::runtime_error("No axis");}
  Py_INCREF(axis);

  Py_DECREF(gca);
  Py_DECREF(gca_kwargs);

  PyObject * plot3 = PyObject_GetAttrString(axis, "plot");
  if (!plot3) {throw std::runtime_error("No 3D line plot");}
  Py_INCREF(plot3);
  PyObject * res = PyObject_Call(plot3, args, kwargs);
  if (!res) {throw std::runtime_error("Failed 3D line plot");}
  Py_DECREF(plot3);

  Py_DECREF(axis);
  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}
}

template<typename T1, typename T2>
bool stem(const T1 & x, const T2 & y, const detail::varargin_t & keywords = {})
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  // using numpy arrays
  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  // construct positional args
  PyObject * args = PyTuple_New(2);
  PyTuple_SetItem(args, 0, xarray);
  PyTuple_SetItem(args, 1, yarray);

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res = PyObject_Call(detail::_interpreter::get().s_python_function_stem, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool fill(const T1 & x, const T2 & y, const detail::varargin_t & keywords = {})
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  // using numpy arrays
  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  // construct positional args
  PyObject * args = PyTuple_New(2);
  PyTuple_SetItem(args, 0, xarray);
  PyTuple_SetItem(args, 1, yarray);

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res = PyObject_Call(detail::_interpreter::get().s_python_function_fill, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);

  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2, typename T3>
bool fill_between(
  const T1 & x, const T2 & y1, const T3 & y2, const detail::varargin_t & keywords = {})
{
  assert(x.size() == y1.size());
  assert(x.size() == y2.size());

  detail::_interpreter::get();

  // using numpy arrays
  PyObject * xarray = detail::get_array(x);
  PyObject * y1array = detail::get_array(y1);
  PyObject * y2array = detail::get_array(y2);

  // construct positional args
  PyObject * args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, xarray);
  PyTuple_SetItem(args, 1, y1array);
  PyTuple_SetItem(args, 2, y2array);

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_fill_between, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename Numeric>
bool arrow(
  Numeric x,
  Numeric y,
  Numeric end_x,
  Numeric end_y,
  const std::string & fc = "r",
  const std::string ec = "k",
  const detail::varargin_t & keywords = {})
{
  PyObject * obj_x = PyFloat_FromDouble(x);
  PyObject * obj_y = PyFloat_FromDouble(y);
  PyObject * obj_end_x = PyFloat_FromDouble(end_x);
  PyObject * obj_end_y = PyFloat_FromDouble(end_y);

  PyObject * kwargs = detail::create_kwargs(keywords);
  PyDict_SetItemString(kwargs, "fc", PyString_FromString(fc.c_str()));
  PyDict_SetItemString(kwargs, "ec", PyString_FromString(ec.c_str()));

  PyObject * plot_args = PyTuple_New(4);
  PyTuple_SetItem(plot_args, 0, obj_x);
  PyTuple_SetItem(plot_args, 1, obj_y);
  PyTuple_SetItem(plot_args, 2, obj_end_x);
  PyTuple_SetItem(plot_args, 3, obj_end_y);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_arrow, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T>
bool hist(
  const T & y,
  long bins = 10,
  std::string color = "b",
  double alpha = 1.0,
  bool cumulative = false,
  const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * yarray = detail::get_array(y);

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "bins", PyLong_FromLong(bins));
  PyDict_SetItemString(kwargs, "color", PyString_FromString(color.c_str()));
  PyDict_SetItemString(kwargs, "alpha", PyFloat_FromDouble(alpha));
  PyDict_SetItemString(kwargs, "cumulative", cumulative ? Py_True : Py_False);
  detail::update_kwargs(kwargs, keywords);

  PyObject * plot_args = PyTuple_New(1);

  PyTuple_SetItem(plot_args, 0, yarray);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_hist, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

namespace detail
{

inline void imshow(
  const void * ptr,
  const NPY_TYPES type,
  const int rows,
  const int columns,
  const int colors,
  const detail::varargin_t & keywords,
  PyObject ** out)
{
  assert(colors == 1 || colors == 3 || colors == 4);

  detail::_interpreter::get();

  // construct args
  npy_intp dims[3] = {rows, columns, colors};
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(
    args, 0, PyArray_SimpleNewFromData(colors == 1 ? 2 : 3, dims, type, const_cast<void *>(ptr)));

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_imshow, args, kwargs);
  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (!res) {throw std::runtime_error("Call to imshow() failed");}
  if (out) {
    *out = res;
  } else {
    Py_DECREF(res);
  }
}

}  // namespace detail

template<typename T>
inline void imshow(
  const T * ptr,
  const int rows,
  const int columns,
  const int colors,
  const detail::varargin_t & keywords = {},
  PyObject ** out = nullptr)
{

  detail::imshow(
    static_cast<const void *>(ptr),
    detail::select_npy_type<T>::type,
    rows,
    columns,
    colors,
    keywords,
    out);
}

template<typename T1, typename T2>
bool scatter(
  const T1 & x,
  const T2 & y,
  const double s = 1.0,                       // The marker size in points**2
  const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  assert(x.size() == y.size());

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "s", PyLong_FromLong(s));
  detail::update_kwargs(kwargs, keywords);

  PyObject * plot_args = PyTuple_New(2);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_scatter, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2, typename T3>
bool scatter_colored(
  const T1 & x,
  const T2 & y,
  const T3 & colors,
  const double s = 1.0,                       // The marker size in points**2
  const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  assert(x.size() == y.size());

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);
  PyObject * colors_array = detail::get_array(colors);

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "s", PyLong_FromLong(s));
  PyDict_SetItemString(kwargs, "c", colors_array);
  detail::update_kwargs(kwargs, keywords);

  PyObject * plot_args = PyTuple_New(2);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_scatter, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2, typename T3>
bool scatter(
  const T1 & x,
  const T2 & y,
  const T3 & z,
  const double s = 1.0,                       // The marker size in points**2
  const detail::varargin_t & keywords = {},
  const long fig_number = 0)
{
  detail::_interpreter::get();

  // Same as with plot_surface: We lazily load the modules here the first time
  // this function is called because I'm not sure that we can assume "matplotlib
  // installed" implies "mpl_toolkits installed" on all platforms, and we don't
  // want to require it for people who don't need 3d plots.
  static PyObject * mpl_toolkitsmod = nullptr, * axis3dmod = nullptr;
  if (!mpl_toolkitsmod) {
    detail::_interpreter::get();

    PyObject * mpl_toolkits = PyString_FromString("mpl_toolkits");
    PyObject * axis3d = PyString_FromString("mpl_toolkits.mplot3d");
    if (!mpl_toolkits || !axis3d) {throw std::runtime_error("couldnt create string");}

    mpl_toolkitsmod = PyImport_Import(mpl_toolkits);
    Py_DECREF(mpl_toolkits);
    if (!mpl_toolkitsmod) {throw std::runtime_error("Error loading module mpl_toolkits!");}

    axis3dmod = PyImport_Import(axis3d);
    Py_DECREF(axis3d);
    if (!axis3dmod) {throw std::runtime_error("Error loading module mpl_toolkits.mplot3d!");}
  }

  assert(x.size() == y.size());
  assert(y.size() == z.size());

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);
  PyObject * zarray = detail::get_array(z);

  // construct positional args
  PyObject * args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, xarray);
  PyTuple_SetItem(args, 1, yarray);
  PyTuple_SetItem(args, 2, zarray);

  // Build up the kw args.
  PyObject * kwargs = detail::create_kwargs(keywords);
  PyObject * fig_args = PyTuple_New(1);
  PyObject * fig = nullptr;
  PyTuple_SetItem(fig_args, 0, PyLong_FromLong(fig_number));
  PyObject * fig_exists =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_fignum_exists, fig_args);
  if (!PyObject_IsTrue(fig_exists)) {
    fig = PyObject_CallObject(
      detail::_interpreter::get().s_python_function_figure,
      detail::_interpreter::get().s_python_empty_tuple);
  } else {
    fig = PyObject_CallObject(detail::_interpreter::get().s_python_function_figure, fig_args);
  }
  Py_DECREF(fig_exists);
  if (!fig) {throw std::runtime_error("Call to figure() failed.");}

  PyObject * gca_kwargs = PyDict_New();
  PyDict_SetItemString(gca_kwargs, "projection", PyString_FromString("3d"));

  PyObject * gca = PyObject_GetAttrString(fig, "gca");
  if (!gca) {throw std::runtime_error("No gca");}
  Py_INCREF(gca);
  PyObject * axis =
    PyObject_Call(gca, detail::_interpreter::get().s_python_empty_tuple, gca_kwargs);

  if (!axis) {throw std::runtime_error("No axis");}
  Py_INCREF(axis);

  Py_DECREF(gca);
  Py_DECREF(gca_kwargs);

  PyObject * plot3 = PyObject_GetAttrString(axis, "scatter");
  if (!plot3) {throw std::runtime_error("No 3D line plot");}
  Py_INCREF(plot3);
  PyObject * res = PyObject_Call(plot3, args, kwargs);
  if (!res) {throw std::runtime_error("Failed 3D line plot");}
  Py_DECREF(plot3);

  Py_DECREF(axis);
  Py_DECREF(args);
  Py_DECREF(kwargs);
  Py_DECREF(fig);
  if (res) {Py_DECREF(res);}
  return res;
}

template<typename T>
bool boxplot(
  const T & data,
  const std::vector<std::string> & labels = {},
  const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * listlist = detail::get_listlist(data);
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, listlist);

  PyObject * kwargs = PyDict_New();

  // kwargs needs the labels, if there are (the correct number of) labels
  if (!labels.empty() && labels.size() == data.size()) {
    PyDict_SetItemString(kwargs, "labels", detail::get_array(labels));
  }

  // take care of the remaining keywords
  detail::update_kwargs(kwargs, keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_boxplot, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);

  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T>
bool boxplot(const T & data, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * vector = detail::get_array(data);
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, vector);

  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_boxplot, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);

  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T>
bool bar(
  const T & x,
  const T & y,
  std::string ec = "black",
  std::string ls = "-",
  double lw = 1.0,
  const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * kwargs = PyDict_New();

  PyDict_SetItemString(kwargs, "ec", PyString_FromString(ec.c_str()));
  PyDict_SetItemString(kwargs, "ls", PyString_FromString(ls.c_str()));
  PyDict_SetItemString(kwargs, "lw", PyFloat_FromDouble(lw));

  detail::update_kwargs(kwargs, keywords);

  PyObject * plot_args = PyTuple_New(2);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_bar, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T>
bool bar(
  const T & y,
  std::string ec = "black",
  std::string ls = "-",
  double lw = 1.0,
  const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  return bar(
    std::views::iota(static_cast<decltype(y.size())>(0), y.size()), y, ec, ls, lw, keywords);
}

template<typename T>
bool barh(
  const T & x,
  const T & y,
  std::string ec = "black",
  std::string ls = "-",
  double lw = 1.0,
  const detail::varargin_t & keywords = {})
{
  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * kwargs = PyDict_New();

  PyDict_SetItemString(kwargs, "ec", PyString_FromString(ec.c_str()));
  PyDict_SetItemString(kwargs, "ls", PyString_FromString(ls.c_str()));
  PyDict_SetItemString(kwargs, "lw", PyFloat_FromDouble(lw));

  detail::update_kwargs(kwargs, keywords);

  PyObject * plot_args = PyTuple_New(2);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_barh, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

inline bool subplots_adjust(const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * plot_args = PyTuple_New(0);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_subplots_adjust, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T>
bool named_hist(
  std::string label, const T & y, long bins = 10, std::string color = "b", double alpha = 1.0)
{
  detail::_interpreter::get();

  PyObject * yarray = detail::get_array(y);

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "label", PyString_FromString(label.c_str()));
  PyDict_SetItemString(kwargs, "bins", PyLong_FromLong(bins));
  PyDict_SetItemString(kwargs, "color", PyString_FromString(color.c_str()));
  PyDict_SetItemString(kwargs, "alpha", PyFloat_FromDouble(alpha));

  PyObject * plot_args = PyTuple_New(1);
  PyTuple_SetItem(plot_args, 0, yarray);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_hist, plot_args, kwargs);

  Py_DECREF(plot_args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool plot(const T1 & x, const T2 & y, const std::string & s = "")
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(s.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);

  PyObject * res =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_plot, plot_args);

  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2, typename T3>
bool contour(const T1 & x, const T2 & y, const T3 & z, const detail::varargin_t & keywords = {})
{
  assert(x.size() == y.size() && x.size() == z.size());

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);
  PyObject * zarray = detail::get_array(z);

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, zarray);

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_contour, plot_args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2, typename T3, typename T4>
bool quiver(
  const T1 & x, const T2 & y, const T3 & u, const T4 & w, const detail::varargin_t & keywords = {})
{
  assert(x.size() == y.size() && x.size() == u.size() && u.size() == w.size());

  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);
  PyObject * uarray = detail::get_array(u);
  PyObject * warray = detail::get_array(w);

  PyObject * plot_args = PyTuple_New(4);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, uarray);
  PyTuple_SetItem(plot_args, 3, warray);

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_quiver, plot_args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool quiver(
  const T1 & x,
  const T2 & y,
  const T3 & z,
  const T4 & u,
  const T5 & w,
  const T6 & v,
  const detail::varargin_t & keywords = {})
{
  // set up 3d axes stuff
  static PyObject * mpl_toolkitsmod = nullptr, * axis3dmod = nullptr;
  if (!mpl_toolkitsmod) {
    detail::_interpreter::get();

    PyObject * mpl_toolkits = PyString_FromString("mpl_toolkits");
    PyObject * axis3d = PyString_FromString("mpl_toolkits.mplot3d");
    if (!mpl_toolkits || !axis3d) {throw std::runtime_error("couldnt create string");}

    mpl_toolkitsmod = PyImport_Import(mpl_toolkits);
    Py_DECREF(mpl_toolkits);
    if (!mpl_toolkitsmod) {throw std::runtime_error("Error loading module mpl_toolkits!");}

    axis3dmod = PyImport_Import(axis3d);
    Py_DECREF(axis3d);
    if (!axis3dmod) {throw std::runtime_error("Error loading module mpl_toolkits.mplot3d!");}
  }

  // assert sizes match up
  assert(
    x.size() == y.size() && x.size() == u.size() && u.size() == w.size() &&
    x.size() == z.size() && x.size() == v.size() && u.size() == v.size());

  // set up parameters
  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);
  PyObject * zarray = detail::get_array(z);
  PyObject * uarray = detail::get_array(u);
  PyObject * warray = detail::get_array(w);
  PyObject * varray = detail::get_array(v);

  PyObject * plot_args = PyTuple_New(6);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, zarray);
  PyTuple_SetItem(plot_args, 3, uarray);
  PyTuple_SetItem(plot_args, 4, warray);
  PyTuple_SetItem(plot_args, 5, varray);

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  // get figure gca to enable 3d projection
  PyObject * fig = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_figure,
    detail::_interpreter::get().s_python_empty_tuple);
  if (!fig) {throw std::runtime_error("Call to figure() failed.");}

  PyObject * gca_kwargs = PyDict_New();
  PyDict_SetItemString(gca_kwargs, "projection", PyString_FromString("3d"));

  PyObject * gca = PyObject_GetAttrString(fig, "gca");
  if (!gca) {throw std::runtime_error("No gca");}
  Py_INCREF(gca);
  PyObject * axis =
    PyObject_Call(gca, detail::_interpreter::get().s_python_empty_tuple, gca_kwargs);

  if (!axis) {throw std::runtime_error("No axis");}
  Py_INCREF(axis);
  Py_DECREF(gca);
  Py_DECREF(gca_kwargs);

  // plot our boys bravely, plot them strongly, plot them with a wink and clap
  PyObject * plot3 = PyObject_GetAttrString(axis, "quiver");
  if (!plot3) {throw std::runtime_error("No 3D line plot");}
  Py_INCREF(plot3);
  PyObject * res = PyObject_Call(plot3, plot_args, kwargs);
  if (!res) {throw std::runtime_error("Failed 3D plot");}
  Py_DECREF(plot3);
  Py_DECREF(axis);
  Py_DECREF(kwargs);
  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool stem(const T1 & x, const T2 & y, const std::string & s = "")
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(s.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);

  PyObject * res =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_stem, plot_args);

  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool semilogx(const T1 & x, const T2 & y, const std::string & s = "")
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(s.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);

  PyObject * res =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_semilogx, plot_args);

  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool semilogy(const T1 & x, const T2 & y, const std::string & s = "")
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(s.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);

  PyObject * res =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_semilogy, plot_args);

  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool loglog(const T1 & x, const T2 & y, const std::string & s = "")
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(s.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);

  PyObject * res =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_loglog, plot_args);

  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2, typename T3>
bool errorbar(const T1 & x, const T2 & y, const T3 & yerr, const detail::varargin_t & keywords = {})
{
  assert(x.size() == y.size());

  detail::_interpreter::get();

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);
  PyObject * yerrarray = detail::get_array(yerr);

  // construct keyword args
  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "yerr", yerrarray);
  detail::update_kwargs(kwargs, keywords);

  PyObject * plot_args = PyTuple_New(2);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_errorbar, plot_args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(plot_args);

  if (res) {
    Py_DECREF(res);
  } else {
    throw std::runtime_error("Call to errorbar() failed.");
  }

  return res;
}

template<typename T>
bool named_plot(const std::string & name, const T & y, const std::string & format = "")
{
  detail::_interpreter::get();

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "label", PyString_FromString(name.c_str()));

  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(format.c_str());

  PyObject * plot_args = PyTuple_New(2);

  PyTuple_SetItem(plot_args, 0, yarray);
  PyTuple_SetItem(plot_args, 1, pystring);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_plot, plot_args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool named_plot(
  const std::string & name, const T1 & x, const T2 & y, const std::string & format = "")
{
  detail::_interpreter::get();

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "label", PyString_FromString(name.c_str()));

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(format.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_plot, plot_args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool named_semilogx(
  const std::string & name, const T1 & x, const T2 & y, const std::string & format = "")
{
  detail::_interpreter::get();

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "label", PyString_FromString(name.c_str()));

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(format.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_semilogx, plot_args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool named_semilogy(
  const std::string & name, const T1 & x, const T2 & y, const std::string & format = "")
{
  detail::_interpreter::get();

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "label", PyString_FromString(name.c_str()));

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(format.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_semilogy, plot_args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T1, typename T2>
bool named_loglog(
  const std::string & name, const T1 & x, const T2 & y, const std::string & format = "")
{
  detail::_interpreter::get();

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "label", PyString_FromString(name.c_str()));

  PyObject * xarray = detail::get_array(x);
  PyObject * yarray = detail::get_array(y);

  PyObject * pystring = PyString_FromString(format.c_str());

  PyObject * plot_args = PyTuple_New(3);
  PyTuple_SetItem(plot_args, 0, xarray);
  PyTuple_SetItem(plot_args, 1, yarray);
  PyTuple_SetItem(plot_args, 2, pystring);
  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_loglog, plot_args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(plot_args);
  if (res) {Py_DECREF(res);}

  return res;
}

template<typename T>
bool plot(const T & y, const std::string & format = "")
{
  return plot(std::views::iota(static_cast<decltype(y.size())>(0), y.size()), y, format);
}

template<typename T>
bool plot(const T & y, const detail::varargin_t & keywords)
{
  return plot(std::views::iota(static_cast<decltype(y.size())>(0), y.size()), y, keywords);
}

template<typename T>
bool stem(const T & y, const std::string & format = "")
{
  return stem(std::views::iota(static_cast<decltype(y.size())>(0), y.size()), y, format);
}

template<typename T>
void text(T x, T y, const std::string & s = "")
{
  detail::_interpreter::get();

  PyObject * args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(x));
  PyTuple_SetItem(args, 1, PyFloat_FromDouble(y));
  PyTuple_SetItem(args, 2, PyString_FromString(s.c_str()));

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_text, args);
  if (!res) {throw std::runtime_error("Call to text() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

inline void colorbar(PyObject * mappable = NULL, const std::map<std::string, float> & keywords = {})
{
  if (mappable == NULL) {
    throw std::runtime_error(
            "Must call colorbar with PyObject* returned from an image, contour, surface, etc.");
  }

  detail::_interpreter::get();

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, mappable);

  PyObject * kwargs = PyDict_New();
  for (std::map<std::string, float>::const_iterator it = keywords.begin(); it != keywords.end();
    ++it)
  {
    PyDict_SetItemString(kwargs, it->first.c_str(), PyFloat_FromDouble(it->second));
  }

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_colorbar, args, kwargs);
  if (!res) {throw std::runtime_error("Call to colorbar() failed.");}

  Py_DECREF(args);
  Py_DECREF(kwargs);
  Py_DECREF(res);
}

inline long figure(long number = -1)
{
  detail::_interpreter::get();

  PyObject * res;
  if (number == -1) {
    res = PyObject_CallObject(
      detail::_interpreter::get().s_python_function_figure,
      detail::_interpreter::get().s_python_empty_tuple);
  } else {
    assert(number > 0);

    // Make sure interpreter is initialised
    detail::_interpreter::get();

    PyObject * args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, PyLong_FromLong(number));
    res = PyObject_CallObject(detail::_interpreter::get().s_python_function_figure, args);
    Py_DECREF(args);
  }

  if (!res) {throw std::runtime_error("Call to figure() failed.");}

  PyObject * num = PyObject_GetAttrString(res, "number");
  if (!num) {throw std::runtime_error("Could not get number attribute of figure object");}
  const long figureNumber = PyLong_AsLong(num);

  Py_DECREF(num);
  Py_DECREF(res);

  return figureNumber;
}

inline bool fignum_exists(long number)
{
  detail::_interpreter::get();

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyLong_FromLong(number));
  PyObject * res =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_fignum_exists, args);
  if (!res) {throw std::runtime_error("Call to fignum_exists() failed.");}

  bool ret = PyObject_IsTrue(res);
  Py_DECREF(res);
  Py_DECREF(args);

  return ret;
}

inline void figure_size(size_t w, size_t h)
{
  detail::_interpreter::get();

  const size_t dpi = 100;
  PyObject * size = PyTuple_New(2);
  PyTuple_SetItem(size, 0, PyFloat_FromDouble((double)w / dpi));
  PyTuple_SetItem(size, 1, PyFloat_FromDouble((double)h / dpi));

  PyObject * kwargs = PyDict_New();
  PyDict_SetItemString(kwargs, "figsize", size);
  PyDict_SetItemString(kwargs, "dpi", PyLong_FromSize_t(dpi));

  PyObject * res = PyObject_Call(
    detail::_interpreter::get().s_python_function_figure,
    detail::_interpreter::get().s_python_empty_tuple,
    kwargs);

  Py_DECREF(kwargs);

  if (!res) {throw std::runtime_error("Call to figure_size() failed.");}
  Py_DECREF(res);
}

inline void legend()
{
  detail::_interpreter::get();

  PyObject * res = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_legend,
    detail::_interpreter::get().s_python_empty_tuple);
  if (!res) {throw std::runtime_error("Call to legend() failed.");}

  Py_DECREF(res);
}

inline void legend(const detail::varargin_t & keywords)
{
  detail::_interpreter::get();

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res = PyObject_Call(
    detail::_interpreter::get().s_python_function_legend,
    detail::_interpreter::get().s_python_empty_tuple,
    kwargs);
  if (!res) {throw std::runtime_error("Call to legend() failed.");}

  Py_DECREF(kwargs);
  Py_DECREF(res);
}

template<typename T>
inline void set_aspect(T ratio)
{
  detail::_interpreter::get();

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(ratio));
  PyObject * kwargs = PyDict_New();

  PyObject * ax = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_gca,
    detail::_interpreter::get().s_python_empty_tuple);
  if (!ax) {throw std::runtime_error("Call to gca() failed.");}
  Py_INCREF(ax);

  PyObject * set_aspect = PyObject_GetAttrString(ax, "set_aspect");
  if (!set_aspect) {throw std::runtime_error("Attribute set_aspect not found.");}
  Py_INCREF(set_aspect);

  PyObject * res = PyObject_Call(set_aspect, args, kwargs);
  if (!res) {throw std::runtime_error("Call to set_aspect() failed.");}
  Py_DECREF(set_aspect);

  Py_DECREF(ax);
  Py_DECREF(args);
  Py_DECREF(kwargs);
}

inline void set_aspect_equal()
{
  // expect ratio == "equal". Leaving error handling to matplotlib.
  detail::_interpreter::get();

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyString_FromString("equal"));
  PyObject * kwargs = PyDict_New();

  PyObject * ax = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_gca,
    detail::_interpreter::get().s_python_empty_tuple);
  if (!ax) {throw std::runtime_error("Call to gca() failed.");}
  Py_INCREF(ax);

  PyObject * set_aspect = PyObject_GetAttrString(ax, "set_aspect");
  if (!set_aspect) {throw std::runtime_error("Attribute set_aspect not found.");}
  Py_INCREF(set_aspect);

  PyObject * res = PyObject_Call(set_aspect, args, kwargs);
  if (!res) {throw std::runtime_error("Call to set_aspect() failed.");}
  Py_DECREF(set_aspect);

  Py_DECREF(ax);
  Py_DECREF(args);
  Py_DECREF(kwargs);
}

template<typename T>
void ylim(T left, T right)
{
  detail::_interpreter::get();

  PyObject * list = PyList_New(2);
  PyList_SetItem(list, 0, PyFloat_FromDouble(left));
  PyList_SetItem(list, 1, PyFloat_FromDouble(right));

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, list);

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_ylim, args);
  if (!res) {throw std::runtime_error("Call to ylim() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

template<typename T>
void xlim(T left, T right)
{
  detail::_interpreter::get();

  PyObject * list = PyList_New(2);
  PyList_SetItem(list, 0, PyFloat_FromDouble(left));
  PyList_SetItem(list, 1, PyFloat_FromDouble(right));

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, list);

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_xlim, args);
  if (!res) {throw std::runtime_error("Call to xlim() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

inline std::array<double, 2> xlim()
{
  PyObject * args = PyTuple_New(0);
  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_xlim, args);

  if (!res) {throw std::runtime_error("Call to xlim() failed.");}

  Py_DECREF(res);

  PyObject * left = PyTuple_GetItem(res, 0);
  PyObject * right = PyTuple_GetItem(res, 1);
  return {PyFloat_AsDouble(left), PyFloat_AsDouble(right)};
}

inline std::array<double, 2> ylim()
{
  PyObject * args = PyTuple_New(0);
  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_ylim, args);

  if (!res) {throw std::runtime_error("Call to ylim() failed.");}

  Py_DECREF(res);

  PyObject * left = PyTuple_GetItem(res, 0);
  PyObject * right = PyTuple_GetItem(res, 1);
  return {PyFloat_AsDouble(left), PyFloat_AsDouble(right)};
}

template<typename T>
inline void xticks(
  const T & ticks,
  const std::vector<std::string> & labels = {},
  const detail::varargin_t & keywords = {})
{
  assert(labels.size() == 0 || ticks.size() == labels.size());

  detail::_interpreter::get();

  // using numpy array
  PyObject * ticksarray = detail::get_array(ticks);

  PyObject * args;
  if (labels.size() == 0) {
    // construct positional args
    args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, ticksarray);
  } else {
    // make tuple of tick labels
    PyObject * labelstuple = PyTuple_New(labels.size());
    for (size_t i = 0; i < labels.size(); i++) {
      PyTuple_SetItem(labelstuple, i, PyUnicode_FromString(labels[i].c_str()));
    }

    // construct positional args
    args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, ticksarray);
    PyTuple_SetItem(args, 1, labelstuple);
  }

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_xticks, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (!res) {throw std::runtime_error("Call to xticks() failed");}

  Py_DECREF(res);
}

template<typename T>
inline void xticks(const T & ticks, const detail::varargin_t & keywords)
{
  xticks(ticks, {}, keywords);
}

template<typename T>
inline void yticks(
  const T & ticks,
  const std::vector<std::string> & labels = {},
  const detail::varargin_t & keywords = {})
{
  assert(labels.size() == 0 || ticks.size() == labels.size());

  detail::_interpreter::get();

  // using numpy array
  PyObject * ticksarray = detail::get_array(ticks);

  PyObject * args;
  if (labels.size() == 0) {
    // construct positional args
    args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, ticksarray);
  } else {
    // make tuple of tick labels
    PyObject * labelstuple = PyTuple_New(labels.size());
    for (size_t i = 0; i < labels.size(); i++) {
      PyTuple_SetItem(labelstuple, i, PyUnicode_FromString(labels[i].c_str()));
    }

    // construct positional args
    args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, ticksarray);
    PyTuple_SetItem(args, 1, labelstuple);
  }

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_yticks, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (!res) {throw std::runtime_error("Call to yticks() failed");}

  Py_DECREF(res);
}

template<typename T>
inline void yticks(const T & ticks, const detail::varargin_t & keywords)
{
  yticks(ticks, {}, keywords);
}

template<typename T>
inline void margins(T margin)
{
  // construct positional args
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(margin));

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_margins, args);
  if (!res) {throw std::runtime_error("Call to margins() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

template<typename T>
inline void margins(T margin_x, T margin_y)
{
  // construct positional args
  PyObject * args = PyTuple_New(2);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(margin_x));
  PyTuple_SetItem(args, 1, PyFloat_FromDouble(margin_y));

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_margins, args);
  if (!res) {throw std::runtime_error("Call to margins() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

inline void tick_params(const detail::varargin_t & keywords, const std::string axis = "both")
{
  detail::_interpreter::get();

  // construct positional args
  PyObject * args;
  args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyString_FromString(axis.c_str()));

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_tick_params, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (!res) {throw std::runtime_error("Call to tick_params() failed");}

  Py_DECREF(res);
}

inline void subplot(long nrows, long ncols, long plot_number)
{
  detail::_interpreter::get();

  // construct positional args
  PyObject * args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(static_cast<double>(nrows)));
  PyTuple_SetItem(args, 1, PyFloat_FromDouble(static_cast<double>(ncols)));
  PyTuple_SetItem(args, 2, PyFloat_FromDouble(static_cast<double>(plot_number)));

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_subplot, args);
  if (!res) {throw std::runtime_error("Call to subplot() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

inline void subplot2grid(
  long nrows, long ncols, long rowid = 0, long colid = 0, long rowspan = 1, long colspan = 1)
{
  detail::_interpreter::get();

  PyObject * shape = PyTuple_New(2);
  PyTuple_SetItem(shape, 0, PyLong_FromLong(nrows));
  PyTuple_SetItem(shape, 1, PyLong_FromLong(ncols));

  PyObject * loc = PyTuple_New(2);
  PyTuple_SetItem(loc, 0, PyLong_FromLong(rowid));
  PyTuple_SetItem(loc, 1, PyLong_FromLong(colid));

  PyObject * args = PyTuple_New(4);
  PyTuple_SetItem(args, 0, shape);
  PyTuple_SetItem(args, 1, loc);
  PyTuple_SetItem(args, 2, PyLong_FromLong(rowspan));
  PyTuple_SetItem(args, 3, PyLong_FromLong(colspan));

  PyObject * res =
    PyObject_CallObject(detail::_interpreter::get().s_python_function_subplot2grid, args);
  if (!res) {throw std::runtime_error("Call to subplot2grid() failed.");}

  Py_DECREF(shape);
  Py_DECREF(loc);
  Py_DECREF(args);
  Py_DECREF(res);
}

inline void title(const std::string & titlestr, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * pytitlestr = PyString_FromString(titlestr.c_str());
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, pytitlestr);

  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res = PyObject_Call(detail::_interpreter::get().s_python_function_title, args, kwargs);
  if (!res) {throw std::runtime_error("Call to title() failed.");}

  Py_DECREF(args);
  Py_DECREF(kwargs);
  Py_DECREF(res);
}

inline void suptitle(const std::string & suptitlestr, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * pysuptitlestr = PyString_FromString(suptitlestr.c_str());
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, pysuptitlestr);

  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_suptitle, args, kwargs);
  if (!res) {throw std::runtime_error("Call to suptitle() failed.");}

  Py_DECREF(args);
  Py_DECREF(kwargs);
  Py_DECREF(res);
}

inline void axis(const std::string & axisstr)
{
  detail::_interpreter::get();

  PyObject * str = PyString_FromString(axisstr.c_str());
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, str);

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_axis, args);
  if (!res) {throw std::runtime_error("Call to title() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

inline void axhline(
  double y, double xmin = 0., double xmax = 1., const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  // construct positional args
  PyObject * args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(y));
  PyTuple_SetItem(args, 1, PyFloat_FromDouble(xmin));
  PyTuple_SetItem(args, 2, PyFloat_FromDouble(xmax));

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_axhline, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);

  if (res) {Py_DECREF(res);}
}

inline void axvline(
  double x, double ymin = 0., double ymax = 1., const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  // construct positional args
  PyObject * args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(x));
  PyTuple_SetItem(args, 1, PyFloat_FromDouble(ymin));
  PyTuple_SetItem(args, 2, PyFloat_FromDouble(ymax));

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_axvline, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);

  if (res) {Py_DECREF(res);}
}

inline void axvspan(
  double xmin,
  double xmax,
  double ymin = 0.,
  double ymax = 1.,
  const detail::varargin_t & keywords = {})
{
  // construct positional args
  PyObject * args = PyTuple_New(4);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(xmin));
  PyTuple_SetItem(args, 1, PyFloat_FromDouble(xmax));
  PyTuple_SetItem(args, 2, PyFloat_FromDouble(ymin));
  PyTuple_SetItem(args, 3, PyFloat_FromDouble(ymax));

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_axvspan, args, kwargs);
  Py_DECREF(args);
  Py_DECREF(kwargs);

  if (res) {Py_DECREF(res);}
}

inline void xlabel(const std::string & str, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * pystr = PyString_FromString(str.c_str());
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, pystr);

  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_xlabel, args, kwargs);
  if (!res) {throw std::runtime_error("Call to xlabel() failed.");}

  Py_DECREF(args);
  Py_DECREF(kwargs);
  Py_DECREF(res);
}

inline void ylabel(const std::string & str, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * pystr = PyString_FromString(str.c_str());
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, pystr);

  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_ylabel, args, kwargs);
  if (!res) {throw std::runtime_error("Call to ylabel() failed.");}

  Py_DECREF(args);
  Py_DECREF(kwargs);
  Py_DECREF(res);
}

inline void set_zlabel(const std::string & str, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  // Same as with plot_surface: We lazily load the modules here the first time
  // this function is called because I'm not sure that we can assume "matplotlib
  // installed" implies "mpl_toolkits installed" on all platforms, and we don't
  // want to require it for people who don't need 3d plots.
  static PyObject * mpl_toolkitsmod = nullptr, * axis3dmod = nullptr;
  if (!mpl_toolkitsmod) {
    PyObject * mpl_toolkits = PyString_FromString("mpl_toolkits");
    PyObject * axis3d = PyString_FromString("mpl_toolkits.mplot3d");
    if (!mpl_toolkits || !axis3d) {throw std::runtime_error("couldnt create string");}

    mpl_toolkitsmod = PyImport_Import(mpl_toolkits);
    Py_DECREF(mpl_toolkits);
    if (!mpl_toolkitsmod) {throw std::runtime_error("Error loading module mpl_toolkits!");}

    axis3dmod = PyImport_Import(axis3d);
    Py_DECREF(axis3d);
    if (!axis3dmod) {throw std::runtime_error("Error loading module mpl_toolkits.mplot3d!");}
  }

  PyObject * pystr = PyString_FromString(str.c_str());
  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, pystr);

  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * ax = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_gca,
    detail::_interpreter::get().s_python_empty_tuple);
  if (!ax) {throw std::runtime_error("Call to gca() failed.");}
  Py_INCREF(ax);

  PyObject * zlabel = PyObject_GetAttrString(ax, "set_zlabel");
  if (!zlabel) {throw std::runtime_error("Attribute set_zlabel not found.");}
  Py_INCREF(zlabel);

  PyObject * res = PyObject_Call(zlabel, args, kwargs);
  if (!res) {throw std::runtime_error("Call to set_zlabel() failed.");}
  Py_DECREF(zlabel);

  Py_DECREF(ax);
  Py_DECREF(args);
  Py_DECREF(kwargs);
  if (res) {Py_DECREF(res);}
}

inline void grid(bool flag)
{
  detail::_interpreter::get();

  PyObject * pyflag = flag ? Py_True : Py_False;
  Py_INCREF(pyflag);

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, pyflag);

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_grid, args);
  if (!res) {throw std::runtime_error("Call to grid() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

inline void show(const bool block = true)
{
  detail::_interpreter::get();

  PyObject * res;
  if (block) {
    res = PyObject_CallObject(
      detail::_interpreter::get().s_python_function_show,
      detail::_interpreter::get().s_python_empty_tuple);
  } else {
    PyObject * kwargs = PyDict_New();
    PyDict_SetItemString(kwargs, "block", Py_False);
    res = PyObject_Call(
      detail::_interpreter::get().s_python_function_show,
      detail::_interpreter::get().s_python_empty_tuple,
      kwargs);
    Py_DECREF(kwargs);
  }

  if (!res) {throw std::runtime_error("Call to show() failed.");}

  Py_DECREF(res);
}

inline void close()
{
  detail::_interpreter::get();

  PyObject * res = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_close,
    detail::_interpreter::get().s_python_empty_tuple);

  if (!res) {throw std::runtime_error("Call to close() failed.");}

  Py_DECREF(res);
}

inline void xkcd()
{
  detail::_interpreter::get();

  PyObject * res;
  PyObject * kwargs = PyDict_New();

  res = PyObject_Call(
    detail::_interpreter::get().s_python_function_xkcd,
    detail::_interpreter::get().s_python_empty_tuple,
    kwargs);

  Py_DECREF(kwargs);

  if (!res) {throw std::runtime_error("Call to show() failed.");}

  Py_DECREF(res);
}

inline void draw()
{
  detail::_interpreter::get();

  PyObject * res = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_draw,
    detail::_interpreter::get().s_python_empty_tuple);

  if (!res) {throw std::runtime_error("Call to draw() failed.");}

  Py_DECREF(res);
}

template<typename T>
inline void pause(T interval)
{
  detail::_interpreter::get();

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyFloat_FromDouble(interval));

  PyObject * res = PyObject_CallObject(detail::_interpreter::get().s_python_function_pause, args);
  if (!res) {throw std::runtime_error("Call to pause() failed.");}

  Py_DECREF(args);
  Py_DECREF(res);
}

inline void save(const std::string & filename, const int dpi = 0)
{
  detail::_interpreter::get();

  PyObject * pyfilename = PyString_FromString(filename.c_str());

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, pyfilename);

  PyObject * kwargs = PyDict_New();

  if (dpi > 0) {PyDict_SetItemString(kwargs, "dpi", PyLong_FromLong(dpi));}

  PyObject * res = PyObject_Call(detail::_interpreter::get().s_python_function_save, args, kwargs);
  if (!res) {throw std::runtime_error("Call to save() failed.");}

  Py_DECREF(args);
  Py_DECREF(kwargs);
  Py_DECREF(res);
}

inline void rcparams(const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();
  PyObject * args = PyTuple_New(0);
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * update =
    PyObject_GetAttrString(detail::_interpreter::get().s_python_function_rcparams, "update");
  PyObject * res = PyObject_Call(update, args, kwargs);
  if (!res) {throw std::runtime_error("Call to rcParams.update() failed.");}
  Py_DECREF(args);
  Py_DECREF(kwargs);
  Py_DECREF(update);
  Py_DECREF(res);
}

inline void clf()
{
  detail::_interpreter::get();

  PyObject * res = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_clf,
    detail::_interpreter::get().s_python_empty_tuple);

  if (!res) {throw std::runtime_error("Call to clf() failed.");}

  Py_DECREF(res);
}

inline void cla()
{
  detail::_interpreter::get();

  PyObject * res = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_cla,
    detail::_interpreter::get().s_python_empty_tuple);

  if (!res) {throw std::runtime_error("Call to cla() failed.");}

  Py_DECREF(res);
}

inline void ion()
{
  detail::_interpreter::get();

  PyObject * res = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_ion,
    detail::_interpreter::get().s_python_empty_tuple);

  if (!res) {throw std::runtime_error("Call to ion() failed.");}

  Py_DECREF(res);
}

inline std::vector<std::array<double, 2>> ginput(
  const int numClicks = 1, const detail::varargin_t & keywords = {})
{
  detail::_interpreter::get();

  PyObject * args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyLong_FromLong(numClicks));

  // construct keyword args
  PyObject * kwargs = detail::create_kwargs(keywords);

  PyObject * res =
    PyObject_Call(detail::_interpreter::get().s_python_function_ginput, args, kwargs);

  Py_DECREF(kwargs);
  Py_DECREF(args);
  if (!res) {throw std::runtime_error("Call to ginput() failed.");}

  const size_t len = PyList_Size(res);
  std::vector<std::array<double, 2>> out;
  out.reserve(len);
  for (size_t i = 0; i < len; i++) {
    PyObject * current = PyList_GetItem(res, i);
    std::array<double, 2> position;
    position[0] = PyFloat_AsDouble(PyTuple_GetItem(current, 0));
    position[1] = PyFloat_AsDouble(PyTuple_GetItem(current, 1));
    out.push_back(position);
  }
  Py_DECREF(res);

  return out;
}

// Actually, is there any reason not to call this automatically for every plot?
inline void tight_layout()
{
  detail::_interpreter::get();

  PyObject * res = PyObject_CallObject(
    detail::_interpreter::get().s_python_function_tight_layout,
    detail::_interpreter::get().s_python_empty_tuple);

  if (!res) {throw std::runtime_error("Call to tight_layout() failed.");}

  Py_DECREF(res);
}

// Support for variadic plot() and initializer lists:

namespace detail
{

template<typename T>
using is_function = typename std::is_function<std::remove_pointer<std::remove_reference<T>>>::type;

template<bool obj, typename T>
struct is_callable_impl;

template<typename T>
struct is_callable_impl<false, T>
{
  typedef is_function<T> type;
};  // a non-object is callable iff it is a function

template<typename T>
struct is_callable_impl<true, T>
{
  struct Fallback
  {
    void operator()();
  };
  struct Derived : T, Fallback
  {};

  template<typename U, U>
  struct Check;

  template<typename U>
  static std::true_type test(...);  // use a variadic function to make sure (1) it accepts
                                    // everything and (2) its always the worst match

  template<typename U>
  static std::false_type test(Check<void (Fallback::*)(), &U::operator()> *);

public:
  typedef decltype (test<Derived>(nullptr)) type;
  typedef decltype ( & Fallback::operator()) dtype;
  static constexpr bool value = type::value;
};  // an object is callable iff it defines operator()

template<typename T>
struct is_callable
{
  // dispatch to is_callable_impl<true, T> or is_callable_impl<false, T> depending on whether T is
  // of class type or not
  typedef typename is_callable_impl<std::is_class<T>::value, T>::type type;
};

template<typename IsYDataCallable>
struct plot_impl
{};

template<>
struct plot_impl<std::false_type>
{
  template<typename IterableX, typename IterableY>
  bool operator()(const IterableX & x, const IterableY & y, const std::string & format)
  {
    detail::_interpreter::get();

    // 2-phase lookup for distance, begin, end
    using std::begin;
    using std::distance;
    using std::end;

    auto xs = distance(begin(x), end(x));
    auto ys = distance(begin(y), end(y));
    assert(xs == ys && "x and y data must have the same number of elements!");

    PyObject * xlist = PyList_New(xs);
    PyObject * ylist = PyList_New(ys);
    PyObject * pystring = PyString_FromString(format.c_str());

    auto itx = begin(x), ity = begin(y);
    for (size_t i = 0; i < xs; ++i) {
      PyList_SetItem(xlist, i, PyFloat_FromDouble(*itx++));
      PyList_SetItem(ylist, i, PyFloat_FromDouble(*ity++));
    }

    PyObject * plot_args = PyTuple_New(3);
    PyTuple_SetItem(plot_args, 0, xlist);
    PyTuple_SetItem(plot_args, 1, ylist);
    PyTuple_SetItem(plot_args, 2, pystring);

    PyObject * res =
      PyObject_CallObject(detail::_interpreter::get().s_python_function_plot, plot_args);

    Py_DECREF(plot_args);
    if (res) {Py_DECREF(res);}

    return res;
  }
};

template<>
struct plot_impl<std::true_type>
{
  template<typename Iterable, typename Callable>
  bool operator()(const Iterable & ticks, const Callable & f, const std::string & format)
  {
    if (begin(ticks) == end(ticks)) {return true;}

    // We could use additional meta-programming to deduce the correct element type of y,
    // but all values have to be convertible to double anyways
    std::vector<double> y;
    for (auto x : ticks) {y.push_back(f(x));}
    return plot_impl<std::false_type>()(ticks, y, format);
  }
};

}  // end namespace detail

// recursion stop for the above
template<typename ... Args>
bool plot()
{
  return true;
}

template<typename A, typename B, typename ... Args>
bool plot(const A & a, const B & b, const std::string & format, Args... args)
{
  return detail::plot_impl<typename detail::is_callable<B>::type>()(a, b, format) && plot(args ...);
}

/*
 * This group of plot() functions is needed to support initializer lists, i.e. calling
 *    plot( {1,2,3,4} )
 */
inline bool plot(
  const std::vector<double> & x, const std::vector<double> & y, const std::string & format = "")
{
  return plot(x, y, format);
}

inline bool plot(const std::vector<double> & y, const std::string & format = "")
{
  return plot(y, format);
}

inline bool plot(
  const std::vector<double> & x, const std::vector<double> & y, const detail::varargin_t & keywords)
{
  return plot(x, y, keywords);
}

/*
 * This class allows dynamic plots, ie changing the plotted data without clearing and re-plotting
 */
class Plot
{
public:
  // default initialization with plot label, some data and format
  template<typename T1, typename T2>
  Plot(const std::string & name, const T1 & x, const T2 & y, const std::string & format = "")
  {
    detail::_interpreter::get();

    assert(x.size() == y.size());

    PyObject * kwargs = PyDict_New();
    if (name != "") {PyDict_SetItemString(kwargs, "label", PyString_FromString(name.c_str()));}

    PyObject * xarray = detail::get_array(x);
    PyObject * yarray = detail::get_array(y);

    PyObject * pystring = PyString_FromString(format.c_str());

    PyObject * plot_args = PyTuple_New(3);
    PyTuple_SetItem(plot_args, 0, xarray);
    PyTuple_SetItem(plot_args, 1, yarray);
    PyTuple_SetItem(plot_args, 2, pystring);

    PyObject * res =
      PyObject_Call(detail::_interpreter::get().s_python_function_plot, plot_args, kwargs);

    Py_DECREF(kwargs);
    Py_DECREF(plot_args);

    if (res) {
      line = PyList_GetItem(res, 0);

      if (line) {
        set_data_fct = PyObject_GetAttrString(line, "set_data");
      } else {
        Py_DECREF(line);
      }
      Py_DECREF(res);
    }
  }

  // shorter initialization with name or format only
  // basically calls line, = plot([], [])
  Plot(const std::string & name = "", const std::string & format = "")
  : Plot(name, std::vector<double>(), std::vector<double>(), format)
  {}

  template<typename T1, typename T2>
  bool update(const T1 & x, const T2 & y)
  {
    assert(x.size() == y.size());
    if (set_data_fct) {
      PyObject * xarray = detail::get_array(x);
      PyObject * yarray = detail::get_array(y);

      PyObject * plot_args = PyTuple_New(2);
      PyTuple_SetItem(plot_args, 0, xarray);
      PyTuple_SetItem(plot_args, 1, yarray);

      PyObject * res = PyObject_CallObject(set_data_fct, plot_args);
      if (res) {Py_DECREF(res);}
      return res;
    }
    return false;
  }

  // clears the plot but keep it available
  bool clear() {return update(std::vector<double>(), std::vector<double>());}

  // definitely remove this line
  void remove()
  {
    if (line) {
      auto remove_fct = PyObject_GetAttrString(line, "remove");
      PyObject * args = PyTuple_New(0);
      PyObject * res = PyObject_CallObject(remove_fct, args);
      if (res) {Py_DECREF(res);}
    }
    decref();
  }

  ~Plot() {decref();}

private:
  void decref()
  {
    if (line) {Py_DECREF(line);}
    if (set_data_fct) {Py_DECREF(set_data_fct);}
  }

  PyObject * line = nullptr;
  PyObject * set_data_fct = nullptr;
};

}  // end namespace matplotlibcpp
