riscv32-unknown-linux-gnu-gcc myoutput.s sylib-riscv-linux.a
qemu-riscv32.sh a.out > ans.log
echo -e "\n$?" >> ans.log