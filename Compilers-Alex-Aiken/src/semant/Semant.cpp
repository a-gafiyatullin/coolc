#include "semant/Semant.h"

using namespace semant;

// ---------------------------------------- SEMANT LOG ----------------------------------------
#ifdef SEMANT_FULL_VERBOSE
void Semant::log(const std::string &msg) const
{
    std::cout << std::string(_level, ' ') << msg << std::endl;
}

void Semant::log_enter(const std::string &msg)
{
    log("Enter " + msg);
    _level += 4;
}

void Semant::log_exit(const std::string &msg)
{
    _level -= 4;
    log("Exit " + msg);
}
#endif // SEMANT_FULL_VERBOSE

// ---------------------------------------- SCOPE ----------------------------------------
const std::string Scope::_self = "self";

Scope::ADD_RESULT Scope::add_if_can(const std::string &name, const std::shared_ptr<ast::Type> &type)
{
    SEMANT_RETURN_IF_FALSE(name != _self, RESERVED);

    SEMANT_FULL_VERBOSE_ONLY(assert(_symbols.size() != 0));
    SEMANT_RETURN_IF_FALSE(_symbols.back().find(name) == _symbols.back().end(), REDEFINED);

    _symbols.back()[name] = type;
    return OK;
}

std::shared_ptr<ast::Type> Scope::find(const std::string &name, const int &scope_shift) const
{
    SEMANT_FULL_VERBOSE_ONLY(dump());

    for (auto class_scope = _symbols.rbegin() + scope_shift; class_scope != _symbols.rend(); class_scope++)
    {
        const auto symbol = class_scope->find(name);
        if (symbol != class_scope->end())
        {
            return symbol->second;
        }
    }

    return nullptr;
}

// ---------------------------------------- INITIALIZATION ----------------------------------------

std::shared_ptr<ast::Type> Semant::_bool = nullptr;
std::shared_ptr<ast::Type> Semant::_object = nullptr;
std::shared_ptr<ast::Type> Semant::_int = nullptr;
std::shared_ptr<ast::Type> Semant::_string = nullptr;
std::shared_ptr<ast::Type> Semant::_io = nullptr;
std::shared_ptr<ast::Type> Semant::_object_parent = nullptr;
std::shared_ptr<ast::Type> Semant::_self_type = nullptr;

std::shared_ptr<ast::Program> Semant::merge_to_one_program(const std::vector<std::shared_ptr<ast::Program>> &programs)
{
    if (programs.empty())
    {
        return nullptr;
    }

    auto program = std::make_shared<ast::Program>();
    program->_line_number = programs.at(0)->_line_number;

    std::for_each(programs.begin(), programs.end(), [program](const std::shared_ptr<ast::Program> &p)
                  { program->_classes.insert(program->_classes.end(), p->_classes.begin(), p->_classes.end()); });

    return program;
}

Semant::Semant(std::vector<std::shared_ptr<ast::Program>> &programs) : _program(merge_to_one_program(programs))
{
    _object_parent = std::make_shared<ast::Type>();
    _object_parent->_string = "";
}

// ---------------------------------------- CLASS CHECK ----------------------------------------

std::shared_ptr<Semant::ClassNode> Semant::create_basic_class(const std::string &name,
                                                              const std::string &parent,
                                                              const std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> methods)
{
    auto class_ = std::make_shared<ClassNode>();
    class_->_class = std::make_shared<ast::Class>();
    class_->_class->_type = std::make_shared<ast::Type>();
    class_->_class->_parent = std::make_shared<ast::Type>();

    class_->_class->_type->_string = name;
    class_->_class->_parent->_string = parent;
    for (const auto &m : methods)
    {
        auto feature = std::make_shared<ast::Feature>();
        feature->_base = ast::MethodFeature();

        // method name
        feature->_object = std::make_shared<ast::ObjectExpression>();
        feature->_object->_object = m.first;

        // method ret type
        SEMANT_FULL_VERBOSE_ONLY(assert(!methods.empty()));
        feature->_type = std::make_shared<ast::Type>();
        feature->_type->_string = m.second.front().second;

        // method args
        for (int i = 1; i < m.second.size(); i++)
        {
            // formal name
            std::get<ast::MethodFeature>(feature->_base)._formals.push_back(std::make_shared<ast::Formal>());
            std::get<ast::MethodFeature>(feature->_base)._formals.back()->_object = std::make_shared<ast::ObjectExpression>();
            std::get<ast::MethodFeature>(feature->_base)._formals.back()->_object->_object = m.second[i].first;

            // formal type
            std::get<ast::MethodFeature>(feature->_base)._formals.back()->_type = std::make_shared<ast::Type>();
            std::get<ast::MethodFeature>(feature->_base)._formals.back()->_type->_string = m.second[i].second;
        }

        class_->_class->_features.push_back(feature);
    }

    _classes.insert(std::make_pair(name, class_));
    return class_;
}

