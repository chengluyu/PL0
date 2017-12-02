#include "ast.h"

namespace pl0::ast {


assign_statement::~assign_statement() {
    delete target_;
    delete expr_;
}

}

