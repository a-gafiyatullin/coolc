#include "semant/Semant.h"

using namespace semant;

// ---------------------------------------- INITIALIZATION ----------------------------------------
std::shared_ptr<ast::Program> Semant::merge_to_one_program(const std::vector<std::shared_ptr<ast::Program>> &programs)
{
    if (programs.empty())
    {
        return nullptr;
    }

    const auto program = std::make_shared<ast::Program>();
    program->_line_number = programs.at(0)->_line_number;

    for (const auto &p : programs)
    {
        program->_classes.insert(program->_classes.end(), p->_classes.begin(), p->_classes.end());
    }

    return program;
}

Semant::Semant(std::vector<std::shared_ptr<ast::Program>> programs) : _program(merge_to_one_program(programs)) {}

// ---------------------------------------- CLASS CHECK ----------------------------------------

std::shared_ptr<ClassNode> Semant::make_basic_class(
    const std::string &name, const std::string &parent,
    const std::vector<std::pair<std::string, std::vector<std::string>>> &methods,
    const std::vector<std::shared_ptr<ast::Type>> &fields)
{
    const auto klass = std::make_shared<ClassNode>();
    klass->_class = std::make_shared<ast::Class>();
    klass->_class->_type = std::make_shared<ast::Type>();
    klass->_class->_parent = std::make_shared<ast::Type>();
    klass->_class->_expression_stack = 0;

    klass->_class->_type->_string = name;
    klass->_class->_parent->_string = parent;
    for (const auto &m : methods)
    {
        const auto feature = std::make_shared<ast::Feature>();
        feature->_base = ast::MethodFeature();

        // method name
        feature->_object = std::make_shared<ast::ObjectExpression>();
        feature->_object->_object = m.first;

        // method ret type
        GUARANTEE_DEBUG(!methods.empty());
        feature->_type = std::make_shared<ast::Type>();
        feature->_type->_string = m.second.front();

        auto &method = std::get<ast::MethodFeature>(feature->_base);
        method._expression_stack = 0;

        // method args
        for (auto i = 1; i < m.second.size(); i++)
        {
            // formal name
            method._formals.push_back(std::make_shared<ast::Formal>());
            method._formals.back()->_object = std::make_shared<ast::ObjectExpression>();
            method._formals.back()->_object->_object = static_cast<std::string>(DUMMY_ARG_SUFFIX) + std::to_string(i);

            // formal type
            method._formals.back()->_type = std::make_shared<ast::Type>();
            method._formals.back()->_type->_string = m.second[i];
        }

        klass->_class->_features.push_back(feature);
    }

    // add fields
    static auto N = 0;
    for (const auto &f : fields)
    {
        const auto feature = std::make_shared<ast::Feature>();
        feature->_base = ast::AttrFeature();

        feature->_object = std::make_shared<ast::ObjectExpression>();
        feature->_type = f;
        feature->_object->_object = static_cast<std::string>(DUMMY_FIELD_SUFFIX) + std::to_string(N);

        klass->_class->_features.push_back(feature);

        N++;
    }

    _classes.insert({name, klass});
    return klass;
}

bool Semant::check_class_hierarchy_for_cycle(const std::shared_ptr<ClassNode> &klass,
                                             std::unordered_map<std::string, int> &visited, const int &loop)
{
    const auto &class_name = klass->_class->_type->_string;

    if (visited[class_name] == loop)
    {
        return false;
    }

    visited[class_name] = loop;

    for (const auto &child : klass->_children)
    {
        SEMANT_RETURN_IF_FALSE(check_class_hierarchy_for_cycle(child, visited, loop), false);
        visited[class_name] = false;
    }

    return true;
}

bool Semant::is_basic_type(const std::shared_ptr<ast::Type> &type)
{
    return is_int(type) || is_bool(type) || is_string(type) || same_type(type, Object) || same_type(type, Io) ||
           is_self_type(type) || is_empty_type(type) || same_type(type, Empty);
}

bool Semant::is_trivial_type(const std::shared_ptr<ast::Type> &type)
{
    return is_int(type) || is_bool(type) || is_string(type);
}

bool Semant::is_inherit_allowed(const std::shared_ptr<ast::Type> &type)
{
    return !(is_int(type) || is_bool(type) || is_string(type) || is_self_type(type) || is_empty_type(type));
}

bool Semant::is_native_type(const std::shared_ptr<ast::Type> &type)
{
    return is_native_int(type) || is_native_bool(type) || is_native_string(type);
}

