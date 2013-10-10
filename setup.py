#!/usr/bin/env python
# -*- coding: utf-8 -*-
from distutils.core import setup, Extension


example_module = Extension('_oll',
                           sources=['oll.cpp','oll_swig_wrap.cxx'],
                           )

setup (name = 'oll',
       version = '0.0.1',
       author      = "Yukino Ikegami",
       description = """OLL for Python""",
       ext_modules = [example_module],
       py_modules = ["oll"],
       )