bool Semant::check_class_hierarchy_for_cycle(const std::shared_ptr<ClassNode> &class_, std::unordered_map<std::string, int> &visited, const int &loop)
{
    if (visited[class_->_class->_type->_string] == loop)
    {
        return false;
    }

    visited[class_->_class->_type->_string] = loop;

    for (const auto &child : class_->_children)
    {
        SEMANT_RETURN_IF_FALSE(check_class_hierarchy_for_cycle(child, visited, loop), false);
        visited[class_->_class->_type->_string] = false;
    }

    return true;
}

bool Semant::is_not_basic_class(const std::shared_ptr<ast::Type> &class_)
{
    return !(same_type(class_, _int) || same_type(class_, _bool) || same_type(class_, _string) ||
             same_type(class_, _object) || same_type(class_, _io) || same_type(class_, _self_type));
}

bool Semant::is_inherit_allowed(const std::shared_ptr<ast::Type> &class_)
{
    return !(same_type(class_, _int) || same_type(class_, _bool) || same_type(class_, _string) || same_type(class_, _self_type));
}

bool Semant::check_class_hierarchy()
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("check class hierarchy"));

    std::vector<std::shared_ptr<ClassNode>> delayed_parent;
    for (const auto &class_ : _program->_classes)
    {
        _error_file_name = class_->_file_name; // for error

        SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_not_basic_class(class_->_type), "Redefinition of basic class " + class_->_type->_string + ".",
                                          class_->_line_number, false);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(_classes.find(class_->_type->_string) == _classes.end(), "Class " + class_->_type->_string + " was previously defined.",
                                          class_->_line_number, false);

        auto new_class = std::make_shared<ClassNode>();
        new_class->_class = class_;
        _classes.insert(std::make_pair(class_->_type->_string, new_class));

        auto parent = _classes.find(class_->_parent->_string);
        if (parent == _classes.end())
        {
            delayed_parent.push_back(new_class);
        }
        else
        {
            SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_inherit_allowed(parent->second->_class->_type),
                                              "Class " + class_->_type->_string + " cannot inherit class " + parent->second->_class->_type->_string + ".",
                                              class_->_line_number, false);
            parent->second->_children.push_back(new_class);
        }
    }

    for (const auto &class_ : delayed_parent)
    {
        _error_file_name = class_->_class->_file_name; // for error

        auto parent = _classes.find(class_->_class->_parent->_string);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(parent != _classes.end(),
                                          "Class " + class_->_class->_type->_string + " inherits from an undefined class " + class_->_class->_parent->_string + ".",
                                          class_->_class->_line_number, false);
        parent->second->_children.push_back(class_);
    }

    std::unordered_map<std::string, int> visited;
    for (const auto &class_ : _classes)
    {
        visited.insert(std::make_pair(class_.first, -1));
    }

    int loop_num = 0;
    int cycle_num = 1;
    for (const auto &class_ : _classes)
    {
        if (visited[class_.first] != -1)
        {
            continue;
        }
        // format error
        if (!check_class_hierarchy_for_cycle(class_.second, visited, loop_num))
        {
            _error_line_number = -1;
            _error_message += "Cycle " + std::to_string(cycle_num) + ":\n";
            cycle_num++;
            for (const auto &class_ : visited)
            {
                if (class_.second == loop_num)
                {
                    const auto class_node = _classes[class_.first]->_class;
                    _error_message += class_node->_file_name + ":" + std::to_string(class_node->_line_number) +
                                      " Class " + class_node->_type->_string + " or an ancestor of " + class_node->_type->_string + " is involved in an inheritance cycle.\n";
                }
            }
        }
        loop_num++;
    }
    if (_error_message != "")
    {
        // delete new line
        _error_message = _error_message.substr(0, _error_message.length() - 1);
        return false;
    }

    SEMANT_FULL_VERBOSE_ONLY(log_exit("check class hierarchy"));
    return true;
}

const std::string Semant::_main_class = "Main";
const std::string Semant::_main_method = "main";

bool Semant::check_main()
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("check main"));

    bool found_main = false;

    auto main_class = _classes.find(_main_class);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(main_class != _classes.end(), "Class Main is not defined.", -1, false);

    for (const auto &feature : main_class->second->_class->_features)
    {
        if (std::holds_alternative<ast::MethodFeature>(feature->_base))
        {
            // main method's name main and 0 parameters
            // further we check if it is an only such method
            if (feature->_object->_object == _main_method && std::get<ast::MethodFeature>(feature->_base)._formals.size() == 0)
            {
                found_main = true;
                break;
            }
        }
    }

    SEMANT_RETURN_IF_FALSE_WITH_ERROR(found_main, "No 'main' method in class Main.", _classes[_main_class]->_class->_line_number, false);

    SEMANT_FULL_VERBOSE_ONLY(log_exit("check main"));
    return true;
}

