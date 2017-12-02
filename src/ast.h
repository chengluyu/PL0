#ifndef PL0_AST_H
#define PL0_AST_H

#include <vector>

#include "token.h"
#include "scope.h"
#include "symbol.h"

namespace pl0::ast {

#define DECLARATION_NODE_LIST(V) \
    V(variable_declaration) \
    V(constant_declaration) \
    V(procedure_declaration) \

#define EXPRESSION_NODE_LIST(V) \
    V(binary_operation) \
    V(unary_operation) \
    V(literal) \
    V(variable_proxy)

#define STATEMENT_NODE_LIST(V) \
    V(statement_list) \
    V(block) \
    V(if_statement) \
    V(while_statement) \
    V(call_statement) \
    V(read_statement) \
    V(write_statement) \
    V(assign_statement) \
    V(return_statement)

#define AST_NODE_LIST(V) \
    DECLARATION_NODE_LIST(V) \
    EXPRESSION_NODE_LIST(V) \
    STATEMENT_NODE_LIST(V)

#define DEF_FORWARD_DECLARATION(type) class type;
AST_NODE_LIST(DEF_FORWARD_DECLARATION)
#undef DEF_FORWARD_DECLARATION

#define DEF_ENUM_AST_NODE_TYPE(type) type,
enum class ast_node_type {
    AST_NODE_LIST(DEF_ENUM_AST_NODE_TYPE)
};
#undef DEF_ENUM_AST_NODE_TYPE

#define PROPERTY_GETTER(field) decltype(field##_) field() const { return field##_; }

#define PROPERTY_CONST_REF_GETTER(field) const decltype(field##_) &field() const { return field##_; }

class ast_node {
    ast_node_type type_;
public:
    explicit ast_node(ast_node_type type) : type_(type) { }

    ast_node_type get_type() const {
        return type_;
    }

    virtual ~ast_node() = default;
};

class declaration : public ast_node {
public:
    explicit declaration(ast_node_type type) : ast_node(type) { }

    ~declaration() override = default;
};

class expression : public ast_node {
public:
    explicit expression(ast_node_type type) : ast_node(type) { }

    ~expression() override = default;
};

class statement : public ast_node {
public:
    explicit statement(ast_node_type type) : ast_node(type) { }

    ~statement() override = default;
};

class variable_declaration : public declaration {
public:
    typedef std::vector<variable*> list_type;

private:
    const list_type variables_;

public:
    explicit variable_declaration(list_type variables)
            : declaration(ast_node_type::variable_declaration), variables_(std::move(variables)) { }

    ~variable_declaration() override = default;

    PROPERTY_CONST_REF_GETTER(variables)
};

class constant_declaration : public declaration {
public:
    typedef std::vector<constant *> list_type;

private:
    const list_type constants_;

public:
    explicit constant_declaration(list_type constants)
            : declaration(ast_node_type::constant_declaration), constants_(std::move(constants)) { }

    ~constant_declaration() override = default;

    PROPERTY_CONST_REF_GETTER(constants)
};


class procedure_declaration : public declaration {
    procedure *symbol_;
    block *main_block_;
public:
    procedure_declaration(procedure *symbol, block *main_block)
            : declaration(ast_node_type::procedure_declaration), symbol_(symbol), main_block_(main_block) { }

    ~procedure_declaration() final = default;

    PROPERTY_GETTER(symbol)

    PROPERTY_GETTER(main_block)
};

class block : public statement {
    scope *belonging_scope_;
    variable_declaration *var_declaration_;
    constant_declaration *const_declaration_;
    std::vector<procedure_declaration *> sub_procedures_;
    statement *body_;
public:
    block(scope *belonging_scope,
          variable_declaration *var_declaration,
          constant_declaration *const_declaration,
          std::vector<procedure_declaration *> sub_procedures,
          statement *body)
            : statement(ast_node_type::block),
              belonging_scope_(belonging_scope),
              var_declaration_(var_declaration),
              const_declaration_(const_declaration),
              sub_procedures_(std::move(sub_procedures)),
              body_(body) { }

    ~block() final {
        delete var_declaration_;
        delete const_declaration_;
        for (auto dec : sub_procedures_)
            delete dec;
        delete body_;
    }

    PROPERTY_GETTER(belonging_scope)

    PROPERTY_GETTER(var_declaration)

    PROPERTY_GETTER(const_declaration)

    PROPERTY_CONST_REF_GETTER(sub_procedures)

    PROPERTY_GETTER(body)
};

class statement_list : public statement {
    const std::vector<statement *> statements_;
public:
    typedef std::vector<statement *> list_type;

    explicit statement_list(list_type statements)
            : statement(ast_node_type::statement_list), statements_(std::move(statements)) { }

    ~statement_list() final = default;

