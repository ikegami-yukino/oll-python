# -*- coding: utf-8 -*-
from nose.tools import ok_, eq_, raises
import oll
import os


class Test_oll(object):

    def __init__(self):
        self.oll = oll.oll('PA1')
        self.oll.add({0: 1.0, 1: 2.0, 2: -1.0}, 1)
        self.oll.add({0: -0.5, 1: 1.0, 2: -0.5}, -1)

    def test__init__(self):
        def _validate_methods(methods):
            for method in methods:
                oll.oll(method)
        methods = ('P','AP','PA','PA1','PA2','PAK','CW','AL')
        eq_(None, _validate_methods(methods))

    @raises(KeyError)
    def test_invalid__init__(self):
        def _validate_methods(methods):
            for method in methods:
                oll.oll(method)
        invalid_methods = ('AWP','PY', 0)
        _validate_methods(invalid_methods)

    @raises(NotImplementedError)
    def test_add(self):
        self.oll.add({1.5: 1.0}, 1)
        self.oll.add({-1.5: 1.0}, -1)
        self.oll.add({10: 1.0}, 3)
        self.oll.add({10: 1.0}, 0)
        self.oll.add({10: 1.0}, -3)

    def test_classify(self):
        eq_(0.171429, round(self.oll.classify({0:1.0, 1:1.0}), 6))

    def test_save_and_load(self):
        try:
            filename = "tmp.model"
            self.oll.save(filename)
            ok_(os.path.exists(filename))
            self.oll = oll.oll('PA1')
            self.oll.load(filename)
            self.test_classify()
        finally:
            os.remove(filename)

    def test_setC(self):
        self.oll.setC(0.14)

    def test_setBias(self):
        self.oll.setBias(0.14)