bool Semant::check_class_hierarchy()
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("CHECK CLASS HIERARCHY"));

    std::vector<std::shared_ptr<ClassNode>> delayed_parent;
    for (const auto &klass : _program->_classes)
    {
        _error_file_name = klass->_file_name; // for error
        const auto &class_name = klass->_type->_string;

        SEMANT_RETURN_IF_FALSE_WITH_ERROR(!is_basic_type(klass->_type),
                                          "Redefinition of basic class " + class_name + ".", klass->_line_number,
                                          false);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(_classes.find(class_name) == _classes.end(),
                                          "Class " + class_name + " was previously defined.", klass->_line_number,
                                          false);

        const auto new_class = std::make_shared<ClassNode>();
        new_class->_class = klass;
        _classes.insert({class_name, new_class});

        const auto parent = _classes.find(klass->_parent->_string);
        if (parent == _classes.end())
        {
            delayed_parent.push_back(new_class);
        }
        else
        {
            const auto &class_type = parent->second->_class->_type;

            SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_inherit_allowed(class_type),
                                              "Class " + class_name + " cannot inherit class " + class_type->_string +
                                                  ".",
                                              klass->_line_number, false);
            parent->second->_children.push_back(new_class);
        }
    }

    for (const auto &klass : delayed_parent)
    {
        _error_file_name = klass->_class->_file_name; // for error
        const auto &parent_class_name = klass->_class->_parent->_string;

        const auto parent = _classes.find(parent_class_name);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(parent != _classes.end(),
                                          "Class " + klass->_class->_type->_string +
                                              " inherits from an undefined class " + parent_class_name + ".",
                                          klass->_class->_line_number, false);
        parent->second->_children.push_back(klass);
    }

    std::unordered_map<std::string, int> visited;
    for (const auto &klass : _classes)
    {
        visited.insert({klass.first, -1});
    }

    auto loop_num = 0;
    auto cycle_num = 1;
    for (const auto &klass : _classes)
    {
        if (visited[klass.first] != -1)
        {
            continue;
        }
        // format error
        if (!check_class_hierarchy_for_cycle(klass.second, visited, loop_num))
        {
            _error_line_number = -1;
            _error_message += "Cycle " + std::to_string(cycle_num) + ":\n";
            cycle_num++;
            for (const auto &klass : visited)
            {
                if (klass.second == loop_num)
                {
                    const auto &class_node = _classes[klass.first]->_class;
                    const auto &class_name = class_node->_type->_string;

                    _error_message += class_node->_file_name + ":" + std::to_string(class_node->_line_number) +
                                      " Class " + class_name + " or an ancestor of " + class_name +
                                      " is involved in an inheritance cycle.\n";
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

    SEMANT_VERBOSE_ONLY(LOG_EXIT("CHECK CLASS HIERARCHY"));
    return true;
}

bool Semant::check_main()
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("CHECK MAIN"));

    auto found_main = false;

    const auto main_class = _classes.find(MainClassName);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(main_class != _classes.end(), "Class Main is not defined.", -1, false);

    for (const auto &feature : main_class->second->_class->_features)
    {
        if (std::holds_alternative<ast::MethodFeature>(feature->_base))
        {
            // main method's name main and 0 parameters
            // further we check if it is an only such method
            if (feature->_object->_object == MainMethodName &&
                std::get<ast::MethodFeature>(feature->_base)._formals.size() == 0)
            {
                found_main = true;
                break;
            }
        }
    }

    SEMANT_RETURN_IF_FALSE_WITH_ERROR(found_main, "No 'main' method in class Main.",
                                      _classes[MainClassName]->_class->_line_number, false);

    SEMANT_VERBOSE_ONLY(LOG_EXIT("CHECK MAIN"));
    return true;
}

bool Semant::check_classes()
{
    // 1. We can inherit classes only from Object, IO and user defined classes
    // add Object to hierarchy
    SEMANT_VERBOSE_ONLY(LOG_ENTER("CREATE BASIC CLASSES"));

    Empty = std::make_shared<ast::Type>();
    Empty->_string = EMPTY_TYPE_NAME;

    NativeInt = std::make_shared<ast::Type>();
    NativeInt->_string = NATIVE_INT_TYPE_NAME;

    NativeBool = std::make_shared<ast::Type>();
    NativeBool->_string = NATIVE_BOOL_TYPE_NAME;

    NativeString = std::make_shared<ast::Type>();
    NativeString->_string = NATIVE_STRING_TYPE_NAME;

    _root = make_basic_class(BaseClassesNames[BaseClasses::OBJECT], Empty->_string,
                             {{ObjectMethodsNames[ObjectMethods::ABORT], {BaseClassesNames[BaseClasses::OBJECT]}},
                              {ObjectMethodsNames[ObjectMethods::TYPE_NAME], {BaseClassesNames[BaseClasses::STRING]}},
                              {ObjectMethodsNames[ObjectMethods::COPY], {BaseClassesNames[BaseClasses::SELF_TYPE]}}},
                             {});
    Object = _root->_class->_type;

    // add IO to hierarchy
    _root->_children.push_back(
        make_basic_class(BaseClassesNames[BaseClasses::IO], BaseClassesNames[BaseClasses::OBJECT],
                         {{IOMethodsNames[IOMethods::OUT_STRING],
                           {BaseClassesNames[BaseClasses::SELF_TYPE], BaseClassesNames[BaseClasses::STRING]}},
                          {IOMethodsNames[IOMethods::OUT_INT],
                           {BaseClassesNames[BaseClasses::SELF_TYPE], BaseClassesNames[BaseClasses::INT]}},
                          {IOMethodsNames[IOMethods::IN_STRING], {BaseClassesNames[BaseClasses::STRING]}},
                          {IOMethodsNames[IOMethods::IN_INT], {BaseClassesNames[BaseClasses::INT]}}},
                         {}));
    Io = _root->_children.back()->_class->_type;

    // add Int to hierarchy
    _root->_children.push_back(
        make_basic_class(BaseClassesNames[BaseClasses::INT], BaseClassesNames[BaseClasses::OBJECT], {}, {NativeInt}));
    Int = _root->_children.back()->_class->_type;

    // add Bool to hierarchy
    _root->_children.push_back(
        make_basic_class(BaseClassesNames[BaseClasses::BOOL], BaseClassesNames[BaseClasses::OBJECT], {}, {NativeBool}));
    Bool = _root->_children.back()->_class->_type;

    // add SELF_TYPE to hierarchy
    _root->_children.push_back(
        make_basic_class(BaseClassesNames[BaseClasses::SELF_TYPE], BaseClassesNames[BaseClasses::OBJECT], {}, {}));
    SelfType = _root->_children.back()->_class->_type;

    // add String to hierarchy
    _root->_children.push_back(
        make_basic_class(BaseClassesNames[BaseClasses::STRING], BaseClassesNames[BaseClasses::OBJECT],
                         {{StringMethodsNames[StringMethods::LENGTH], {BaseClassesNames[BaseClasses::INT]}},
                          {StringMethodsNames[StringMethods::CONCAT],
                           {BaseClassesNames[BaseClasses::STRING], BaseClassesNames[BaseClasses::STRING]}},
                          {StringMethodsNames[StringMethods::SUBSTR],
                           {BaseClassesNames[BaseClasses::STRING], BaseClassesNames[BaseClasses::INT],
                            BaseClassesNames[BaseClasses::INT]}}},
                         {Int, NativeString}));
    String = _root->_children.back()->_class->_type;

    SEMANT_VERBOSE_ONLY(LOG_EXIT("CREATE BASIC CLASSES"));

    // 2. Add user defined classes to hierarchy
    SEMANT_RETURN_IF_FALSE(check_class_hierarchy(), false);

    // 3. Check main method in Main class
    SEMANT_RETURN_IF_FALSE(check_main(), false);

    return true;
}

// ---------------------------------------- TYPES CHECK ----------------------------------------

std::pair<std::shared_ptr<ClassNode>, std::shared_ptr<ast::Program>> Semant::infer_types_and_check()
{
    SEMANT_RETURN_IF_FALSE(check_classes(), std::make_pair(nullptr, nullptr));
    SEMANT_RETURN_IF_FALSE(check_expressions(), std::make_pair(nullptr, nullptr));

    // now we are not interested in SELF_TYPE in class hierarchy
    _root->_children.erase(std::remove_if(_root->_children.begin(), _root->_children.end(),
                                          [](const auto &klass) { return is_self_type(klass->_class->_type); }));

    return std::make_pair(_root, _program);
}

bool Semant::check_expression_in_method(const std::shared_ptr<ast::Feature> &feature, Scope &scope)
{
    const auto &name = feature->_object->_object;
    const auto &ret_type_name = feature->_type->_string;

    SEMANT_VERBOSE_ONLY(LOG_ENTER("CHECK METHOD \"" + name + "\""));

    scope.push_scope();

    auto &this_method = std::get<ast::MethodFeature>(feature->_base);
    const auto &klass = _classes[_current_class->_string]->_class;

    _expression_stack = 0;

    // return type is defined
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(feature->_type),
                                      "Undefined return type " + ret_type_name + " in method " + name + ".",
                                      feature->_line_number, false);

    // multiple defined in one class
    const bool is_ones_defined =
        std::count_if(klass->_features.begin(), klass->_features.end(), [&feature, &name](const auto &m) {
            return (std::holds_alternative<ast::MethodFeature>(m->_base) && m->_object->_object == name);
        }) == 1;
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_ones_defined, "Method " + name + " is multiply defined.",
                                      feature->_line_number, false);

    // parent method
    const auto parent_feature = find_method(name, klass->_type, false);
    ast::MethodFeature parent_method;

    if (parent_feature)
    {
        parent_method = std::get<ast::MethodFeature>(parent_feature->_base);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(this_method._formals.size() == parent_method._formals.size(),
                                          "Incompatible number of formal parameters in redefined method " + name + ".",
                                          feature->_line_number, false);
    }

    auto formal_num = 0;
    // add formal parameters to scope and check method overriding
    for (const auto &formal : this_method._formals)
    {
        const auto &formal_name = formal->_object->_object;
        const auto &formal_type_name = formal->_type->_string;

        // formal type if defined and not SELF_TYPE
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(!is_self_type(formal->_type),
                                          "Formal parameter " + formal_name + " cannot have type SELF_TYPE.",
                                          formal->_line_number, false);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(formal->_type),
                                          "Class " + formal_type_name + " of formal parameter " + formal_name +
                                              " is undefined.",
                                          formal->_line_number, false);

        const auto result = scope.add_if_can(formal_name, formal->_type);
        if (result != Scope::AddResult::OK)
        {
            _error_line_number = formal->_line_number;

            if (result == Scope::AddResult::REDEFINED)
            {
                _error_message = "Formal parameter " + formal_name + " is multiply defined.";
            }
            else if (result == Scope::AddResult::RESERVED)
            {
                _error_message = "'" + formal_name + "' cannot be the name of a formal parameter.";
            }

            return false;
        }

        if (parent_feature)
        {
            const auto &original_type = parent_method._formals[formal_num]->_type;

            SEMANT_RETURN_IF_FALSE_WITH_ERROR(same_type(original_type, formal->_type),
                                              "In redefined method " + name + ", parameter type " + formal_type_name +
                                                  " is different from original type " + original_type->_string,
                                              formal->_line_number, false);
        }

        formal_num++;
    }

    if (feature->_expr)
    {
        const auto &body_type = feature->_expr->_type;

        SEMANT_RETURN_IF_FALSE(infer_expression_type(feature->_expr, scope), false);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(body_type, feature->_type),
                                          "Inferred return type " + body_type->_string + " of method " + name +
                                              " does not conform to declared return type " + ret_type_name + ".",
                                          feature->_line_number, false);
    }

    scope.pop_scope();

    // slots for args
    this_method._expression_stack = _expression_stack;

    SEMANT_VERBOSE_ONLY(LOG_EXIT("CHECK METHOD \"" + name + "\", max stack = " + std::to_string(_expression_stack)));
    return true;
}

