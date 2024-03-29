# defaults if environment variables not set. allows scritable command line

# N_SPS ?= 64
N_SPS ?= 1

# ifneq ($(ENV_N_SPS),)
# 	N_SPS ?= $(ENV_N_SPS)
# endif

EXTRA_FLAGS ?=
# ifneq ($(ENV_EXTRA_MAKE_FLAGS),)
# 	EXTRA_FLAGS := $(EXTRA_FLAGS) $(ENV_EXTRA_MAKE_FLAGS)
# endif

# check if cache line size is defined
ifeq (,$(findstring -DCACHE_LINE_SIZE=,$(EXTRA_FLAGS)))
	EXTRA_FLAGS:=$(EXTRA_FLAGS) -DCACHE_LINE_SIZE=64 
endif


# Overridable arguments to the simulation command for `make run`.
GEM5_ARGS ?= --remote-gdb-port=0
HB_ARGS ?= --options=""
CONFIG_ARGS ?=  

# unique binary id to differentiate
BINARY_NAME ?= $(BENCHNAME) 

# Find the repository's base directory.
COMMON_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
BASE_DIR := $(COMMON_DIR)/../..

# installed cross compiler gcc for riscv
# RV_CC10 ?=/scratch/pbb59/riscv-rv64gv/bin/riscv64-unknown-linux-gnu-gcc
RV_CC10 ?=gcc

# RV_CC10 ?=/home/yinlee/riscv-gnu-toolchain/bin/riscv64-unknown-linux-gnu-gcc

RV_CC=$(RV_CC10)
# CFLAGS=-D_N_SPS=$(N_SPS) $(EXTRA_FLAGS) -O3 --std=gnu11 -static -I../common/  -lpthread -lm -g


CFLAGS=-D_N_SPS=$(N_SPS) $(EXTRA_FLAGS) -O3 --std=gnu11 -static -I../common/ -T../common/spm.ld -lpthread -lm -g

C_SRCS_NOKERN := $(filter-out $(TRILLIASM_KERNEL), $(wildcard *.c))
C_DEPS_NOKERN := $(C_SRCS_NOKERN:.c=.o)

# Build common libraries, but build them "locally" (in this directory) so
# they get updated according to this benchmark's configuration.
COMMON_SRCS := $(wildcard $(COMMON_DIR)/*.c)
COMMON_OBJS := $(notdir $(COMMON_SRCS:.c=.o))

$(BENCHNAME) : $(TRILLIASM_OBJS) $(C_DEPS_NOKERN) $(COMMON_OBJS)
	$(RV_CC) $(TRILLIASM_OBJS) $(C_DEPS_NOKERN) $(COMMON_OBJS) $(CFLAGS) -o $(BINARY_NAME)

run_simple: $(BENCHNAME)
	$(BASE_DIR)/build/RVSP/gem5.opt \
		$(DEBUG_FLAGS) \
		$(GEM5_ARGS) \
		$(BASE_DIR)/configs/phil/brg_hammerblade.py \
		--cmd=$(BENCHNAME) \
		$(HB_ARGS) \
		
		

run: $(BENCHNAME)
	$(BASE_DIR)/build/RVSP/gem5.opt \
		$(DEBUG_FLAGS) \
		$(GEM5_ARGS) \
		$(BASE_DIR)/configs/phil/brg_hammerblade.py \
		--cmd=$(BENCHNAME) \
		$(HB_ARGS) \
		--num-cpus=$(N_SPS) \
		--vector \
		$(CONFIG_ARGS)		

$(C_DEPS_NOKERN): %.o: %.c
	$(RV_CC) $(CFLAGS) -c $^ -o $@

$(COMMON_OBJS): %.o: $(COMMON_DIR)/%.c
	$(RV_CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf *.o *.s $(BINARY_NAME) m5out
