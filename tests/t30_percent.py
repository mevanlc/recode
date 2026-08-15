# -*- coding: utf-8 -*-
import common
from common import Recode, outer, setup_module, teardown_module


sample = b"AZaz09-._~ /+?&=\xff"


def test_url_encode():
    common.request('data..URL')
    common.validate(sample, b'AZaz09-._~+%2F%2B%3F%26%3D%FF')


def test_url_decode():
    common.request('URL..data')
    common.validate(b'a+b%2Bc%20d%ff', b'a b+c d\xff')


def test_uri_encode():
    common.request('data..URI')
    common.validate(sample, b'AZaz09-._~%20%2F%2B%3F%26%3D%FF')


def test_uri_decode_preserves_plus():
    common.request('URI..data')
    common.validate(b'a+b%2Bc%20d%ff', b'a+b+c d\xff')


def test_round_trips_all_bytes():
    sample = bytes(range(1, 256))
    for surface in ('URL', 'URI'):
        common.request('data..%s' % surface)
        common.validate_back(sample)


def test_rejects_malformed_percent_escapes():
    for surface in (b'URL', b'URI'):
        for sample in (b'%', b'%A', b'%G0', b'%0G'):
            request = Recode.Request(outer)
            request.scan(surface + b'..data')
            task = Recode.Task(request)
            task.set_input(sample)
            task.perform()
            assert task.get_error() == Recode.INVALID_INPUT
