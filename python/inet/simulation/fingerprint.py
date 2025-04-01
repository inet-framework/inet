"""
This module provides abstractions for fingerprints.
"""

import logging

from inet.common import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

class Fingerprint:
    def __init__(self, fingerprint, ingredients):
        self.fingerprint = fingerprint
        self.ingredients = ingredients

    def __repr__(self):
        return repr(self)

    def __str__(self):
        return self.fingerprint + "/" + self.ingredients

    def __eq__(self, other):
        return other and self.fingerprint == other.fingerprint and self.ingredients == other.ingredients

    def __ne__(self, other):
        return not self.__eq__(other)

    def __composite_values__(self):
        return self.fingerprint, self.ingredients

    @classmethod
    def parse(self, text):
        match = re.match(r"(.*)/(.*)", text)
        fingerprint = match.groups()[0]
        ingredients = match.groups()[1]
        return Fingerprint(fingerprint, ingredients)

class FingerprintTrajectory:
    def __init__(self, simulation_result, ingredients, fingerprints, event_numbers):
        self.simulation_result = simulation_result
        self.ingredients = ingredients
        self.fingerprints = fingerprints
        self.event_numbers = event_numbers

    def get_unique(self):
        i = 0
        unique_fingerprints = []
        event_numbers = []
        while i < len(self.fingerprints):
            fingerprint = self.fingerprints[i]
            j = i
            while (j < len(self.fingerprints)) and (fingerprint == self.fingerprints[j]):
                j = j + 1
            unique_fingerprints.append(fingerprint)
            event_numbers.append(j - 1)
            i = j
        return FingerprintTrajectory(self.simulation_result, self.ingredients, unique_fingerprints, event_numbers)