bool Semant::check_expression_in_attribute(const std::shared_ptr<ast::Feature> &attr, Scope &scope)
{
    const auto &name = attr->_object->_object;
    const auto &type = attr->_type;

    SEMANT_VERBOSE_ONLY(LOG_ENTER("CHECK ATTRIBUTE \"" + name + "\""));

    // type if defined
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(type),
                                      "Class " + type->_string + " of attribute " + name + " is undefined.",
                                      attr->_line_number, false);

    if (attr->_expr)
    {
        const auto &expr_type = attr->_expr->_type;

        SEMANT_RETURN_IF_FALSE(infer_expression_type(attr->_expr, scope), false);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(expr_type, type),
                                          "Inferred type " + expr_type->_string +
                                              " of initialization of attribute a does not conform to declared type " +
                                              type->_string + ".",
                                          -1, false);
    }

    SEMANT_VERBOSE_ONLY(LOG_EXIT("CHECK ATTRIBUTE \"" + name + "\""));
    return true;
}

bool Semant::check_expressions_in_class(const std::shared_ptr<ClassNode> &node, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("CHECK CLASS \"" + node->_class->_type->_string + "\""));
    _error_file_name = node->_class->_file_name; // save for error messages

    scope.push_scope();

    const auto prev_class = _current_class;
    _current_class = node->_class->_type; // current SELF_TYPE

    // Don't check basic classes
    if (!is_basic_type(_current_class))
    {
        // 1. Add class attributes to current scope and check if redefined
        for (const auto &feature : node->_class->_features)
        {
            const auto &feature_name = feature->_object->_object;
            if (std::holds_alternative<ast::AttrFeature>(feature->_base))
            {
                const auto result = scope.add_if_can(feature_name, feature->_type);
                _error_line_number = feature->_line_number;
                if (result == Scope::RESERVED)
                {
                    _error_message = "\'" + feature_name + "\' cannot be the name of an attribute.";
                }
                else if (result == Scope::REDEFINED)
                {
                    _error_message = "Attribute " + feature_name + " is multiply defined in class.";
                }

                if (result != Scope::OK)
                {
                    return false;
                }

                // check if attribute is inherited from parent or redefined
                const auto parent_attr = scope.find(feature_name, 1);
                SEMANT_RETURN_IF_FALSE_WITH_ERROR(
                    !parent_attr, "Attribute " + feature_name + " is an attribute of an inherited class.",
                    feature->_line_number, false);
            }
        }

        // 2. Check attributes types
        for (const auto &feature : node->_class->_features)
        {
            if (std::holds_alternative<ast::AttrFeature>(feature->_base))
            {
                _expression_stack = 0;
                SEMANT_RETURN_IF_FALSE(check_expression_in_attribute(feature, scope), false);
                node->_class->_expression_stack = std::max(_expression_stack, node->_class->_expression_stack);
            }
            else
            {
                SEMANT_RETURN_IF_FALSE(check_expression_in_method(feature, scope), false);
            }
        }

        SEMANT_VERBOSE_ONLY(LOG("Init method max stack = " + std::to_string(node->_class->_expression_stack)));
    }

    // 3. Check childs with parent scope
    for (const auto &child : node->_children)
    {
        SEMANT_RETURN_IF_FALSE(check_expressions_in_class(child, scope), false);
    }

    _current_class = prev_class; // restore self type
    scope.pop_scope();

    SEMANT_VERBOSE_ONLY(LOG_EXIT("CHECK CLASS \"" + node->_class->_type->_string + "\""));
    return true;
}

