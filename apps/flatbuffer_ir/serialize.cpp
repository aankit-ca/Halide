#include "FBIRVisitor.h"
#include "serialize.h"
#include "Halide.h"
#include <unordered_map>

using namespace Halide;

namespace HL = Halide::Internal;
namespace FB = Halide::FlatBufferIR;

namespace {

class HalideToFlatBufferIR : public HL::IRVisitor {
    flatbuffers::FlatBufferBuilder& builder;
    FBExpr expr;

    auto get_type_code(const Type &t) {
        using namespace FB;
        static const std::unordered_map<halide_type_code_t, TypeCode> fb_type_code = {
            {Type::Int, TypeCode_Int},
            {Type::UInt, TypeCode_UInt},
            {Type::Float, TypeCode_Float},
            {Type::Handle, TypeCode_Handle},
            {Type::BFloat, TypeCode_BFloat},
        };
        auto result = fb_type_code.find(t.code());
        assert((result != fb_type_code.end()) && "No matching FlatBuffer Type found");
        return result->second;
    }
    // TODO: CPPHandle
    auto get_type(Type t) {
        using namespace FB;
        auto runtime_type = RuntimeType(get_type_code(t),
                                        t.bits(), t.lanes());
        auto cpp_handle_type_name = builder.CreateString("placeholder");
        auto cpp_handle_type = CreateCPPHandleType(builder, cpp_handle_type_name);
        return CreateIRType(builder, &runtime_type, cpp_handle_type);
    }
    auto get_memory_kind(const MemoryType &t) {
        using namespace FB;
        static const std::unordered_map<MemoryType, MemoryKind> fb_memory_kind = {
            {MemoryType::Auto, MemoryKind_Auto},
            {MemoryType::Heap, MemoryKind_Heap},
            {MemoryType::Stack, MemoryKind_Stack},
            {MemoryType::Register, MemoryKind_Register},
            {MemoryType::GPUShared, MemoryKind_GPUShared},
            {MemoryType::GPUTexture, MemoryKind_GPUTexture},
            {MemoryType::LockedCache, MemoryKind_LockedCache},
            {MemoryType::VTCM, MemoryKind_VTCM},
            {MemoryType::AMXTile, MemoryKind_AMXTile},
        };
        auto result = fb_memory_kind.find(t);
        assert((result != fb_memory_kind.end()) && "No matching FlatBuffer MemoryKind found");
        return result->second;
    }

    auto get_buffer_descriptor(const Buffer<> &buffer) {
        using namespace FB;
        auto buffer_shape_type = get_type(buffer.type());
        // auto buffer_shape_arity = buffer.get()->static_dimensions();
        auto buffer_shape_arity = buffer.dimensions();
        std::vector<Dimension> dimensions;
        for (int i = 0; i < buffer.dimensions(); i++) {
            dimensions.emplace_back(buffer.min(i), buffer.extent(i), buffer.stride(i));
        }
        auto buffer_shape_dims = builder.CreateVectorOfStructs(dimensions);
        auto buffer_shape = CreateBufferShape(builder, buffer_shape_type, buffer_shape_arity, buffer_shape_dims);

        auto buffer_descriptor_name = CreateSymbol(builder, builder.CreateString(buffer.name()));
        auto buffer_descriptor = CreateBufferDescriptor(builder, buffer_descriptor_name, buffer_shape);
        return buffer_descriptor;
    }

    template<typename T, typename Fn_create_ty>
    void visit_const(const T *op, FB::ExprNode node_type, Fn_create_ty fn_create) {
        using namespace FB;
        auto expr_type = get_type(op->type);
        auto expr_node = fn_create(builder, op->value).Union();
        expr = CreateExpr(builder, expr_type, node_type, expr_node);
    }

    template<typename T, typename Fn_create_ty>
    void visit_unary(const T *op, FB::ExprNode node_type, Fn_create_ty fn_create) {
        using namespace FB;
        auto val = convert(op->val);
        auto expr_type = get_type(op->type);
        auto expr_node = fn_create(builder, val).Union();
        expr = CreateExpr(builder, expr_type, node_type, expr_node);
    }

    template<typename T, typename Fn_create_ty>
    void visit_binary(const T *op, FB::ExprNode node_type, Fn_create_ty fn_create) {
        using namespace FB;
        auto a = convert(op->a);
        auto b = convert(op->b);
        auto expr_type = get_type(op->type);
        auto expr_node = fn_create(builder, a, b).Union();
        expr = CreateExpr(builder, expr_type, node_type, expr_node);
    }

