#!/usr/env/bin bash

set -uoe pipefail

./../unit_test samples/sample1.mm "[1, 1, 1, 1, 1]" 1> test.out 2> errors.out
./../unit_test samples/m_05_05_crg.mm "[1, 1, 1, 1, 1]" 1>> test.out 2>> errors.out
./../unit_test samples/m_05_05_crs.mm "[1, 1, 1, 1, 1]" 1>> test.out 2>> errors.out
./../unit_test samples/m_05_05_crk.mm "[1, 1, 1, 1, 1]" 1>> test.out 2>> errors.out
./../unit_test samples/west_67_67_crg.mm "[1@67]" 1>> test.out 2>> errors.out
./../unit_test samples/gre_343_343_crg.mm "[1@343]" 1>> test.out 2>> errors.out
./../unit_test samples/gr_900_900_crg.mm "[1@900]" 1>> test.out 2>> errors.out