bool Semant::check_expressions()
{
    Scope scope(SelfType);

    SEMANT_RETURN_IF_FALSE(check_expressions_in_class(_root, scope), false);

    scope.pop_scope();
    return true;
}

// -------------------------------------- Infer Expression Type --------------------------------------

bool Semant::infer_expression_type(const std::shared_ptr<ast::Expression> &expr, Scope &scope)
{
    expr->_type = std::visit(
        ast::overloaded{[&](const ast::BoolExpression &bool_expr) { return Bool; },
                        [&](const ast::StringExpression &str) { return String; },
                        [&](const ast::IntExpression &number) { return Int; },
                        [&](const ast::ObjectExpression &object) { return infer_object_type(object, scope); },
                        [&](const ast::BinaryExpression &binary_expr) {
                            expr->_can_allocate = true;
                            return infer_binary_type(binary_expr, scope);
                        },
                        [&](const ast::UnaryExpression &unary_expr) {
                            const auto res = infer_unary_type(unary_expr, scope);
                            expr->_can_allocate = std::holds_alternative<ast::NegExpression>(unary_expr._base) ||
                                                  unary_expr._expr->_can_allocate;
                            return res;
                        },
                        [&](const ast::NewExpression &alloc) {
                            expr->_can_allocate = true;
                            return infer_new_type(alloc);
                        },
                        [&](const ast::CaseExpression &branch) {
                            const auto res = infer_cases_type(branch, scope);
                            expr->_can_allocate = branch._expr->_can_allocate;
                            expr->_can_allocate |= std::any_of(
                                branch._cases.begin(), branch._cases.end(),
                                [](const std::shared_ptr<ast::Case> &c) { return c->_expr->_can_allocate; });
                            return res;
                        },
                        [&](const ast::LetExpression &let) {
                            const auto res = infer_let_type(let, scope);
                            if (let._expr)
                            {
                                expr->_can_allocate = let._expr->_can_allocate;
                            }
                            expr->_can_allocate |= let._body_expr->_can_allocate;
                            return res;
                        },
                        [&](const ast::ListExpression &list) {
                            const auto res = infer_sequence_type(list, scope);
                            expr->_can_allocate =
                                std::any_of(list._exprs.begin(), list._exprs.end(),
                                            [](const std::shared_ptr<ast::Expression> &c) { return c->_can_allocate; });
                            return res;
                        },
                        [&](const ast::WhileExpression &loop) {
                            const auto res = infer_loop_type(loop, scope);
                            expr->_can_allocate = loop._body_expr->_can_allocate | loop._predicate->_can_allocate;
                            return res;
                        },
                        [&](const ast::IfExpression &branch) {
                            const auto res = infer_if_type(branch, scope);
                            expr->_can_allocate = branch._false_path_expr->_can_allocate |
                                                  branch._true_path_expr->_can_allocate |
                                                  branch._predicate->_can_allocate;
                            return res;
                        },
                        [&](const ast::DispatchExpression &dispatch) {
                            expr->_can_allocate = true;
                            return infer_dispatch_type(dispatch, scope);
                        },
                        [&](const ast::AssignExpression &assign) {
                            const auto res = infer_assign_type(assign, scope);
                            expr->_can_allocate = assign._expr->_can_allocate;
                            return res;
                        }},
        expr->_data);

    if (_error_line_number == -1)
    {
        _error_line_number = expr->_line_number;
    }

    return expr->_type != nullptr;
}