    void visit(const HL::IntImm *op) override {
        visit_const(op, FB::ExprNode_IntImm, FB::CreateIntImm);
    }
    void visit(const HL::UIntImm *op) override {
        visit_const(op, FB::ExprNode_UIntImm, FB::CreateUIntImm);
    }
    void visit(const HL::FloatImm *op) override {
        visit_const(op, FB::ExprNode_FloatImm, FB::CreateFloatImm);
    }
    void visit(const HL::StringImm *op) override {
        using namespace FB;
        auto expr_type = get_type(op->type);
        auto expr_node = FB::CreateStringImmDirect(builder, op->value.c_str()).Union();
        expr = CreateExpr(builder, expr_type, ExprNode_StringImm, expr_node);
    }

    void visit(const HL::Cast *op) override {
        using namespace FB;
        auto val = convert(op->value);
        auto expr_type = get_type(op->type);
        auto expr_node = CreateCast(builder, val).Union();
        expr = CreateExpr(builder, expr_type, ExprNode_Cast, expr_node);
    }
    void visit(const HL::Reinterpret *op) override {
        using namespace FB;
        auto val = convert(op->value);
        auto expr_type = get_type(op->type);
        auto expr_node = CreateReinterpret(builder, val).Union();
        expr = CreateExpr(builder, expr_type, FB::ExprNode_Reinterpret, expr_node);
    }

    void visit(const HL::Add *op) override {
        visit_binary(op, FB::ExprNode_Add, FB::CreateAdd);
    }
    void visit(const HL::Sub *op) override {
        visit_binary(op, FB::ExprNode_Sub, FB::CreateSub);
    }
    void visit(const HL::Mul *op) override {
        visit_binary(op, FB::ExprNode_Mul, FB::CreateMul);
    }
    void visit(const HL::Div *op) override {
        visit_binary(op, FB::ExprNode_Div, FB::CreateDiv);
    }
    void visit(const HL::Mod *op) override {
        visit_binary(op, FB::ExprNode_Mod, FB::CreateMod);
    }
    void visit(const HL::Min *op) override {
        visit_binary(op, FB::ExprNode_Min, FB::CreateMin);
    }
    void visit(const HL::Max *op) override {
        visit_binary(op, FB::ExprNode_Max, FB::CreateMax);
    }
    void visit(const HL::EQ *op) override {
        visit_binary(op, FB::ExprNode_EQ, FB::CreateEQ);
    }
    void visit(const HL::NE *op) override {
        visit_binary(op, FB::ExprNode_NE, FB::CreateNE);
    }
    void visit(const HL::LT *op) override {
        visit_binary(op, FB::ExprNode_LT, FB::CreateLT);
    }
    void visit(const HL::LE *op) override {
        visit_binary(op, FB::ExprNode_LE, FB::CreateLE);
    }
    void visit(const HL::GT *op) override {
        visit_binary(op, FB::ExprNode_GT, FB::CreateGT);
    }
    void visit(const HL::GE *op) override {
        visit_binary(op, FB::ExprNode_GE, FB::CreateGE);
    }
    void visit(const HL::And *op) override {
        visit_binary(op, FB::ExprNode_And, FB::CreateAnd);
    }
    void visit(const HL::Or *op) override {
        visit_binary(op, FB::ExprNode_Or, FB::CreateOr);
    }
    void visit(const HL::Not *op) override {
        using namespace FB;
        auto a = convert(op->a);
        auto expr_type = get_type(op->type);
        auto expr_node = CreateNot(builder, a).Union();
        expr = CreateExpr(builder, expr_type, FB::ExprNode_Not, expr_node);
    }
    void visit(const HL::Select *op) override {
        using namespace FB;
        auto condition = convert(op->condition);
        auto true_value = convert(op->true_value);
        auto false_value = convert(op->false_value);
        auto expr_type = get_type(op->type);
        auto expr_node = CreateSelect(builder, condition, true_value, false_value).Union();
        expr = CreateExpr(builder, expr_type, ExprNode_Select, expr_node);
    }

