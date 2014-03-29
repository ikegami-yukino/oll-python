oll-python
==========
.. image:: https://badge.fury.io/py/oll.png
    :target: http://badge.fury.io/py/oll
.. image:: https://travis-ci.org/ikegami-yukino/oll-python.svg?branch=master
    :target: https://travis-ci.org/ikegami-yukino/oll-python


This is a Python binding of the OLL library for machine learning.

Currently, OLL 0.03 supports following algorithms:

- Perceptron
- Averaged Perceptron
- Passive Agressive (PA, PA-I, PA-II)
- ALMA (modified slightly from original)
- Confidence Weighted Linear-Classification.

For details of oll, see: http://code.google.com/p/oll

Installation
------------

::

 $ pip install oll

OLL library is bundled, so you don't need to install it separately.

Usage
-----

::

 >>> import oll
 >>> o = oll.oll("CW")
 >>> o.add({0: 1.0, 1: 2.0, 2: -1.0}, 1)  # train
 >>> o.classify({0:1.0, 1:1.0})  # predict
 >>> o.save('oll.model')
 >>> o.load('oll.model')


Note
----
- This module requires C++ compiler to build.
- oll.cpp & oll.hpp : Copyright (c) 2011, Daisuke Okanohara
- oll_swig_wrap.cxx is generated based on 'oll_swig.i' in oll-ruby (https://github.com/syou6162/oll-ruby)

License
-------
New BSD License.

