# source this file: lets plain `root` auto-load libEvent and find Event.h
_here="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
export ROOT_INCLUDE_PATH="$_here/include${ROOT_INCLUDE_PATH:+:$ROOT_INCLUDE_PATH}"
export DYLD_LIBRARY_PATH="$_here/build${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"  # macOS
export LD_LIBRARY_PATH="$_here/build${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"        # Linux
unset _here
