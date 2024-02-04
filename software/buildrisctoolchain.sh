# NOTE: for MacOS this would be done via brew and replace all of the following:
# brew tap riscv-software-src/riscv
# brew install riscv-tools

# On Ubuntu, run the following to grab the riscv gnu toolchain into a drive with sufficient space:
# (at this time the latest version of g++ was 12.2.0)

git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain

# Install the prerequisites

sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build

# Optional - If you have something left from a prior session, run:

# sudo make clean

# Next step is to configure for the variants of the RISC-V chip you're going to use

./configure --prefix=/opt/riscv --with-multilib-generator="rv32i_zicsr_zifencei-ilp32--;rv32ia_zicsr_zifencei-ilp32--;rv32iac_zicsr_zifencei-ilp32--;rv32ic_zicsr_zifencei-ilp32--;rv32im_zicsr_zifencei-ilp32--;rv32ima_zicsr_zifencei-ilp32--;rv32imac_zicsr_zifencei-ilp32--;rv32imc_zicsr_zifencei-ilp32--;rv32imfc_zicsr_zifencei-ilp32f--;rv32imf_zicsr_zifencei-ilp32f--;rv32imaf_zicsr_zifencei-ilp32f--;rv32imacf_zicsr_zifencei-ilp32f--;"

# And finally, build the embedded toolchain

sudo make newlib

# Optionally, one can also build the linux toolchain:

# sudo make linux

# When done, add /opt/riscv/bin to PATH

export PATH=/opt/riscv/bin:$PATH

# To make path permanent, use the following commands:
#
# cd ~
# nano .bashrc
#
# and add this to the end of the .bashrc file:
#
# export PATH=/opt/riscv/bin:$PATH
#
# Save and quit (ctrl+x, y)
# Re-login for this setup to take effect.
#
# At this point you should be able to compile risc-v programs using riscv64-unknown-elf-g++ and others.
