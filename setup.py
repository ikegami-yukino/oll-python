#!/usr/bin/env python
# -*- coding: utf-8 -*-
from distutils.core import setup, Extension


oll_module = Extension('_oll',
                           sources=['lib/oll.cpp','oll_swig_wrap.cxx'],
                           include_dirs=['lib'],
                           language='c++'
                           )

setup (name = 'oll',
       version = '0.0.1',
       author      = "Yukino Ikegami",
       description = """OLL for Python""",
       ext_modules = [oll_module],
       py_modules = ["oll"],
       )


