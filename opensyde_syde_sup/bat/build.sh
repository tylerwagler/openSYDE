#!/bin/bash
#script usage: 
# * ./build.sh -> build a cmake configuration, compile project and install build target
# * ./build.sh -l -> "-l(int_config)" -> only build cmake configuration (needed for pc_lint_plus scripts)

build_only_lint_config=false

# exit with error code if a command fails
set -e
# cause a pipeline to return the exit status of the last command in the pipe that failed
set -o pipefail

while getopts ':l' flag; do
 case "$flag" in
    l)
        echo "Only build cmake configuration for pc_lint usage"
        build_only_lint_config=true
        ;;
   \?)
        echo "script usage:"
        echo "./build.sh -> compile project"
        echo "./build.sh -l -> -l(int_config) -> only build cmake configuration (for pc_lint usage)"
        exit 1
        ;;
    esac
done


# Due to the static linking of glibc, the linker emits a warning that is known and can be ignored.
# See toolchain_ubuntu.cmake for details.
# Filtering is quite tricky, so we use a small awk script to filter the known warning and keep all other diagnostics.
# Each error is reported with two lines, e.g.:
# /usr/bin/ld: /usr/lib/x86_64-linux-gnu/libcrypto.a(libcrypto-lib-dso_dlfcn.o): in function `dlfcn_globallookup':
# (.text+0x1f): warning: Using 'dlopen' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
filter_known_linker_warnings() {
	stdbuf -oL -eL awk '
		{
			c_Line = $0

            # Only if there was a previous (pending) error referring to "ld":
			if (c_Pending != "") {
				# Keep one ld context line buffered until we know whether the next line is the known glibc warning.
				if (c_Line ~ /warning: Using .* in statically linked applications requires at runtime the shared libraries from the glibc version used for linking/) {
					c_Pending = ""
					next
				}
				# Matched pair: drop both the buffered context line and the warning line.
				print c_Pending
				c_Pending = ""
			}

			# Buffer only ld "in function" context lines for the known static archives.
			if ((c_Line ~ /^\/usr\/bin\/ld: .*: in function .*:$/) &&
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


echo "create build directory 'temp'"
rm -rf ../temp
mkdir ../temp
cd ../temp

# this step is always needed
echo "run cmake with toolchain file for 64bit compilation"

cmake ../pjt -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_ubuntu.cmake 

# only build the whole thing if no flag was given
if [ "$build_only_lint_config" = false ] ; then
    echo "build and install"
    cmake --build . --target all -- -j4 2> >(filter_known_linker_warnings >&2)
    cmake --build . --target install
fi

#return
cd ../bat
