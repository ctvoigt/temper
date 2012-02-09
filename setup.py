#!/usr/bin/env python
"""
setup.py file for pypcsensor
usage: python setup.py build_ext --inplace
"""
import os

os.path.dirname(os.path.realpath(__file__))

from subprocess import call
#try:
#call(["make","clean"])
#call(["make"])
call(["rm", "pcsensor_wrap.c pypcsensor.py"])
call(["swig", "-python pcsensor.i"])
#except :
#    print('Please, install swig, in ubuntu use: sudo apt-get install swig')

from distutils.core import setup, Extension

pypcsensor_module = Extension('_pypcsensor',
    sources=['pcsensor_wrap.c', 'pcsensor.c'], extra_compile_args=['-lusb']
)
setup(name='pypcsensor',
    version='0.1',
    author="Fabio C. Barrionuevo da Luz",
    description="""Simple python wrap from pcsensor""",
    ext_modules=[pypcsensor_module, ],
    py_modules=["pypcsensor"],
)