bool Semant::check_classes()
{
    // 1. We can inherit classes only from Object, IO and user defined classes
    // add Object to hierarchy
    SEMANT_FULL_VERBOSE_ONLY(log_enter("create basic classes"));

    _root = create_basic_class("Object", _object_parent->_string,
                               {{"abort", {{"", "Object"}}},
                                {"type_name", {{"", "String"}}},
                                {"copy", {{"", "SELF_TYPE"}}}});
    _object = _root->_class->_type;

    // add IO to hierarchy
    _root->_children.push_back(create_basic_class("IO", "Object",
                                                  {{"out_string", {{"", "SELF_TYPE"}, {"x", "String"}}},
                                                   {"out_int", {{"", "SELF_TYPE"}, {"x", "Int"}}},
                                                   {"in_string", {{"", "String"}}},
                                                   {"in_int", {{"", "Int"}}}}));
    _io = _root->_children.back()->_class->_type;

    // add Int to hierarchy
    _root->_children.push_back(create_basic_class("Int", "Object", {}));
    _int = _root->_children.back()->_class->_type;

    // add Bool to hierarchy
    _root->_children.push_back(create_basic_class("Bool", "Object", {}));
    _bool = _root->_children.back()->_class->_type;

    // add SELF_TYPE to hierarchy
    _root->_children.push_back(create_basic_class("SELF_TYPE", "Object", {}));
    _self_type = _root->_children.back()->_class->_type;

    // add String to hierarchy
    _root->_children.push_back(create_basic_class("String", "Object",
                                                  {{"length", {{"", "Int"}}},
                                                   {"concat", {{"", "String"}, {"s", "String"}}},
                                                   {"substr", {{"", "String"}, {"i", "Int"}, {"l", "Int"}}}}));
    _string = _root->_children.back()->_class->_type;

    SEMANT_FULL_VERBOSE_ONLY(log_exit("create basic classes"));

    // 2. Add user defined classes to hierarchy
    SEMANT_RETURN_IF_FALSE(check_class_hierarchy(), false);

    // 3. Check main method in Main class
    SEMANT_RETURN_IF_FALSE(check_main(), false);

    return true;
}

// ---------------------------------------- TYPES CHECK ----------------------------------------

std::shared_ptr<ast::Program> Semant::infer_types_and_check()
{
    SEMANT_RETURN_IF_FALSE(check_classes(), nullptr);
    SEMANT_RETURN_IF_FALSE(check_expressions(), nullptr);

    return _program;
}

bool Semant::check_expression_in_method(const std::shared_ptr<ast::Feature> &feature, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("check method " + feature->_object->_object));

    scope.push_scope();

    const ast::MethodFeature &this_method = std::get<ast::MethodFeature>(feature->_base);
    const auto class_node = _classes[_current_class->_string];

    // return type if defined
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(feature->_type),
                                      "Undefined return type " + feature->_type->_string + " in method " + feature->_object->_object + ".",
                                      feature->_line_number, false);

    // multiple defined in one class
    const bool is_ones_defined = std::count_if(class_node->_class->_features.begin(), class_node->_class->_features.end(),
                                               [feature](const std::shared_ptr<ast::Feature> &m)
                                               { return (std::holds_alternative<ast::MethodFeature>(m->_base) &&
                                                         m->_object->_object == feature->_object->_object); }) == 1;
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_ones_defined, "Method " + feature->_object->_object + " is multiply defined.", feature->_line_number, false);

    // parent method
    const auto parent_feature = find_method(feature->_object->_object, class_node->_class->_type, false);
    ast::MethodFeature parent_method;

    if (parent_feature)
    {
        parent_method = std::get<ast::MethodFeature>(parent_feature->_base);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(this_method._formals.size() == parent_method._formals.size(),
                                          "Incompatible number of formal parameters in redefined method " + feature->_object->_object + ".",
                                          feature->_line_number, false);
    }

    int formal_num = 0;
    // add formal parameters to scope and check method overriding
    for (const auto &formal : this_method._formals)
    {
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(!same_type(formal->_type, _self_type),
                                          "Formal parameter " + formal->_object->_object + " cannot have type SELF_TYPE.", formal->_line_number, false);
        // formal type if defined
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(formal->_type),
                                          "Class " + formal->_type->_string + " of formal parameter " + formal->_object->_object + " is undefined.",
                                          formal->_line_number, false);

        const auto result = scope.add_if_can(formal->_object->_object, formal->_type);
        if (result != Scope::ADD_RESULT::OK)
        {
            _error_line_number = formal->_line_number;
            if (result == Scope::ADD_RESULT::REDEFINED)
            {
                _error_message = "Formal parameter " + formal->_object->_object + " is multiply defined.";
            }
            else if (result == Scope::ADD_RESULT::RESERVED)
            {
                _error_message = "'" + formal->_object->_object + "' cannot be the name of a formal parameter.";
            }
            return false;
        }

        if (parent_feature)
        {
            const auto original_type = parent_method._formals[formal_num]->_type;
            SEMANT_RETURN_IF_FALSE_WITH_ERROR(same_type(original_type, formal->_type),
                                              "In redefined method " + feature->_object->_object + ", parameter type " + formal->_type->_string +
                                                  " is different from original type " + original_type->_string,
                                              formal->_line_number, false);
        }

        formal_num++;
    }

    if (feature->_expr)
    {
        SEMANT_RETURN_IF_FALSE(infer_expression_type(feature->_expr, scope), false);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(feature->_expr->_type, feature->_type, true),
                                          "Inferred return type " + feature->_expr->_type->_string + " of method " +
                                              feature->_object->_object + " does not conform to declared return type " + feature->_type->_string + ".",
                                          feature->_line_number, false);
    }

    scope.pop_scope();

    SEMANT_FULL_VERBOSE_ONLY(log_exit("check method " + feature->_object->_object));
    return true;
}