    void visit(const HL::Load *op) override {
        using namespace FB;
        auto predicate = convert(op->predicate);
        auto index = convert(op->index);
        auto expr_type = get_type(op->type);
        auto name = CreateSymbol(builder, builder.CreateString(op->name));
        auto alignment = ModulusRemainder(op->alignment.modulus, op->alignment.remainder);
        auto referent_type = LoadReferent_NONE;
        flatbuffers::Offset<void> referent = 0;

        if (!op->param.defined()) {
            if (op->image.defined()) {
                auto buffer_descriptor = get_buffer_descriptor(op->image);
                if (op->image.data()) {
                    referent_type = LoadReferent_BufferConstant;
                    auto data = builder.CreateVector((unsigned char *) op->image.data(), op->image.size_in_bytes());
                    referent = CreateBufferConstant(builder, buffer_descriptor, data).Union();
                } else {
                    referent_type = LoadReferent_BufferDescriptor;
                    referent = buffer_descriptor.Union();
                }
            }
        } else if (op->param.is_buffer()) {
            // Buffer Parameter
            referent_type = LoadReferent_BufferParameter;
            auto buffer_name = CreateSymbol(builder, builder.CreateString(op->param.name()));
            auto memory_kind = get_memory_kind(op->param.memory_type());
            auto host_alignment = op->param.host_alignment();
            auto buffer_descriptor = get_buffer_descriptor(op->param.buffer());
            std::vector<flatbuffers::Offset<BufferConstraint>> buffer_constraints_vector;
            for (int i = 0; i < op->param.dimensions(); i++) {
                auto min_constraint = convert(op->param.min_constraint(i));
                auto extent_constraint = convert(op->param.extent_constraint(i));
                auto stride_constraint = convert(op->param.stride_constraint(i));
                auto min_constraint_estimate = convert(op->param.min_constraint_estimate(i));
                auto extent_constraint_estimate = convert(op->param.extent_constraint_estimate(i));
                auto buffer_constraint = CreateBufferConstraint(builder, min_constraint, extent_constraint, stride_constraint,
                                                                min_constraint_estimate, extent_constraint_estimate);
                buffer_constraints_vector.push_back(buffer_constraint);
            }
            auto buffer_constraints = builder.CreateVector(buffer_constraints_vector);
            referent = CreateBufferParameter(builder, buffer_name, buffer_descriptor, memory_kind, host_alignment, buffer_constraints).Union();
        } else {
            // Scalar Parameter
            referent_type = LoadReferent_ScalarParameter;
            auto scalar_name = CreateSymbol(builder, builder.CreateString(op->param.name()));
            auto type = get_type(op->param.type());
            auto scalar_default = convert(op->param.default_value());
            auto scalar_min = convert(op->param.min_value());
            auto scalar_max = convert(op->param.max_value());
            auto scalar_estimate = convert(op->param.estimate());
            referent = CreateScalarParameter(builder, scalar_name, type, scalar_default, scalar_min, scalar_max, scalar_estimate).Union();
        }
        auto expr_node = CreateLoad(builder, name, predicate, index, &alignment, referent_type, referent).Union();
        expr = CreateExpr(builder, expr_type, ExprNode_Load, expr_node);
    }
public:
    HalideToFlatBufferIR(flatbuffers::FlatBufferBuilder& builder) : builder(builder) {
    }
    FBExpr convert(Expr e) {
        if (!e.defined()) {
            return FB::CreateExpr(builder);
        }
        e.accept(this);
        return expr;
    }
};

class FlatBufferToHalideIR {
    std::stack<Expr> st;

    auto get_type_code(const FB::IRType* fb_t) {
        using namespace FB;
        static const std::unordered_map<TypeCode, halide_type_code_t> halide_type_code = {
            {TypeCode_Int, Type::Int},
            {TypeCode_UInt, Type::UInt},
            {TypeCode_Float, Type::Float},
            {TypeCode_Handle, Type::Handle},
            {TypeCode_BFloat, Type::BFloat},
        };
        auto fb_type_code = fb_t->runtime_type()->code();
        auto result = halide_type_code.find(fb_type_code);
        assert((result != halide_type_code.end()) && "No matching Halide Type found");
        return result->second;
    }

    // TODO: CPPHandle
    auto get_type(const FB::IRType* fb_t) {
        halide_type_code_t type_code = get_type_code(fb_t);
        return Type(type_code, fb_t->runtime_type()->bits(), fb_t->runtime_type()->lanes());
    }

