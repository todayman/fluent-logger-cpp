import unittest
from tests import mockserver
import logging
import msgpack
import subprocess

class TestLogger(unittest.TestCase):
    def setUp(self):
        super(TestLogger, self).setUp()
        for port in range(10000, 20000):
            try:
                self._server = mockserver.MockRecvServer(port)
                self._port = port
                break
            except IOError as e:
                pass

    def get_data(self):
        return self._server.get_recieved()

    def test_simple(self):
        print('directory:')
        subprocess.call(['pwd'])
        print('results')
        subprocess.call(['./fluent_test', str(self._port)])

        data = self.get_data()
        eq = self.assertEqual
        eq(1, len(data))
        eq(3, len(data[0]))
        eq('fluent.test', data[0][0])
        eq('userA', data[0][2]['from'])
        eq('userB', data[0][2]['to'])
        self.assert_(data[0][1])
        self.assert_(isinstance(data[0][1], int))