bool Semant::check_expression_in_attribute(const std::shared_ptr<ast::Feature> &attr, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("check attribute " + attr->_object->_object));

    // type if defined
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(attr->_type),
                                      "Class " + attr->_type->_string + " of attribute " + attr->_object->_object + " is undefined.",
                                      attr->_line_number, false);

    if (attr->_expr)
    {
        SEMANT_RETURN_IF_FALSE(infer_expression_type(attr->_expr, scope), false);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(attr->_expr->_type, attr->_type, true),
                                          "Inferred type " + attr->_expr->_type->_string + " of initialization of attribute a does not conform to declared type " +
                                              attr->_type->_string + ".",
                                          -1, false);
    }

    SEMANT_FULL_VERBOSE_ONLY(log_exit("check attribute " + attr->_object->_object));
    return true;
}

bool Semant::check_expressions_in_class(const std::shared_ptr<ClassNode> &node, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("check class " + node->_class->_type->_string));
    _error_file_name = node->_class->_file_name; // save for error messages

    scope.push_scope();

    const auto prev_class = _current_class;
    _current_class = node->_class->_type; // current SELF_TYPE

    // 1. Add class attributes to current scope and check if redefined
    for (const auto &feature : node->_class->_features)
    {
        if (std::holds_alternative<ast::AttrFeature>(feature->_base))
        {
            const auto result = scope.add_if_can(feature->_object->_object, feature->_type);
            _error_line_number = feature->_line_number;
            if (result == Scope::RESERVED)
            {
                _error_message = "\'" + feature->_object->_object + "\' cannot be the name of an attribute.";
                return false;
            }
            else if (result == Scope::REDEFINED)
            {
                _error_message = "Attribute " + feature->_object->_object + " is multiply defined in class.";
                return false;
            }

            // check if attribute is inherited from parent or redefined
            const auto parent_attr = scope.find(feature->_object->_object, 1);
            SEMANT_RETURN_IF_FALSE_WITH_ERROR(!parent_attr, "Attribute " + feature->_object->_object + " is an attribute of an inherited class.",
                                              feature->_line_number, false);
        }
    }

    // 2. Check features types
    for (const auto &feature : node->_class->_features)
    {
        if (std::holds_alternative<ast::AttrFeature>(feature->_base))
        {
            SEMANT_RETURN_IF_FALSE(check_expression_in_attribute(feature, scope), false);
        }
        else
        {
            SEMANT_RETURN_IF_FALSE(check_expression_in_method(feature, scope), false);
        }
    }

    // 3. Check childs with parent scope
    for (const auto &child : node->_children)
    {
        SEMANT_RETURN_IF_FALSE(check_expressions_in_class(child, scope), false);
    }

    _current_class = prev_class; // restore self type
    scope.pop_scope();

    SEMANT_FULL_VERBOSE_ONLY(log_exit("check class " + node->_class->_type->_string));
    return true;
}

bool Semant::check_expressions()
{
    Scope scope;
    scope.push_scope();

    scope.add("self", _self_type);
    SEMANT_RETURN_IF_FALSE(check_expressions_in_class(_root, scope), false);

    scope.pop_scope();
    return true;
}

// -------------------------------------- Infer Expression Type --------------------------------------