    auto get_memory_type(const FB::MemoryKind fb_memory_kind) {
        using namespace FB;
        static const std::unordered_map<MemoryKind, MemoryType> halide_memory_type = {
            {MemoryKind_Auto, MemoryType::Auto},
            {MemoryKind_Heap, MemoryType::Heap},
            {MemoryKind_Stack, MemoryType::Stack},
            {MemoryKind_Register, MemoryType::Register},
            {MemoryKind_GPUShared, MemoryType::GPUShared},
            {MemoryKind_GPUTexture, MemoryType::GPUTexture},
            {MemoryKind_LockedCache, MemoryType::LockedCache},
            {MemoryKind_VTCM, MemoryType::VTCM},
            {MemoryKind_AMXTile, MemoryType::AMXTile},
        };
        auto result = halide_memory_type.find(fb_memory_kind);
        assert((result != halide_memory_type.end()) && "No matching Halide MemoryType found");
        return result->second;
    }

    auto get_buffer_descriptor(const FB::BufferDescriptor *desc,
                               void* data = nullptr) {
        Type t = get_type(desc->shape()->type());
        auto dims = desc->shape()->dims();
        halide_dimension_t halide_dims[dims->size()];
        for (int i = 0; i < (int) dims->size(); i++) {
            auto dim = dims->Get(i);
            halide_dims[i] = halide_dimension_t(dim->min(), dim->extent(), dim->stride());
        }
        return Buffer(t, data, dims->size(), halide_dims, desc->name()->name()->str());
    }

    template<typename T>
    Expr visit_const(const FB::Expr *e) {
        Type t = get_type(e->type());
        auto fb_expr = static_cast<const T *>(e->node());
        return HL::make_const(t, fb_expr->value());
    }

    template<typename FB_TYPE, typename HL_TYPE>
    Expr visit_binary(const FB::Expr *e) {
        auto op = static_cast<const FB_TYPE *>(e->node());
        Expr a = convert(op->a());
        Expr b = convert(op->b());
        HL::IRPrinter irp(std::cerr);
        irp.print(a);
        std::cout << "\n" ;
        irp.print(b);
        std::cout << "\n" ;
        auto expr =  HL_TYPE::make(a, b);
        irp.print(expr);
        std::cout << "\n" ;
        return expr;
    }

    Expr visit_IntImm(const FB::Expr *e) {
        return visit_const<FB::IntImm>(e);
    }
    Expr visit_UIntImm(const FB::Expr *e) {
        return visit_const<FB::UIntImm>(e);
    }
    Expr visit_FloatImm(const FB::Expr *e) {
        return visit_const<FB::FloatImm>(e);
    }
    Expr visit_StringImm(const FB::Expr *e) {
        auto fb_string_imm = static_cast<const FB::StringImm *>(e->node());
        return HL::StringImm::make(fb_string_imm->value()->str());
    }

    Expr visit_Cast(const FB::Expr *e) {
        auto op = static_cast<const FB::Cast *>(e->node());
        Type t = get_type(e->type());
        Expr val = convert(op->value());
        return HL::Cast::make(t, val);
    }

    Expr visit_Reinterpret(const FB::Expr *e) {
        auto op = static_cast<const FB::Reinterpret *>(e->node());
        Type t = get_type(e->type());
        Expr val = convert(op->value());
        return HL::Reinterpret::make(t, val);
    }

    Expr visit_Add(const FB::Expr *e) {
        return visit_binary<FB::Add, HL::Add>(e);
    }
    Expr visit_Sub(const FB::Expr *e) {
        return visit_binary<FB::Sub, HL::Sub>(e);
    }
    Expr visit_Mul(const FB::Expr *e) {
        return visit_binary<FB::Mul, HL::Mul>(e);
    }
    Expr visit_Div(const FB::Expr *e) {
        return visit_binary<FB::Div, HL::Div>(e);
    }
    Expr visit_Mod(const FB::Expr *e) {
        return visit_binary<FB::Mod, HL::Mod>(e);
    }
    Expr visit_Min(const FB::Expr *e) {
        return visit_binary<FB::Min, HL::Min>(e);
    }
    Expr visit_Max(const FB::Expr *e) {
        return visit_binary<FB::Max, HL::Max>(e);
    }
    Expr visit_EQ(const FB::Expr *e) {
        return visit_binary<FB::EQ, HL::EQ>(e);
    }
    Expr visit_NE(const FB::Expr *e) {
        return visit_binary<FB::NE, HL::NE>(e);
    }
    Expr visit_LT(const FB::Expr *e) {
        return visit_binary<FB::LT, HL::LT>(e);
    }
    Expr visit_LE(const FB::Expr *e) {
        return visit_binary<FB::LE, HL::LE>(e);
    }
    Expr visit_GT(const FB::Expr *e) {
        return visit_binary<FB::GT, HL::GT>(e);
    }
    Expr visit_GE(const FB::Expr *e) {
        return visit_binary<FB::GE, HL::GE>(e);
    }
    Expr visit_And(const FB::Expr *e) {
        return visit_binary<FB::And, HL::And>(e);
    }
    Expr visit_Or(const FB::Expr *e) {
        return visit_binary<FB::Or, HL::Or>(e);
    }

