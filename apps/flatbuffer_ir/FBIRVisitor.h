#pragma once

#include <map>
#include "halide_ir_generated.h"

/** \file
 * Defines the base class for things that recursively walk over the IR
 */

namespace Halide::FlatBufferIR {

/** A base class for algorithms that need to recursively walk over the
 * IR. The default implementations just recursively walk over the
 * children. Override the ones you care about.
 */
class FBIRVisitor {
    template<typename T> void visit_unary(const Expr* e);
    template<typename T> void visit_binary(const Expr* e);
public:
    FBIRVisitor() = default;
    virtual ~FBIRVisitor() = default;
    void visit(const Expr* e);

protected:
    // // ExprNode<> and StmtNode<> are allowed to call visit (to implement accept())
    // template<typename T>
    // friend struct ExprNode;

    // template<typename T>
    // friend struct StmtNode;

    virtual void visit_IntImm(const Expr* e);
    virtual void visit_UIntImm(const Expr* e);
    virtual void visit_FloatImm(const Expr* e);
    virtual void visit_StringImm(const Expr* e);
    virtual void visit_NONE(const Expr* e);
    virtual void visit_Cast(const Expr* e);
    virtual void visit_Reinterpret(const Expr* e);
    virtual void visit_Variable(const Expr* e);
    virtual void visit_Add(const Expr* e);
    virtual void visit_Sub(const Expr* e);
    virtual void visit_Mul(const Expr* e);
    virtual void visit_Div(const Expr* e);
    virtual void visit_Mod(const Expr* e);
    virtual void visit_Min(const Expr* e);
    virtual void visit_Max(const Expr* e);
    virtual void visit_EQ(const Expr* e);
    virtual void visit_NE(const Expr* e);
    virtual void visit_LT(const Expr* e);
    virtual void visit_LE(const Expr* e);
    virtual void visit_GT(const Expr* e);
    virtual void visit_GE(const Expr* e);
    virtual void visit_And(const Expr* e);
    virtual void visit_Or(const Expr* e);
    virtual void visit_Not(const Expr* e);
    virtual void visit_Select(const Expr* e);
    virtual void visit_Load(const Expr* e);
    virtual void visit_Ramp(const Expr* e);
    virtual void visit_Broadcast(const Expr* e);
    virtual void visit_Call(const Expr* e);
    virtual void visit_Let(const Expr* e);
    virtual void visit_LetStmt(const Expr* e);
    virtual void visit_AssertStmt(const Expr* e);
    virtual void visit_ProducerConsumer(const Expr* e);
    virtual void visit_For(const Expr* e);
    virtual void visit_Store(const Expr* e);
    virtual void visit_Provide(const Expr* e);
    virtual void visit_Allocate(const Expr* e);
    virtual void visit_Free(const Expr* e);
    virtual void visit_Realize(const Expr* e);
    virtual void visit_Block(const Expr* e);
    virtual void visit_IfThenElse(const Expr* e);
    virtual void visit_Evaluate(const Expr* e);
    virtual void visit_Shuffle(const Expr* e);
    virtual void visit_VectorReduce(const Expr* e);
    virtual void visit_Prefetch(const Expr* e);
    virtual void visit_Fork(const Expr* e);
    virtual void visit_Acquire(const Expr* e);
    virtual void visit_Atomic(const Expr* e);

/*
    void visit(const Stmt e) {
        typedef void (*visitFuncPtr)(const Stmt *) ;
        auto node_type = e.node_type();
        static std::map<int, visitFuncPtr> vtable = {
            {Stmt_LetStmt, &FBIRVisitor::visit_LetStmt},
            {Stmt_AssertStmt, &FBIRVisitor::visit_AssertStmt},
            {Stmt_ProducerConsumer, &FBIRVisitor::visit_ProducerConsumer},
            {Stmt_For, &FBIRVisitor::visit_For},
            {Stmt_Store, &FBIRVisitor::visit_Store},
            {Stmt_Provide, &FBIRVisitor::visit_Provide},
            {Stmt_Allocate, &FBIRVisitor::visit_Allocate},
            {Stmt_Free, &FBIRVisitor::visit_Free},
            {Stmt_Realize, &FBIRVisitor::visit_Realize},
            {Stmt_Block, &FBIRVisitor::visit_Block},
            {Stmt_IfThenElse, &FBIRVisitor::visit_IfThenElse},
            {Stmt_Evaluate, &FBIRVisitor::visit_Evaluate},
            {Stmt_Shuffle, &FBIRVisitor::visit_Shuffle},
            {Stmt_VectorReduce, &FBIRVisitor::visit_VectorReduce},
            {Stmt_Prefetch, &FBIRVisitor::visit_Prefetch},
            {Stmt_Fork, &FBIRVisitor::visit_Fork},
            {Stmt_Acquire, &FBIRVisitor::visit_Acquire},
            {Stmt_Atomic, &FBIRVisitor::visit_sAtomic},
        };
        return (this->*(vtable[node_type]))(e.node());
    }
*/
};


}  // namespace Halide::FlatBufferIR
