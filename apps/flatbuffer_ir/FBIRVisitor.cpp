#include "FBIRVisitor.h"
#include <iostream>

namespace Halide::FlatBufferIR {

void FBIRVisitor::visit(const Expr* e) {
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

void FBIRVisitor::visit_IntImm(const Expr* e) {
}
void FBIRVisitor::visit_UIntImm(const Expr* e) {
}
void FBIRVisitor::visit_FloatImm(const Expr* e) {
}
void FBIRVisitor::visit_StringImm(const Expr* e) {
}
void FBIRVisitor::visit_NONE(const Expr* e) {
}

template<typename T>
void FBIRVisitor::visit_unary(const FlatBufferIR::Expr *e) {
    auto op = static_cast<const T *>(e->node());
    visit(op->value());
}
void FBIRVisitor::visit_Cast(const Expr* e) {
    visit_unary<FlatBufferIR::Cast>(e);
}
void FBIRVisitor::visit_Reinterpret(const Expr* e) {
    visit_unary<FlatBufferIR::Reinterpret>(e);
}

void FBIRVisitor::visit_Variable(const Expr* e) {}

template<typename T>
void FBIRVisitor::visit_binary(const FlatBufferIR::Expr *e) {
    auto op = static_cast<const T *>(e->node());
    visit(op->a());
    visit(op->b());
}

void FBIRVisitor::visit_Add(const Expr* e) {
    visit_binary<FlatBufferIR::Add>(e);
}
void FBIRVisitor::visit_Sub(const Expr* e) {
    visit_binary<FlatBufferIR::Sub>(e);
}
void FBIRVisitor::visit_Mul(const Expr* e) {
    visit_binary<FlatBufferIR::Mul>(e);
}
void FBIRVisitor::visit_Div(const Expr* e) {
    visit_binary<FlatBufferIR::Div>(e);
}
void FBIRVisitor::visit_Mod(const Expr* e) {
    visit_binary<FlatBufferIR::Mod>(e);
}
void FBIRVisitor::visit_Min(const Expr* e) {
    visit_binary<FlatBufferIR::Min>(e);
}
void FBIRVisitor::visit_Max(const Expr* e) {
    visit_binary<FlatBufferIR::Max>(e);
}
void FBIRVisitor::visit_EQ(const Expr* e) {
    visit_binary<FlatBufferIR::EQ>(e);
}
void FBIRVisitor::visit_NE(const Expr* e) {
    visit_binary<FlatBufferIR::NE>(e);
}
void FBIRVisitor::visit_LT(const Expr* e) {
    visit_binary<FlatBufferIR::LT>(e);
}
void FBIRVisitor::visit_LE(const Expr* e) {
    visit_binary<FlatBufferIR::LE>(e);
}
void FBIRVisitor::visit_GT(const Expr* e) {
    visit_binary<FlatBufferIR::GT>(e);
}
void FBIRVisitor::visit_GE(const Expr* e) {
    visit_binary<FlatBufferIR::GE>(e);
}
void FBIRVisitor::visit_And(const Expr* e) {
    visit_binary<FlatBufferIR::And>(e);
}
void FBIRVisitor::visit_Or(const Expr* e) {
    visit_binary<FlatBufferIR::Or>(e);
}
void FBIRVisitor::visit_Not(const Expr* e) {
    visit_unary<FlatBufferIR::Reinterpret>(e);
}

void FBIRVisitor::visit_Select(const Expr* e) {
    auto op = static_cast<const Select *>(e->node());
    visit(op->condition());
    visit(op->true_value());
    visit(op->false_value());
}
void FBIRVisitor::visit_Load(const Expr* e) {
    auto op = static_cast<const Load *>(e->node());
    visit(op->predicate());
    visit(op->index());
}
void FBIRVisitor::visit_Ramp(const Expr* e) {
    auto op = static_cast<const Ramp *>(e->node());
    visit(op->base());
    visit(op->stride());
}
void FBIRVisitor::visit_Broadcast(const Expr* e) {
    visit_unary<FlatBufferIR::Broadcast>(e);
}
void FBIRVisitor::visit_Call(const Expr* e) {}
void FBIRVisitor::visit_Let(const Expr* e) {}
void FBIRVisitor::visit_LetStmt(const Expr* e) {}
void FBIRVisitor::visit_AssertStmt(const Expr* e) {}
void FBIRVisitor::visit_ProducerConsumer(const Expr* e) {}
void FBIRVisitor::visit_For(const Expr* e) {}
void FBIRVisitor::visit_Store(const Expr* e) {}
void FBIRVisitor::visit_Provide(const Expr* e) {}
void FBIRVisitor::visit_Allocate(const Expr* e) {}
void FBIRVisitor::visit_Free(const Expr* e) {}
void FBIRVisitor::visit_Realize(const Expr* e) {}
void FBIRVisitor::visit_Block(const Expr* e) {}
void FBIRVisitor::visit_IfThenElse(const Expr* e) {}
void FBIRVisitor::visit_Evaluate(const Expr* e) {}
void FBIRVisitor::visit_Shuffle(const Expr* e) {}
void FBIRVisitor::visit_VectorReduce(const Expr* e) {}
void FBIRVisitor::visit_Prefetch(const Expr* e) {}
void FBIRVisitor::visit_Fork(const Expr* e) {}
void FBIRVisitor::visit_Acquire(const Expr* e) {}
void FBIRVisitor::visit_Atomic(const Expr* e) {}

}  // namespace Halide::FlatBufferIR 
