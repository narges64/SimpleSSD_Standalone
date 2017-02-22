# Makefile for new-ssdsim-pam/proto-02

EXEC = new-ssdsim-pam

OBJ_FILES = \
	mem/GlobalConfig.o mem/ConfigReader.o \
	mem/Simulator.o \
	mem/Latency.o mem/LatencyMLC.o mem/LatencyTLC.o mem/LatencySLC.o\
	mem/hil.o \
	mem/PAM2_TimeSlot.o\
	mem/PAMStatistics.o \
	mem/PAM2.o \
	mem/ssd_sim.o\

DEPS = mem/SimpleSSD_types.h mem/SimpleSSD.h mem/ftl_defs.hh

OBJ_DIR = obj

OBJS = $(patsubst %.o,$(OBJ_DIR)/%.o,$(OBJ_FILES))

BASE_OBJ_FILES = base/types.o
BASE_OBJS = $(patsubst %.o,$(OBJ_DIR)/%.o,$(BASE_OBJ_FILES))

FTL_SRC_FILES = \
        mem/ftl_block.o mem/ftl_commandqueue.o \
	mem/ftl_hybridmapping.o mem/ftl_mappingtable.o \
	mem/ftl_request.o \
	mem/ftl.o \
	mem/ftl_statistics.o

FTL_OBJS = $(patsubst %.o,$(OBJ_DIR)/%.o,$(FTL_SRC_FILES))

CC = c++
CFLAGS = -I. -g -std=c++11

$(EXEC): $(OBJS) $(FTL_OBJS) $(BASE_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJ_DIR)/base/%.o: base/%.cc base/%.hh
	@mkdir -p $(OBJ_DIR)/base
	$(CC) -c -o $@ $< $(CFLAGS)
	
$(OBJ_DIR)/mem/%.o: mem/%.cc mem/%.hh
	@mkdir -p $(OBJ_DIR)/mem
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ_DIR)/mem/%.o: mem/%.cpp mem/%.h $(DEPS)
	@mkdir -p $(OBJ_DIR)/mem
	$(CC) -c -o $@ $< $(CFLAGS)


debug: CFLAGS += -DDEBUG -g
debug: $(EXEC)

clean:
	rm -rf $(OBJ_DIR) *.bak $(EXEC) *~
