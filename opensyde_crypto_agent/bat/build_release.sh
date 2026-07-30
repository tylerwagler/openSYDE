#!/bin/bash

# build a cmake configuration, compile project and install build target

# exit with error code if a command fails
set -e
# cause a pipeline to return the exit status of the last command in the pipe that failed
set -o pipefail

# Due to the static linking of glibc, the linker emits a warning that is known and can be ignored.
# See toolchain_ubuntu.cmake for details.
# Filtering is quite tricky, so we use a small awk script to filter the known warning and keep all other diagnostics.
# Each error is reported with two lines, e.g.:
# /usr/bin/ld: /usr/lib/x86_64-linux-gnu/libcrypto.a(libcrypto-lib-dso_dlfcn.o): in function `dlfcn_globallookup':
# (.text+0x1f): warning: Using 'dlopen' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
filter_known_linker_warnings() {
	awk '
		{
			c_Line = $0

            # Only if there was a previous (pending) error referring to "ld":
			if (c_Pending != "") {
				# Keep one ld context line buffered until we know whether the next line is the known glibc warning.
				if (c_Line ~ /warning: Using '\''.*'\'' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking/) {
					c_Pending = ""
					next
				}
				# Matched pair: drop both the buffered context line and the warning line.
				print c_Pending
				c_Pending = ""
			}

			# Buffer only ld "in function" context lines for the known static archives.
			if ((c_Line ~ /^\/usr\/bin\/ld: .*: in function `[^`]+'\'':$/) &&
					((c_Line ~ /libcrypto\.a\(/) || (c_Line ~ /libopensyde_core\.a\(/))) {
				c_Pending = c_Line
				next
			}

			# Pass through all unrelated diagnostics unchanged.
			print c_Line
		}
		END {
			# Emit buffered line at EOF if no known warning followed it.
			if (c_Pending != "") {
				print c_Pending
			}
		}
	'
}

echo "create build directory 'temp_linux'"
rm -rf ../temp_linux
mkdir ../temp_linux
cd ../temp_linux

# this step is always needed
echo "run cmake with toolchain file"

cmake ../pjt -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_ubuntu.cmake 

echo "build and install"
cmake --build . --target all -- -j4 2> >(filter_known_linker_warnings >&2)
cmake --build . --target install

#return
cd ../bat
