#!/bin/bash

#rm -r uj
mkdir uj
mkdir uj/results

find results -name '*.sca' -exec ./update_sca_file.sh '{}' 'uj/{}' ';'