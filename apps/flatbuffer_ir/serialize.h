#pragma once

#include "Halide.h"
#include "halide_ir_generated.h"

using namespace Halide;

using FBExpr = flatbuffers::Offset<FlatBufferIR::Expr>;

namespace Halide::FlatBufferIR {

uint8_t* to_flatbuffer_ir(Halide::Expr &e);

Halide::Expr to_halide_ir(uint8_t *buffer_pointer);

} // namspace Halide::FlatBufferIR
