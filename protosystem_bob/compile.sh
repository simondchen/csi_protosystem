#!/bin/bash
current_dir=`pwd`
src_dir=("src/injection" "src/netlink")
for dir in ${src_dir[@]}
do
	cd $current_dir
	cd $dir
	echo "$(pwd)"
	make clean
	make all
	execs=(`find ./ -executable -type f`)
	echo ${execs[@]}
	cp ${execs[@]} $current_dir"/bin/"
	make clean
done
