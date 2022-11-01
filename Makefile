BENCHNAME := spgemm
default: $(BENCHNAME)

SIZE_M := 100
SIZE_N := 100
SIZE_K := 16
DENSITY := 0.2

N_ROWS := 4
N_COLS := 4
# Specify number of CPUs. (TODO: rename to NUM_CPUS)
#DEBUG_FLAGS := --debug-flags=Exec,RubyCache,Scratchpad #--debug-file=trace.out.gz --debug-start=308818000
N_SPS ?= $$(($(N_ROWS)*$(N_COLS)))
EXTRA_FLAGS ?= -DNO_VEC  #-DCACHE_WARM #-DPER_CORE_SIMD #-DVECTOR_LEN=4
HB_ARGS := --options="$(N_COLS) $(N_ROWS)"
# CONFIG_ARGS := --num-row=$(N_ROWS) --num-col=$(N_COLS) --doublel2=False --l2-size=8192kB --crossbar_bw=16 --cpu-type=DerivO3CPU
CONFIG_ARGS := -n 1 --llc-max-size=8192kB --cpu-type=TimingSimpleCPU

# GEM5_ARGS ?= --remote-gdb-port=0 --debug-flags=RiscvVector

# Build the kernel with trilliasm.
#TRILLIASM_KERNEL := gemm_kernel.c
#include ../../trillium/trilliasm.mk

# Standard benchmark compilation.
include ../common/common.mk

