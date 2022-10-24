#include "Halide.h"
#include "flatbuffers/flatbuffers.h"
#include "halide_ir_generated.h"
#include "serialize.h"

using namespace Halide;

namespace HL = Halide::Internal;
namespace FB = Halide::FlatBufferIR;

int test_expr(Expr e) {
    std::cout << "Converting Halide IR -> FlatBuffer IR" << std::endl;
    auto e_flat = FlatBufferIR::to_flatbuffer_ir(e);
    std::cout << "Converting FlatBuffer IR -> Halide IR" << std::endl;
    auto e_new = FlatBufferIR::to_halide_ir(e_flat);
    std::cout << "Done!" << std::endl;
    HL::IRPrinter irp(std::cerr);
    irp.print(e);
    std::cout << std::endl;
    irp.print(e_new);
    std::cout << std::endl;
    std::cout << "Checking results\n" << std::endl;
    if (!equal(e, e_new)) {
        return 1;
    }
    return 0;
}

int test() {
    Expr a = 1;
    Expr b = 2;
    Expr c = 3; // Make Variables work here
    Expr a_bool = a == b;
    Expr b_bool = a == c;
    int16_t *image_buf = (int16_t *) malloc(8 * 16 * 32 * sizeof(int16_t));
    halide_dimension_t dims[3] = {{0, 8, 1}, {0, 16, 8}, {0, 32, 8 * 16}};
    auto buffer = Buffer(image_buf, 3, dims, "image_buf");
    HL::Parameter buffer_param = HL::Parameter(Int(16), true, 3, "input");
    buffer_param.set_buffer(buffer);
    std::vector<Expr> v = {
        a + b + c,
        a - b,
        a * b,
        a / b,
        a % b,
        min(a, b),
        max(a, b),
        a == b,
        a != b,
        a < b,
        a <= b,
        a > b,
        a >= b,
        a_bool && b_bool,
        a_bool || b_bool,
        Expr("Testing FlatBuffers"),
        cast(UInt(8), a),
        reinterpret(UInt(8, 4), a),
        not(a_bool),
        select(a_bool, a, b),
        // LoadReferent_NONE
        HL::Load::make(Int(16), "image", a + b, Buffer<>(), HL::Parameter(), HL::const_true(), HL::ModulusRemainder(3, 2)),
        // LoadReferent_ScalarParameter
        HL::Load::make(Int(16), "image", a + b, Buffer<>(), HL::Parameter(Int(16, 64), false, 0, "input"), HL::const_true(), HL::ModulusRemainder(32, 16)),
        // LoadReferent_BufferParameter
        HL::Load::make(Int(16), "image", c, Buffer<>(), buffer_param, HL::const_true(), HL::ModulusRemainder(32, 16)),
        // LoadReferent_BufferDescriptor
        HL::Load::make(Float(16), "image", a, Buffer<float>::make_scalar(), HL::Parameter(), HL::const_true(), HL::ModulusRemainder()),
        // LoadReferent_BufferConstant
        HL::Load::make(Float(16), "image", a, Buffer<int16_t>::make_scalar(image_buf), HL::Parameter(), HL::const_true(), HL::ModulusRemainder()),
        // LoadReferent_BufferConstant
        HL::Load::make(Float(16), "image", a, buffer, HL::Parameter(Int(16, 64), false, 0, "input"), HL::const_true(), HL::ModulusRemainder()),
    };
    int res = 0;
    for (auto e : v) {
        res += test_expr(e);
    }
    return res;
}

int main() {
    if (test()) {
        std::cout << "Test failed!\n" << std::endl;
        return 1;
    }
    std::cout << "Test passed!\n" << std::endl;
    return 0;
}