// -------------------------------------- Infer Expression Type Helpers --------------------------------------
std::shared_ptr<ast::Type> Semant::infer_object_type(const ast::ObjectExpression &obj, Scope &scope)
{
    const auto expr = scope.find(obj._object);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(expr, "Undeclared identifier " + obj._object + ".", -1, nullptr);

    return expr;
}

std::shared_ptr<ast::Type> Semant::infer_new_type(const ast::NewExpression &alloc)
{
    const auto &type = alloc._type;

#ifdef LLVM_SHADOW_STACK
    _expression_stack++; // need at least one slot, because init can cause GC

#endif // LLVM_SHADOW_STACK

    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER NEW TYPE"));

    // type if defined
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(type), "'new' used with undefined class " + type->_string + ".", -1,
                                      nullptr);

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER NEW TYPE"));
    return type;
}

std::shared_ptr<ast::Type> Semant::infer_let_type(const ast::LetExpression &let, Scope &scope)
{
    const auto &var_type = let._type;
    const auto &var_name = let._object->_object;

    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER LET TYPE"));

    // type if defined
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(
        check_exists(var_type),
        "Class " + var_type->_string + " of let-bound identifier " + var_name + " is undefined.", -1, nullptr);

    int let_expr_stack = _expression_stack;
    int let_body_stack = _expression_stack + 1;
    if (let._expr)
    {
        const auto &init_type = let._expr->_type;

        SEMANT_RETURN_IF_FALSE(infer_expression_type(let._expr, scope), nullptr);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(init_type, var_type),
                                          "Inferred type " + init_type->_string + " of initialization of " + var_name +
                                              " does not conform to identifier's declared type " + var_type->_string +
                                              ".",
                                          -1, nullptr);
        let_expr_stack = _expression_stack;
    }

    scope.push_scope();
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(scope.add_if_can(var_name, var_type) == Scope::AddResult::OK,
                                      "'" + var_name + "' cannot be bound in a 'let' expression.", -1, nullptr);

    _expression_stack = let_body_stack;
    SEMANT_RETURN_IF_FALSE(infer_expression_type(let._body_expr, scope), nullptr);
    let_body_stack = _expression_stack;

    // let doesn't create a new value
    _expression_stack = std::max(let_expr_stack, let_body_stack);

    scope.pop_scope();

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER LET TYPE"));
    return let._body_expr->_type;
}

std::shared_ptr<ast::Type> Semant::infer_loop_type(const ast::WhileExpression &loop, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER LOOP TYPE"));

    int predicate_expr_stack = _expression_stack;
    int body_expr_stack = _expression_stack;

    SEMANT_RETURN_IF_FALSE(infer_expression_type(loop._predicate, scope), nullptr);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_bool(loop._predicate->_type), "Loop condition does not have type Bool.", -1,
                                      nullptr);
    predicate_expr_stack = _expression_stack;

    _expression_stack = body_expr_stack;
    SEMANT_RETURN_IF_FALSE(infer_expression_type(loop._body_expr, scope), nullptr);
    body_expr_stack = _expression_stack;

    // loop doesn't ceate a new value
    _expression_stack = std::max(predicate_expr_stack, body_expr_stack);

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER LOOP TYPE"));
    return Object;
}

