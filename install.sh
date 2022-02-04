#!/usr/bin/env bash
set -eou pipefail
(cd lib/vendor/abduco/. && make clean)
(cd lib/vendor/abduco/. && make install)
which abduco-sb