bool Semant::infer_expression_type(std::shared_ptr<ast::Expression> &expr, Scope &scope)
{
    expr->_type = std::visit(ast::overloaded{[&](ast::BoolExpression &bool_expr) -> std::shared_ptr<ast::Type> {
                                                 return _bool;
                                             },
                                             [&](ast::StringExpression &str) -> std::shared_ptr<ast::Type> {
                                                 return _string;
                                             },
                                             [&](ast::IntExpression &number) -> std::shared_ptr<ast::Type> {
                                                 return _int;
                                             },
                                             [&](ast::ObjectExpression &object) -> std::shared_ptr<ast::Type> {
                                                 return infer_object_type(object, scope);
                                             },
                                             [&](ast::BinaryExpression &binary_expr) -> std::shared_ptr<ast::Type> {
                                                 return infer_binary_type(binary_expr, scope);
                                             },
                                             [&](ast::UnaryExpression &unary_expr) -> std::shared_ptr<ast::Type> {
                                                 return infer_unary_type(unary_expr, scope);
                                             },
                                             [&](ast::NewExpression &new_) -> std::shared_ptr<ast::Type> {
                                                 return infer_new_type(new_);
                                             },
                                             [&](ast::CaseExpression &case_) -> std::shared_ptr<ast::Type> {
                                                 return infer_cases_type(case_, scope);
                                             },
                                             [&](ast::LetExpression &let) -> std::shared_ptr<ast::Type> {
                                                 return infer_let_type(let, scope);
                                             },
                                             [&](ast::ListExpression &list) -> std::shared_ptr<ast::Type> {
                                                 return infer_sequence_type(list, scope);
                                             },
                                             [&](ast::WhileExpression &while_) -> std::shared_ptr<ast::Type> {
                                                 return infer_loop_type(while_, scope);
                                             },
                                             [&](ast::IfExpression &if_) -> std::shared_ptr<ast::Type> {
                                                 return infer_if_type(if_, scope);
                                             },
                                             [&](ast::DispatchExpression &dispatch) -> std::shared_ptr<ast::Type> {
                                                 return infer_dispatch_type(dispatch, scope);
                                             },
                                             [&](ast::AssignExpression &assign) -> std::shared_ptr<ast::Type> {
                                                 return infer_assign_type(assign, scope);
                                             }},
                             expr->_data);

    if (_error_line_number == -1)
    {
        _error_line_number = expr->_line_number;
    }

    return expr->_type != nullptr;
}

// -------------------------------------- Infer Expression Type Helpers --------------------------------------
std::shared_ptr<ast::Type> Semant::infer_object_type(ast::ObjectExpression &obj, Scope &scope)
{
    const auto expr = scope.find(obj._object);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(expr, "Undeclared identifier " + obj._object + ".", -1, nullptr);

    return expr;
}

std::shared_ptr<ast::Type> Semant::infer_new_type(const ast::NewExpression &new_)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("NEW"));

    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(new_._type), "'new' used with undefined class " + new_._type->_string + ".", -1, nullptr);

    SEMANT_FULL_VERBOSE_ONLY(log_exit("NEW"));
    return new_._type;
}

std::shared_ptr<ast::Type> Semant::infer_let_type(ast::LetExpression &let, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("LET"));

    // type if defined
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(let._type),
                                      "Class " + let._type->_string + " of let-bound identifier " + let._object->_object + " is undefined.",
                                      -1, nullptr);

    if (let._expr)
    {
        SEMANT_RETURN_IF_FALSE(infer_expression_type(let._expr, scope), nullptr);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(let._expr->_type, let._type),
                                          "Inferred type " + let._expr->_type->_string + " of initialization of " + let._object->_object +
                                              " does not conform to identifier's declared type " + let._type->_string + ".",
                                          -1, nullptr);
    }

    scope.push_scope();
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(scope.add_if_can(let._object->_object, let._type) == Scope::ADD_RESULT::OK,
                                      "'" + let._object->_object + "' cannot be bound in a 'let' expression.", -1, nullptr);
    SEMANT_RETURN_IF_FALSE(infer_expression_type(let._body_expr, scope), nullptr);

    scope.pop_scope();

    SEMANT_FULL_VERBOSE_ONLY(log_exit("LET"));
    return let._body_expr->_type;
}

std::shared_ptr<ast::Type> Semant::infer_loop_type(ast::WhileExpression &loop, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("LOOP"));

    SEMANT_RETURN_IF_FALSE(infer_expression_type(loop._predicate, scope), nullptr);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_bool(loop._predicate->_type), "Loop condition does not have type Bool.", -1, nullptr);

    SEMANT_RETURN_IF_FALSE(infer_expression_type(loop._body_expr, scope), nullptr);

    SEMANT_FULL_VERBOSE_ONLY(log_exit("LOOP"));
    return _object;
}