std::shared_ptr<ast::Type> Semant::infer_unary_type(const ast::UnaryExpression &unary, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER UNARY TYPE"));

    SEMANT_RETURN_IF_FALSE(infer_expression_type(unary._expr, scope), nullptr);

    const auto &expr_type = unary._expr->_type;
    const auto result =
        std::visit(ast::overloaded{[&](const ast::IsVoidExpression &isvoid) { return Bool; },
                                   [&](const ast::NotExpression &) {
                                       SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_bool(expr_type),
                                                                         "Argument of 'not' has type " +
                                                                             expr_type->_string + " instead of Bool.",
                                                                         -1, (std::shared_ptr<ast::Type>)nullptr);
                                       return Bool;
                                   },
                                   [&](const ast::NegExpression &neg) {
                                       SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_int(expr_type),
                                                                         "Argument of '~' has type " +
                                                                             expr_type->_string + " instead of Int.",
                                                                         -1, (std::shared_ptr<ast::Type>)nullptr);
                                       return Int;
                                   }},
                   unary._base);

    SEMANT_RETURN_IF_FALSE(result, nullptr);

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER UNARY TYPE"));
    return result;
}

std::shared_ptr<ast::Type> Semant::infer_binary_type(const ast::BinaryExpression &binary, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER BINARY TYPE"));

    int lhs_expr_stack = _expression_stack;

#ifdef LLVM_SHADOW_STACK
    int rhs_expr_stack = _expression_stack + 1;
#else
    int rhs_expr_stack = _expression_stack;
#endif // LLVM_SHADOW_STACK

    std::shared_ptr<ast::Type> result;

    SEMANT_RETURN_IF_FALSE(infer_expression_type(binary._lhs, scope), nullptr);
    lhs_expr_stack = _expression_stack;

    _expression_stack = rhs_expr_stack;
    SEMANT_RETURN_IF_FALSE(infer_expression_type(binary._rhs, scope), nullptr);
    rhs_expr_stack = _expression_stack;

    _expression_stack = std::max(lhs_expr_stack, rhs_expr_stack);

    const auto &lhs_type = binary._lhs->_type;
    const auto &rhs_type = binary._rhs->_type;

    if (!std::holds_alternative<ast::EqExpression>(binary._base))
    {
        std::string op;
        std::visit(ast::overloaded{[&](const ast::MinusExpression &minus) {
                                       result = Int;
                                       op = "-";
                                   },
                                   [&](const ast::PlusExpression &plus) {
                                       result = Int;
                                       op = "+";
                                   },
                                   [&](const ast::DivExpression &div) {
                                       result = Int;
                                       op = "/";
                                   },
                                   [&](const ast::MulExpression &mul) {
                                       result = Int;
                                       op = "*";
                                   },
                                   [&](const ast::LTExpression &lt) {
                                       result = Bool;
                                       op = "<";
                                   },
                                   [&](const ast::LEExpression &le) {
                                       result = Bool;
                                       op = "<=";
                                   },
                                   [&](const ast::EqExpression &le) {

                                   }},
                   binary._base);

        SEMANT_RETURN_IF_FALSE_WITH_ERROR(
            is_int(lhs_type) && is_int(rhs_type),
            "non-Int arguments: " + lhs_type->_string + " " + op + " " + rhs_type->_string, -1, nullptr);
    }
    else
    {
        if (is_int(lhs_type) || is_bool(lhs_type) || is_string(lhs_type))
        {
            SEMANT_RETURN_IF_FALSE_WITH_ERROR(same_type(lhs_type, rhs_type), "Illegal comparison with a basic type.",
                                              -1, nullptr);
        }
        if (is_int(rhs_type) || is_bool(rhs_type) || is_string(rhs_type))
        {
            SEMANT_RETURN_IF_FALSE_WITH_ERROR(same_type(lhs_type, rhs_type), "Illegal comparison with a basic type.",
                                              -1, nullptr);
        }
        result = Bool;
    }

    SEMANT_RETURN_IF_FALSE(result, nullptr);

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER BINARY TYPE"));
    return result;
}

std::shared_ptr<ast::Type> Semant::infer_assign_type(const ast::AssignExpression &assign, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER ASSIGN TYPE"));

    SEMANT_RETURN_IF_FALSE(infer_expression_type(assign._expr, scope), nullptr);

    const auto &var_name = assign._object->_object;
    const auto &expr_type = assign._expr->_type;

    std::shared_ptr<ast::Type> var_type = nullptr;
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(var_type = scope.find(var_name),
                                      "Assignment to undeclared variable " + var_name + ".", -1, nullptr);

    SEMANT_RETURN_IF_FALSE_WITH_ERROR(Scope::can_assign(var_name), "Cannot assign to 'self'.", -1, nullptr);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(expr_type, var_type),
                                      "Type " + expr_type->_string +
                                          " of assigned expression does not conform to declared type " +
                                          var_type->_string + " of identifier " + var_name + ".",
                                      -1, nullptr);

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER ASSIGN TYPE"));
    return expr_type;
}