    Expr visit_Not(const FB::Expr *e) {
        auto op = static_cast<const FB::Not *>(e->node());
        Expr val = convert(op->a());
        return HL::Not::make(val);
    }
    Expr visit_Select(const FB::Expr *e) {
        auto op = static_cast<const FB::Select *>(e->node());
        Expr condition = convert(op->condition());
        Expr true_value = convert(op->true_value());
        Expr false_value = convert(op->false_value());
        return HL::Select::make(condition, true_value, false_value);
    }

    Expr visit_Load(const FB::Expr *e) {
        auto fb_load = static_cast<const FB::Load *>(e->node());
        Expr predicate = convert(fb_load->predicate());
        Expr index = convert(fb_load->index());
        auto name = fb_load->name()->name()->str();
        auto alignment = HL::ModulusRemainder(fb_load->alignment()->modulus(), fb_load->alignment()->remainder());
        auto referent_type = fb_load->referent_type();
        auto referent = fb_load->referent();
        HL::Parameter param;
        // Only valid for non parameters (BufferConstant & BufferDescriptor)
        const FB::BufferDescriptor *desc = nullptr;
        void *data = nullptr;
        Type t = get_type(e->type());

        if (referent_type == FB::LoadReferent_BufferDescriptor) {
            std::cout << "case BufferDescriptor\n";
            desc = static_cast<const FB::BufferDescriptor *>(referent);
        } else if (referent_type == FB::LoadReferent_BufferConstant) {
            std::cout << "case BufferConstant\n";
            auto buffer_constant = static_cast<const FB::BufferConstant *>(referent);
            desc = buffer_constant->buffer();
            data = (void *) buffer_constant->data()->Data();
        } else if (referent_type == FB::LoadReferent_BufferParameter) {
            std::cout << "case BufferParameter\n";
            auto buffer_parameter = static_cast<const FB::BufferParameter *>(referent);
            auto buffer_desc = buffer_parameter->buffer();
            Type param_type = get_type(buffer_desc->shape()->type());
            auto buffer = get_buffer_descriptor(buffer_desc);
            auto buffer_memory_type = get_memory_type(buffer_parameter->memory_kind());
            auto buffer_constraints = buffer_parameter->buffer_constraints();

            param = HL::Parameter(param_type, true, buffer_constraints->size(), buffer_parameter->name()->name()->str());
            param.store_in(buffer_memory_type);
            param.set_host_alignment(buffer_parameter->host_alignment());
            param.set_buffer(buffer);
            int i = 0;
            for (auto buffer_constraint : *buffer_constraints) {
                param.set_min_constraint(i, convert(buffer_constraint->min()));
                param.set_extent_constraint(i, convert(buffer_constraint->extent()));
                param.set_stride_constraint(i, convert(buffer_constraint->stride()));
                param.set_min_constraint_estimate(i, convert(buffer_constraint->min_estimate()));
                param.set_extent_constraint_estimate(i, convert(buffer_constraint->extent_estimate()));
                i++;
            }
        } else if (referent_type == FB::LoadReferent_ScalarParameter) {
            std::cout << "case ScalarParameter\n";
            // ScalarParameter
            auto scalar_parameter = static_cast<const FB::ScalarParameter *>(referent);
            auto param_name = scalar_parameter->name()->name()->str();
            Type param_type = get_type(scalar_parameter->type());
            param = HL::Parameter(param_type, false, 0, param_name);
            param.set_min_value(convert(scalar_parameter->scalar_min()));
            param.set_max_value(convert(scalar_parameter->scalar_max()));
            param.set_estimate(convert(scalar_parameter->scalar_estimate()));
            param.set_default_value(convert(scalar_parameter->scalar_default()));
        } else {
            std::cout << "case NONE\n";
        }
        if (param.defined()) {
            return HL::Load::make(t, name, index, Buffer<>(), param, predicate, alignment);
        } else if (desc) {
            auto image = get_buffer_descriptor(desc, data);
            return HL::Load::make(t, name, index, image, HL::Parameter(), predicate, alignment);
        } else {
            return HL::Load::make(t, name, index, Buffer<>(), HL::Parameter(), predicate, alignment);
        }
    }
    Expr visit_NONE(const FB::Expr *e) {
        return Expr();
    }

