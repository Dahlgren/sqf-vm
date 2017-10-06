#!/usr/bin/env sh
find . -iname *.h -o -iname *.c | sort | xargs -I {} bash -c '
  diff <(clang-format {}) {} > /dev/null;
  retVal=$?;
  if [ "$retVal" -ne 0 ]; then 
    echo {} has formatting issues
  fi
  exit $retVal
'