std::shared_ptr<ast::Type> Semant::infer_if_type(const ast::IfExpression &branch, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER IF TYPE"));

    const auto &pred = branch._predicate;
    const auto &true_branch = branch._true_path_expr;
    const auto &false_branch = branch._false_path_expr;

    int pred_expr_stack = _expression_stack;
    int true_expr_stack = _expression_stack;
    int false_expr_stack = _expression_stack;

    SEMANT_RETURN_IF_FALSE(infer_expression_type(pred, scope), nullptr);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(is_bool(pred->_type), "Predicate of 'if' does not have type Bool.", -1, nullptr);
    pred_expr_stack = _expression_stack;

    _expression_stack = true_expr_stack;
    SEMANT_RETURN_IF_FALSE(infer_expression_type(true_branch, scope), nullptr);
    true_expr_stack = _expression_stack;

    _expression_stack = false_expr_stack;
    SEMANT_RETURN_IF_FALSE(infer_expression_type(false_branch, scope), nullptr);
    false_expr_stack = _expression_stack;

    _expression_stack = std::max(pred_expr_stack, true_expr_stack);
    _expression_stack = std::max(_expression_stack, false_expr_stack);

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER IF TYPE"));
    return find_common_ancestor({true_branch->_type, false_branch->_type});
}

std::shared_ptr<ast::Type> Semant::infer_sequence_type(const ast::ListExpression &seq, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER SEQUENCE TYPE"));

    std::vector<int> stacks(seq._exprs.size(), _expression_stack);
    int i = 0;
    for (const auto &expr : seq._exprs)
    {
        _expression_stack = stacks.at(i);

        SEMANT_RETURN_IF_FALSE(infer_expression_type(expr, scope), nullptr);

        stacks[i] = _expression_stack;
        i++;
    }

    // don't need extra slot, because we don't save any values
    _expression_stack = *std::max_element(stacks.begin(), stacks.end());

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER SEQUENCE TYPE"));
    return seq._exprs.back()->_type;
}

std::shared_ptr<ast::Type> Semant::infer_cases_type(const ast::CaseExpression &cases, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER CASE TYPE"));

    std::vector<int> stacks(cases._cases.size() + 1, _expression_stack);

    SEMANT_RETURN_IF_FALSE(infer_expression_type(cases._expr, scope), nullptr);
    stacks[0] = _expression_stack;

    std::vector<std::shared_ptr<ast::Type>> meet_types;
    std::vector<std::shared_ptr<ast::Type>> classes;
    int i = 1;
    for (const auto &kase : cases._cases)
    {
        scope.push_scope();

        // one additional slot for object
        _expression_stack = stacks.at(i) + 1;

        const auto &var_name = kase->_object->_object;
        const auto &var_type = kase->_type;

        // type if defined and not SELF_TYPE
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(!is_self_type(var_type),
                                          "Identifier " + var_name + " declared with type SELF_TYPE in case branch.",
                                          kase->_line_number, nullptr);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(var_type),
                                          "Class " + var_type->_string + " of case branch is undefined.",
                                          kase->_line_number, nullptr);

        SEMANT_RETURN_IF_FALSE_WITH_ERROR(scope.add_if_can(var_name, var_type) == Scope::OK,
                                          "'" + var_name + "' bound in 'case'.", kase->_line_number, nullptr);

        // check if we already seen such a branch
        const bool seen = (std::find_if(meet_types.begin(), meet_types.end(),
                                        [&](const auto &t) { return same_type(t, var_type); }) == meet_types.end());
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(seen, "Duplicate branch " + var_type->_string + " in case statement.", -1,
                                          nullptr);
        meet_types.push_back(var_type);

        SEMANT_RETURN_IF_FALSE(infer_expression_type(kase->_expr, scope), nullptr);

        stacks[i] = _expression_stack;
        i++;

        scope.pop_scope();
        classes.push_back(kase->_expr->_type);
    }

    // case don't create a new value
    _expression_stack = *std::max_element(stacks.begin(), stacks.end());

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER CASE TYPE"));
    return find_common_ancestor(classes);
}