    Expr visit(const FB::Expr* e) {
        using namespace FB;
        // typedef void(FBIRVisitor::*visitFuncPtr)(const Expr* e) ;
        auto node_type = e->node_type();
        switch (node_type) {
        case ExprNode_IntImm:
            return visit_IntImm(e);
        case ExprNode_UIntImm:
            return visit_UIntImm(e);
        case ExprNode_FloatImm:
            return visit_FloatImm(e);
        case ExprNode_StringImm:
            return visit_StringImm(e);
        case ExprNode_Cast:
            return visit_Cast(e);
        case ExprNode_Reinterpret:
            return visit_Reinterpret(e);
        case ExprNode_Variable:
            return visit_Variable(e);
        case ExprNode_Add:
            return visit_Add(e);
        case ExprNode_Sub:
            return visit_Sub(e);
        case ExprNode_Mul:
            return visit_Mul(e);
        case ExprNode_Div:
            return visit_Div(e);
        case ExprNode_Mod:
            return visit_Mod(e);
        case ExprNode_Min:
            return visit_Min(e);
        case ExprNode_Max:
            return visit_Max(e);
        case ExprNode_EQ:
            return visit_EQ(e);
        case ExprNode_NE:
            return visit_NE(e);
        case ExprNode_LT:
            return visit_LT(e);
        case ExprNode_LE:
            return visit_LE(e);
        case ExprNode_GT:
            return visit_GT(e);
        case ExprNode_GE:
            return visit_GE(e);
        case ExprNode_And:
            return visit_And(e);
        case ExprNode_Or:
            return visit_Or(e);
        case ExprNode_Not:
            return visit_Not(e);
        case ExprNode_Select:
            return visit_Select(e);
        case ExprNode_Load:
            return visit_Load(e);
        case ExprNode_Ramp:
            return visit_Ramp(e);
        case ExprNode_Broadcast:
            return visit_Broadcast(e);
        case ExprNode_Call:
            return visit_Call(e);
        case ExprNode_NONE:
            return visit_NONE(e);
        default:
            assert(false);
        }
    }
    Expr visit_Call(const FB::Expr* e) {return Expr();}
    Expr visit_Let(const FB::Expr* e) {return Expr();}
    Expr visit_Ramp(const FB::Expr* e) {
        // auto op = static_cast<const Ramp *>(e->node());
        // visit(op->base());
        // visit(op->stride());
        return Expr();
    }
    Expr visit_Broadcast(const FB::Expr* e) {
        // visit_unary<FlatBufferIR::Broadcast>(e);
        return Expr();
    }
    Expr visit_Variable(const FB::Expr* e) {
        // visit_unary<FlatBufferIR::Broadcast>(e);
        return Expr();
    }

public:
    Expr convert(const FB::Expr *fb_expr) {
        return this->visit(fb_expr);
    }
};

} // namespace

namespace Halide::FlatBufferIR {

uint8_t* to_flatbuffer_ir(Halide::Expr &e) {
    flatbuffers::FlatBufferBuilder builder;
    HalideToFlatBufferIR fb_ir(builder);
    auto fb_expr = fb_ir.convert(e);
    builder.Finish(fb_expr);
    return builder.GetBufferPointer();
}

Halide::Expr to_halide_ir(uint8_t *buffer_ptr) {
    auto fb_expr = flatbuffers::GetRoot<Expr>(buffer_ptr);
    FlatBufferToHalideIR halide_ir;
    return halide_ir.convert(fb_expr);
}

} // namespace FB