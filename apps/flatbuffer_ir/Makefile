include ../support/Makefile.inc

FLATC_DIR?=/local/mnt/workspace/aankit/softwares/flatbuffers/
FLATC=flatc
OPTIMIZE=-O3
# OPTIMIZE=-O0 -g -gdwarf-2

.PHONY: test clean

run: $(BIN)/roundtrip_test
	$(BIN)/roundtrip_test

$(BIN)/halide_ir_generated.h: halide_ir.fbs
	$(FLATC) -o $(BIN) -c $^

$(BIN)/serialize.o: serialize.cpp serialize.h $(BIN)/halide_ir_generated.h
	$(CXX) $(CXXFLAGS) $(OPTIMIZE) -I$(FLATC_DIR)/include -I$(BIN)/$* -c $< -o $@

$(BIN)/FBIRVisitor.o: FBIRVisitor.cpp FBIRVisitor.h $(BIN)/halide_ir_generated.h
	$(CXX) $(CXXFLAGS) $(OPTIMIZE) -I$(BIN)/$* -c $< -o $@

$(BIN)/roundtrip_test.o: roundtrip_test.cpp $(BIN)/halide_ir_generated.h
	$(CXX) $(CXXFLAGS) $(OPTIMIZE) -I$(BIN)/$* -c $< -o $@

$(BIN)/roundtrip_test: $(BIN)/roundtrip_test.o $(BIN)/serialize.o $(BIN)/FBIRVisitor.o
	$(CXX) $(CXXFLAGS) $(OPTIMIZE) -I$(BIN)/$* -Wall -O3 $^ -o $@ $(LIBHALIDE_LDFLAGS) $(HALIDE_SYSTEM_LIBS)

clean:
	rm -rf $(BIN)