std::shared_ptr<ast::Type> Semant::infer_unary_type(ast::UnaryExpression &unary, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("UNARY"));

    SEMANT_RETURN_IF_FALSE(infer_expression_type(unary._expr, scope), nullptr);

    const auto result = std::visit(ast::overloaded{[&](const ast::IsVoidExpression &isvoid) -> std::shared_ptr<ast::Type> {
                                                       return _bool;
                                                   },
                                                   [&](const ast::NotExpression &not_) -> std::shared_ptr<ast::Type>
                                                   {
                                                       SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_bool(unary._expr->_type),
                                                                                         "Argument of 'not' has type " + unary._expr->_type->_string + " instead of Bool.",
                                                                                         -1, nullptr);
                                                       return _bool;
                                                   },
                                                   [&](const ast::NegExpression &neg) -> std::shared_ptr<ast::Type>
                                                   {
                                                       SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_int(unary._expr->_type),
                                                                                         "Argument of '~' has type " + unary._expr->_type->_string + " instead of Int.",
                                                                                         -1, nullptr);
                                                       return _int;
                                                   }},
                                   unary._base);

    SEMANT_RETURN_IF_FALSE(result, nullptr);

    SEMANT_FULL_VERBOSE_ONLY(log_exit("UNARY"));
    return result;
}

std::shared_ptr<ast::Type> Semant::infer_binary_type(ast::BinaryExpression &binary, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("BINARY"));

    std::shared_ptr<ast::Type> result;
    SEMANT_RETURN_IF_FALSE(infer_expression_type(binary._lhs, scope), nullptr);
    SEMANT_RETURN_IF_FALSE(infer_expression_type(binary._rhs, scope), nullptr);

    if (!std::holds_alternative<ast::EqExpression>(binary._base))
    {
        std::string op;
        std::visit(ast::overloaded{[&](const ast::MinusExpression &minus)
                                   {
                                       result = _int;
                                       op = "-";
                                   },
                                   [&](const ast::PlusExpression &plus)
                                   {
                                       result = _int;
                                       op = "+";
                                   },
                                   [&](const ast::DivExpression &div)
                                   {
                                       result = _int;
                                       op = "/";
                                   },
                                   [&](const ast::MulExpression &mul)
                                   {
                                       result = _int;
                                       op = "*";
                                   },
                                   [&](const ast::LTExpression &lt)
                                   {
                                       result = _bool;
                                       op = "<";
                                   },
                                   [&](const ast::LEExpression &le)
                                   {
                                       result = _bool;
                                       op = "<=";
                                   },
                                   [&](const ast::EqExpression &le)
                                   {
                                       SEMANT_FULL_VERBOSE_ONLY(assert(false && ("Impossible binary expression")));
                                   }},
                   binary._base);

        SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_int(binary._lhs->_type) && is_int(binary._rhs->_type),
                                          "non-Int arguments: " + binary._lhs->_type->_string + " " + op + " " + binary._rhs->_type->_string, -1, nullptr);
    }
    else
    {
        if (is_int(binary._lhs->_type) || is_bool(binary._lhs->_type) || is_string(binary._lhs->_type))
        {
            SEMANT_RETURN_IF_FALSE_WITH_ERROR(same_type(binary._lhs->_type, binary._rhs->_type),
                                              "Illegal comparison with a basic type.", -1, nullptr);
        }
        if (is_int(binary._rhs->_type) || is_bool(binary._rhs->_type) || is_string(binary._rhs->_type))
        {
            SEMANT_RETURN_IF_FALSE_WITH_ERROR(same_type(binary._lhs->_type, binary._rhs->_type),
                                              "Illegal comparison with a basic type.", -1, nullptr);
        }
        result = _bool;
    }

    SEMANT_RETURN_IF_FALSE(result, nullptr);

    SEMANT_FULL_VERBOSE_ONLY(log_exit("BINARY"));
    return result;
}

std::shared_ptr<ast::Type> Semant::infer_assign_type(ast::AssignExpression &assign, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("ASSIGN"));

    SEMANT_RETURN_IF_FALSE(infer_expression_type(assign._expr, scope), nullptr);

    std::shared_ptr<ast::Type> var_type = nullptr;
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(var_type = scope.find(assign._object->_object),
                                      "Assignment to undeclared variable " + assign._object->_object + ".", -1, nullptr);

    SEMANT_RETURN_IF_FALSE_WITH_ERROR(assign._object->_object != Scope::_self, "Cannot assign to 'self'.", -1, nullptr);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(assign._expr->_type, var_type),
                                      "Type " + assign._expr->_type->_string + " of assigned expression does not conform to declared type " +
                                          var_type->_string + " of identifier " + assign._object->_object + ".",
                                      -1, nullptr);

    SEMANT_FULL_VERBOSE_ONLY(log_exit("ASSIGN"));
    return assign._expr->_type;
}

