#!/usr/bin/env python3
"""Reject silent PulseAudio loopback recordings of the packaged audio probe."""
from array import array
from pathlib import Path
import sys


def validate(data):
    if not data or len(data) % 2 or len(data) > 32*1024*1024:
        raise ValueError('Invalid or oversized signed 16-bit PCM capture')
    samples = array('h')
    samples.frombytes(data)
    if sys.byteorder != 'little':
        samples.byteswap()
    peak = max(abs(value) for value in samples)
    audible = sum(abs(value) > 64 for value in samples)
    if peak < 256 or audible < 100:
        raise ValueError(f'Audio output is silent or incomplete: samples={len(samples)}, peak={peak}, audible={audible}')
    return {'samples': len(samples), 'peak': peak, 'audible': audible}


if __name__ == '__main__':
    print('PASS: real PulseAudio monitor PCM', validate(Path(sys.argv[1]).read_bytes()))