std::shared_ptr<ast::Type> Semant::infer_dispatch_type(const ast::DispatchExpression &disp, Scope &scope)
{
    SEMANT_VERBOSE_ONLY(LOG_ENTER("INFER DISPATCH TYPE"));

    const auto is_static = std::holds_alternative<ast::StaticDispatchExpression>(disp._base);

    std::vector<int> stacks(disp._args.size() + 1, _expression_stack);

    SEMANT_RETURN_IF_FALSE(infer_expression_type(disp._expr, scope), nullptr);

#ifdef LLVM_SHADOW_STACK
    stacks[0] = _expression_stack + disp._args.size(); // can save all args on stack
#else
    stacks[0] = _expression_stack;
#endif // LLVM_SHADOW_STACK

    auto dispatch_expr_type = disp._expr->_type;
    if (is_static)
    {
        dispatch_expr_type = std::get<ast::StaticDispatchExpression>(disp._base)._type;

        // type if defined and not SELF_TYPE
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(!is_self_type(dispatch_expr_type), "Static dispatch to SELF_TYPE.", -1,
                                          nullptr);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_exists(dispatch_expr_type),
                                          "Static dispatch to undefined class " + dispatch_expr_type->_string + ".", -1,
                                          nullptr);

        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(disp._expr->_type, dispatch_expr_type),
                                          "Expression type " + disp._expr->_type->_string +
                                              " does not conform to declared static dispatch type " +
                                              dispatch_expr_type->_string + ".",
                                          -1, nullptr);
    }

    const auto &method_name = disp._object->_object;

    const auto feature = find_method(method_name, exact_type(dispatch_expr_type), false);
    SEMANT_RETURN_IF_FALSE_WITH_ERROR(feature, "Dispatch to undefined method " + method_name + ".", -1, nullptr);
    const auto &method = std::get<ast::MethodFeature>(feature->_base);

    SEMANT_RETURN_IF_FALSE_WITH_ERROR(method._formals.size() == disp._args.size(), "", 0, nullptr);
    for (auto i = 0; i < disp._args.size(); i++)
    {
        _expression_stack = stacks.at(i + 1);

        const auto &arg = disp._args[i];
        const auto &formal = method._formals[i];

        SEMANT_RETURN_IF_FALSE(infer_expression_type(arg, scope), nullptr);
        SEMANT_RETURN_IF_FALSE_WITH_ERROR(check_types_meet(arg->_type, formal->_type),
                                          "In call of method " + method_name + ", type " + arg->_type->_string +
                                              " of parameter " + formal->_object->_object +
                                              " does not conform to declared type " + formal->_type->_string + ".",
                                          arg->_line_number, nullptr);
#ifdef LLVM_SHADOW_STACK
        stacks[i + 1] = _expression_stack + i;
#else
        stacks[i + 1] = _expression_stack;
#endif // LLVM_SHADOW_STACK
    }

    _expression_stack = *std::max_element(stacks.begin(), stacks.end());

    SEMANT_VERBOSE_ONLY(LOG_EXIT("INFER DISPATCH TYPE"));
    return is_self_type(feature->_type) ? disp._expr->_type : feature->_type;
}

bool Semant::check_types_meet(const std::shared_ptr<ast::Type> &dynamic_type,
                              const std::shared_ptr<ast::Type> &static_type) const
{
    if (is_self_type(static_type))
    {
        return same_type(dynamic_type, static_type);
    }

    if (same_type(exact_type(dynamic_type), static_type))
    {
        return true;
    }

    for (const auto &t : _classes.at(static_type->_string)->_children)
    {
        if (check_types_meet(dynamic_type, t->_class->_type))
        {
            return true;
        }
    }

    return false;
}

std::shared_ptr<ast::Type> Semant::exact_type(const std::shared_ptr<ast::Type> &type) const
{
    return exact_type(type, _current_class);
}

std::shared_ptr<ast::Type> Semant::find_common_ancestor(const std::vector<std::shared_ptr<ast::Type>> &classes) const
{
    auto lca = exact_type(classes[0]);
    // if all classes are SELF_TYPE, so LCA is SELF_TYPE
    auto all_self_type = is_self_type(classes[0]);

    for (int i = 1; i < classes.size(); i++)
    {
        lca = find_common_ancestor_of_two(lca, exact_type(classes[i]));
        if (all_self_type)
        {
            all_self_type = is_self_type(classes[i]);
        }
        if (same_type(lca, Object))
        {
            break;
        }
    }

    return all_self_type ? SelfType : lca;
}

std::shared_ptr<ast::Type> Semant::find_common_ancestor_of_two(const std::shared_ptr<ast::Type> &t1,
                                                               const std::shared_ptr<ast::Type> &t2) const
{
    auto h1 = 0, h2 = 0;

    auto t1_par = t1;
    auto t2_par = t2;

    // find depth of the t1
    while (!same_type(_classes.at(t1_par->_string)->_class->_type, Object))
    {
        h1++;
        t1_par = _classes.at(t1_par->_string)->_class->_parent;
    }

    // find depth of the t2
    while (!same_type(_classes.at(t2_par->_string)->_class->_type, Object))
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

std::shared_ptr<ast::Feature> Semant::find_method(const std::string &name, const std::shared_ptr<ast::Type> &klass,
                                                  const bool &exact) const
{
    if (same_type(klass, Empty))
    {
        return nullptr;
    }

    std::shared_ptr<ast::Feature> method;

    for (const auto &m : _classes.at(klass->_string)->_class->_features)
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
    auto current_class = _classes.at(klass->_string)->_class->_parent;
    while (!same_type(current_class, Empty))
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

        current_class = _classes.at(current_class->_string)->_class->_parent;
    }

    return method;
}

std::string Semant::error_msg() const
{
    std::string prefix;
    if (_error_line_number != -1)
    {
        prefix = _error_file_name + ":" + std::to_string(_error_line_number) + ": ";
    }
    return prefix + _error_message;
}

std::shared_ptr<ast::Type> Semant::exact_type(const std::shared_ptr<ast::Type> &ltype,
                                              const std::shared_ptr<ast::Type> &rtype)
{
    return is_self_type(ltype) ? rtype : ltype;
}

std::shared_ptr<ast::Type> Semant::Bool = nullptr;
std::shared_ptr<ast::Type> Semant::Object = nullptr;
std::shared_ptr<ast::Type> Semant::Int = nullptr;
std::shared_ptr<ast::Type> Semant::String = nullptr;
std::shared_ptr<ast::Type> Semant::Io = nullptr;
std::shared_ptr<ast::Type> Semant::SelfType = nullptr;
std::shared_ptr<ast::Type> Semant::Empty = nullptr;
std::shared_ptr<ast::Type> Semant::NativeInt = nullptr;
std::shared_ptr<ast::Type> Semant::NativeBool = nullptr;
std::shared_ptr<ast::Type> Semant::NativeString = nullptr;