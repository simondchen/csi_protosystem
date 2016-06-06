#!/bin/bash
matlab -nodisplay -nojvm -nosplash -nodesktop -r "try, run('./test.m'), catch, exit(1), end, exit(0);"
echo "matlab exit code:$?"