    PROPERTY_CONST_REF_GETTER(statements)
};

class if_statement : public statement {
    expression *condition_;
    statement *then_statement_;
    statement *else_statement_;
public:
    if_statement(expression *condition, statement *then_statement, statement *else_statement)
            : statement(ast_node_type::if_statement),
              condition_(condition),
              then_statement_(then_statement),
              else_statement_(else_statement) { }

    ~if_statement() final {
        delete condition_;
        delete then_statement_;
        delete else_statement_;
    }

    bool has_else_statement() const {
        return else_statement_ != nullptr;
    }

    PROPERTY_GETTER(condition)

    PROPERTY_GETTER(then_statement)

    PROPERTY_GETTER(else_statement)
};

class while_statement : public statement {
    expression *cond_;
    statement *body_;
public:
    while_statement(expression *cond, statement *body)
            : statement(ast_node_type::while_statement), cond_(cond), body_(body) { }

    ~while_statement() final {
        delete cond_;
        delete body_;
    }

    PROPERTY_GETTER(cond)

    PROPERTY_GETTER(body)
};

class call_statement : public statement {
    std::string callee_;
public:
    explicit call_statement(std::string callee)
            : statement(ast_node_type::call_statement), callee_(std::move(callee)) { }

    ~call_statement() final = default;

    PROPERTY_CONST_REF_GETTER(callee)
};

class read_statement : public statement {
    const std::vector<variable_proxy *> targets_;
public:
    typedef std::vector<variable_proxy *> list_type;

    explicit read_statement(list_type targets)
            : statement(ast_node_type::read_statement), targets_(std::move(targets)) { }

    ~read_statement() final = default;

    PROPERTY_CONST_REF_GETTER(targets)
};

class write_statement : public statement {
    const std::vector<expression *> expressions_;
public:
    typedef std::vector<expression *> list_type;

    explicit write_statement(list_type expressions)
            : statement(ast_node_type::write_statement), expressions_(std::move(expressions)) { }

    ~write_statement() final = default;

    PROPERTY_CONST_REF_GETTER(expressions)
};

class assign_statement : public statement {
    variable_proxy *target_;
    expression *expr_;
public:
    assign_statement(variable_proxy *target, expression *expr)
            : statement(ast_node_type::assign_statement), target_(target), expr_(expr) { }

    ~assign_statement() final;

    PROPERTY_GETTER(target)

    PROPERTY_GETTER(expr)
};

class return_statement : public statement {
public:
    return_statement() : statement(ast_node_type::return_statement) { }

    ~return_statement() final = default;
};

class unary_operation : public expression {
    token op_;
    expression *expr_;
public:
    unary_operation(token op, expression *expr) : expression(ast_node_type::unary_operation), op_(op), expr_(expr) { }

    ~unary_operation() final {
        delete expr_;
    }

    PROPERTY_GETTER(op)

    PROPERTY_GETTER(expr)
};

class binary_operation : public expression {
    token op_;
    expression *left_;
    expression *right_;
public:
    binary_operation(token op, expression *left, expression *right)
            : expression(ast_node_type::binary_operation), op_(op), left_(left), right_(right) { }

    ~binary_operation() final {
        delete left_;
        delete right_;
    }

    PROPERTY_GETTER(op)

    PROPERTY_GETTER(left)

    PROPERTY_GETTER(right)
};

class variable_proxy : public expression {
    variable *target_;
public:
    explicit variable_proxy(variable *target)
            : expression(ast_node_type::variable_proxy), target_(target) { }

    ~variable_proxy() final = default; // TODO add a proper destructor here

    PROPERTY_GETTER(target)
};

class literal : public expression {
    int value_;
public:
    explicit literal(int value)
            : expression(ast_node_type::literal), value_(value) { }

    ~literal() final = default;

    PROPERTY_GETTER(value)
};

template <class Visitor>
class ast_visitor {
protected:
    Visitor *impl() {
        return static_cast<Visitor*>(this);
    }
public:
    void visit(ast_node *node) {
        impl()->visit(node);
    }
};

#define DECLARE_VISIT(type) \
    void visit_##type(type *node);

#define DECLARE_VISIT_METHODS \
    AST_NODE_LIST(DECLARE_VISIT)

#define GENERATE_VISIT_CASE(type) \
    case ast_node_type::type: \
        return this->impl()->visit_##type(dynamic_cast<type*>(node));

#define GENERATE_AST_VISITOR_SWITCH() \
    switch(node->get_type()) { \
        AST_NODE_LIST(GENERATE_VISIT_CASE) \
    }

#define DEFINE_AST_VISITOR_SUBCLASS_MEMBERS() \
    void visit(ast_node *node) { \
        GENERATE_AST_VISITOR_SWITCH() \
    }

}

#endif //PL0_AST_H
