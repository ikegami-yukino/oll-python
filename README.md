oll-python
==========
This is a Python binding of oll, which is a library for machine learning.

Currently, Oll 0.03 supports following algorithms:

- Perceptron
- Averaged Perceptron
- Passive Agressive (PA, PA-I, PA-II)
- ALMA (modified slightly from original)
- Confidence Weighted Linear-Classification.

For details of oll, see: http://code.google.com/p/oll

## Installation
```
  $ git clone https://github.com/ikegami-yukino/oll-python.git
  $ cd oll-python
  $ sudo python setup.py build install
```
 oll library is bundled, so you don't need to install it separately.

## Usage
```
  import oll
  o = oll.oll("CW")
  o.add({0: 1.0, 1: 2.0, 2: -1.0}, 1)
  o.classify({0:1.0, 1:1.0}), 6)
  o.save('oll.model')
  o.load('oll.model')
```
## Note
- oll.cpp & oll.hpp : Copyright (c) 2011, Daisuke Okanohara
- oll_swig_wrap.cxx is generated based on [oll-ruby](https://github.com/syou6162/oll-ruby)'s oll_swig.i

## License
New BSD License.