std::shared_ptr<ast::Type> Semant::infer_if_type(ast::IfExpression &if_, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("IF"));

    SEMANT_RETURN_IF_FALSE(infer_expression_type(if_._predicate, scope), nullptr);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_bool(if_._predicate->_type), "Predicate of 'if' does not have type Bool.", -1, nullptr);

    SEMANT_RETURN_IF_FALSE(infer_expression_type(if_._true_path_expr, scope), nullptr);
    SEMANT_RETURN_IF_FALSE(infer_expression_type(if_._false_path_expr, scope), nullptr);

    SEMANT_FULL_VERBOSE_ONLY(log_exit("IF"));
    return find_common_ancestor({if_._true_path_expr->_type, if_._false_path_expr->_type});
}

std::shared_ptr<ast::Type> Semant::infer_sequence_type(ast::ListExpression &seq, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("SEQUENCE"));

    for (auto &expr : seq._exprs)
    {
        SEMANT_RETURN_IF_FALSE(infer_expression_type(expr, scope), nullptr);
    }

    SEMANT_FULL_VERBOSE_ONLY(log_exit("SEQUENCE"));
    return seq._exprs.back()->_type;
}

std::shared_ptr<ast::Type> Semant::infer_cases_type(ast::CaseExpression &cases, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("CASE"));

    SEMANT_RETURN_IF_FALSE(infer_expression_type(cases._expr, scope), nullptr);

    std::vector<std::shared_ptr<ast::Type>> meet_types;
    std::vector<std::shared_ptr<ast::Type>> classes;
    for (auto &case_ : cases._cases)
    {
        scope.push_scope();
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(scope.add_if_can(case_->_object->_object, case_->_type) == Scope::OK,
                                          "'" + case_->_object->_object + "' bound in 'case'.", case_->_line_number, nullptr);

        // check if we already seen such a branch
        bool seen = (std::find_if(meet_types.begin(), meet_types.end(), [&](const std::shared_ptr<ast::Type> &t)
                                  { return same_type(t, case_->_type); }) == meet_types.end());
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(seen, "Duplicate branch " + case_->_type->_string + " in case statement.", -1, nullptr);
        meet_types.push_back(case_->_type);

        SEMANT_RETURN_IF_FALSE(infer_expression_type(case_->_expr, scope), nullptr);

        scope.pop_scope();
        classes.push_back(case_->_expr->_type);
    }

    SEMANT_FULL_VERBOSE_ONLY(log_exit("CASE"));
    return find_common_ancestor(classes);
}

std::shared_ptr<ast::Type> Semant::infer_dispatch_type(ast::DispatchExpression &disp, Scope &scope)
{
    SEMANT_FULL_VERBOSE_ONLY(log_enter("DISPATCH"));

    bool is_static = std::holds_alternative<ast::StaticDispatchExpression>(disp._base);

    SEMANT_RETURN_IF_FALSE(infer_expression_type(disp._expr, scope), nullptr);

    auto dispatch_expr_type = disp._expr->_type;
    if (is_static)
    {
        dispatch_expr_type = std::get<ast::StaticDispatchExpression>(disp._base)._type;
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(disp._expr->_type, dispatch_expr_type),
                                          "Expression type " + disp._expr->_type->_string +
                                              " does not conform to declared static dispatch type " + dispatch_expr_type->_string + ".",
                                          -1, nullptr);
    }

    const auto feature = find_method(disp._object->_object, exact_type(dispatch_expr_type), false);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(feature, "Dispatch to undefined method " + disp._object->_object + ".", -1, nullptr);
    const auto method = std::get<ast::MethodFeature>(feature->_base);

    SEMANT_RETURN_IF_FALSE_WITH_ERROR(method._formals.size() == disp._args.size(), "", 0, nullptr);
    for (int i = 0; i < disp._args.size(); i++)
    {
        SEMANT_RETURN_IF_FALSE(infer_expression_type(disp._args[i], scope), nullptr);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(disp._args[i]->_type, method._formals[i]->_type),
                                          "In call of method " + disp._object->_object + ", type " +
                                              disp._args[i]->_type->_string + " of parameter " + method._formals[i]->_object->_object +
                                              " does not conform to declared type " + method._formals[i]->_type->_string + ".",
                                          disp._args[i]->_line_number, nullptr);
    }

    SEMANT_FULL_VERBOSE_ONLY(log_exit("DISPATCH"));
    return same_type(feature->_type, _self_type) ? dispatch_expr_type : feature->_type;
}

bool Semant::check_types_meet(const std::shared_ptr<ast::Type> &t1, const std::shared_ptr<ast::Type> &t2, const bool &exact) const
{
    if (exact && same_type(t2, _self_type))
    {
        return same_type(t1, t2);
    }

    if (same_type(exact_type(t1), exact_type(t2)))
    {
        return true;
    }

    for (const auto &t : _classes.at(t2->_string)->_children)
    {
        if (check_types_meet(t1, t->_class->_type))
        {
            return true;
        }
    }

    return false;
}

