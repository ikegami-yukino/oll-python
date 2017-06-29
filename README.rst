oll-python
==========

|travis| |coveralls| |version| |license|

This is a Python binding of the OLL library for machine learning.

Currently, OLL 0.03 supports following binary classification algorithms:

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

.. code:: python

 import oll
 # You can choose algorithms in
 # "P" -> Perceptron,
 # "AP" -> Averaged Perceptron,
 # "PA" -> Passive Agressive,
 # "PA1" -> Passive Agressive-I,
 # "PA2" -> Passive Agressive-II,
 # "PAK" -> Kernelized Passive Agressive,
 # "CW" -> Confidence Weighted Linear-Classification,
 # "AL" -> ALMA
 o = oll.oll("CW", C=1.0, bias=0.0)
 o.add({0: 1.0, 1: 2.0, 2: -1.0}, 1)  # train
 o.classify({0:1.0, 1:1.0})  # predict
 o.save('oll.model')
 o.load('oll.model')

 # scikit-learn like fit/predict interface
 import numpy as np
 array = np.array([[1, 2, -1], [0, 0, 1]])
 o.fit(array, [1, -1])
 o.predict(np.array([[1, 2, -1], [0, 0, 1]]))
 # => [1, -1]
 from scipy.sparse import csr_matrix
 matrix = csr_matrix([[1, 2, -1], [0, 0, 1]])
 o.fit(matrix, [1, -1])
 o.predict(matrix)
 # => [1, -1]

 # Multi label classification
 import time
 import oll
 from sklearn.multiclass import OutputCodeClassifier
 from sklearn import datasets, cross_validation, metrics


 dataset = datasets.load_digits()
 ALGORITHMS = ("P", "AP", "PA", "PA1", "PA2", "PAK", "CW", "AL")
 for algorithm in ALGORITHMS:
     print(algorithm)
     occ_predicts = []
     expected = []
     start = time.time()
     for (train_idx, test_idx) in cross_validation.StratifiedKFold(dataset.target,
                                                                   n_folds=10, shuffle=True):
         clf = OutputCodeClassifier(oll.oll(algorithm))
         clf.fit(dataset.data[train_idx], dataset.target[train_idx])
         occ_predicts += list(clf.predict(dataset.data[test_idx]))
         expected += list(dataset.target[test_idx])
     print('Elapsed time: %s' % (time.time() - start))
     print('Accuracy', metrics.accuracy_score(expected, occ_predicts))
 # => P
 # => Elapsed time: 109.82188701629639
 # => Accuracy 0.770172509738
 # => AP
 # => Elapsed time: 111.42936396598816
 # => Accuracy 0.760155815248
 # => PA
 # => Elapsed time: 110.95964503288269
 # => Accuracy 0.74735670562
 # => PA1
 # => Elapsed time: 111.39844799041748
 # => Accuracy 0.806343906511
 # => PA2
 # => Elapsed time: 115.12716913223267
 # => Accuracy 0.766277128548
 # => PAK
 # => Elapsed time: 119.53838682174683
 # => Accuracy 0.77796327212
 # => CW
 # => Elapsed time: 121.20785689353943
 # => Accuracy 0.771285475793
 # => AL
 # => Elapsed time: 116.52497220039368
 # => Accuracy 0.785754034502

Note
----
- This module requires C++ compiler to build.
- oll.cpp & oll.hpp : Copyright (c) 2011, Daisuke Okanohara
- oll_swig_wrap.cxx is generated based on 'oll_swig.i' in oll-ruby (https://github.com/syou6162/oll-ruby)

License
-------
New BSD License.

.. |travis| image:: https://travis-ci.org/ikegami-yukino/oll-python.svg?branch=master
    :target: https://travis-ci.org/ikegami-yukino/oll-python
    :alt: travis-ci.org
.. |coveralls| image:: https://coveralls.io/repos/ikegami-yukino/oll-python/badge.png
    :target: https://coveralls.io/r/ikegami-yukino/oll-python
    :alt: coveralls.io

.. |version| image:: https://img.shields.io/pypi/v/oll.svg
    :target: http://pypi.python.org/pypi/oll/
    :alt: latest version

.. |license| image:: https://img.shields.io/pypi/l/oll.svg
    :target: http://pypi.python.org/pypi/oll/
    :alt: license