std::shared_ptr<ast::Type> Semant::exact_type(const std::shared_ptr<ast::Type> &type) const
{
    return same_type(type, _self_type) ? _current_class : type;
}

std::shared_ptr<ast::Type> Semant::find_common_ancestor(const std::vector<std::shared_ptr<ast::Type>> &classes) const
{
    auto lca = exact_type(classes[0]);
    // if all classes are SELF_TYPE, so LCA is SELF_TYPE
    bool all_self_type = same_type(_self_type, classes[0]);

    for (int i = 1; i < classes.size(); i++)
    {
        lca = find_common_ancestor_of_two(lca, exact_type(classes[i]));
        if (all_self_type)
        {
            all_self_type = same_type(_self_type, classes[i]);
        }
        if (same_type(lca, _object))
        {
            break;
        }
    }

    return all_self_type ? _self_type : lca;
}

std::shared_ptr<ast::Type> Semant::find_common_ancestor_of_two(const std::shared_ptr<ast::Type> &t1, const std::shared_ptr<ast::Type> &t2) const
{
    int h1 = 0, h2 = 0;

    auto t1_par = t1;
    auto t2_par = t2;

    // find depth of the t1
    while (!same_type(_classes.at(t1_par->_string)->_class->_type, _object))
    {
        h1++;
        t1_par = _classes.at(t1_par->_string)->_class->_parent;
    }

    // find depth of the t2
    while (!same_type(_classes.at(t2_par->_string)->_class->_type, _object))
    {
        h2++;
        t2_par = _classes.at(t2_par->_string)->_class->_parent;
    }

    // align subtrees
    t1_par = t1;
    t2_par = t2;
    while (h1 != h2)
    {
        if (h1 > h2)
        {
            t1_par = _classes.at(t1_par->_string)->_class->_parent;
            h1--;
        }
        else if (h2 > h1)
        {
            t2_par = _classes.at(t2_par->_string)->_class->_parent;
            h2--;
        }
    }

    // find lca
    while (!same_type(t1_par, t2_par))
    {
        t1_par = _classes.at(t1_par->_string)->_class->_parent;
        t2_par = _classes.at(t2_par->_string)->_class->_parent;
    }

    return t1_par;
}

std::shared_ptr<ast::Feature> Semant::find_method(const std::string &name, const std::shared_ptr<ast::Type> &class_, const bool &exact) const
{
    if (same_type(class_, _object_parent))
    {
        return nullptr;
    }

    std::shared_ptr<ast::Feature> method;

    for (const auto &m : _classes.at(class_->_string)->_class->_features)
    {
        if (std::holds_alternative<ast::MethodFeature>(m->_base) && m->_object->_object == name)
        {
            method = m;
            break;
        }
    }

    if (exact)
    {
        return method;
    }

    // find method in ancestors
    auto current_class = _classes.at(class_->_string)->_class->_parent;
    while (!same_type(current_class, _object_parent))
    {
        for (const auto &m : _classes.at(current_class->_string)->_class->_features)
        {
            if (std::holds_alternative<ast::MethodFeature>(m->_base) && m->_object->_object == name)
            {
                method = m;
                break;
            }
        }
        if (method)
        {
            break;
        }
        else
        {
            current_class = _classes.at(current_class->_string)->_class->_parent;
        }
    }

    return method;
}

std::string Semant::get_error_msg() const
{
    std::string prefix;
    if (_error_line_number != -1)
    {
        prefix = _error_file_name + ":" + std::to_string(_error_line_number) + ": ";
    }
    return prefix + _error_message;
}

#ifdef SEMANT_FULL_VERBOSE
void Scope::dump() const
{
    for (const auto &class_scope : _symbols)
    {
        std::cout << "--------------------------" << std::endl;
        for (const auto &symbol : class_scope)
        {
            std::cout << symbol.first << " = " << symbol.second->_string << std::endl;
        }
    }
}
#endif // SEMANT_FULL_VERBOSE

#ifdef SEMANT_STANDALONE
#include <parser/Parser.h>

int main(int argc, char *argv[])
{
    std::vector<std::shared_ptr<ast::Program>> programs;
    for (int i = 1; i < argc; i++)
    {
        parser::Parser parser(std::make_shared<lexer::Lexer>(argv[i]));
        if (auto ast = parser.parse_program())
        {
            programs.push_back(ast);
        }
        else
        {
            std::cout << parser.get_error_msg() << std::endl;
            return 0;
        }
    }

    Semant semant(programs);
    if (auto ast = semant.infer_types_and_check())
    {
        ast::dump_program(*ast);
    }
    else
    {
        std::cout << semant.get_error_msg() << std::endl;
    }

    return 0;
}
#endif // SEMANT_STANDALONE