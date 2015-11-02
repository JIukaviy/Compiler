#include "parser_symbol_node.h"
#include "exceptions.h"

//--------------------------------SYMBOL-------------------------------

symbol_t::symbol_t() {}

symbol_t::symbol_t(string name) : name(name) {}

void symbol_t::update_name() {
	name = _get_name();
}

const string& symbol_t::get_name() const {
	return name;
}

bool symbol_t::lower(symbol_t* s) const {
	return name < s->get_name();
}

//--------------------------------TYPE-------------------------------

type_t::type_t(bool is_const_) : is_const_(is_const_) {}

void type_t::set_const(bool val) {
	is_const_ = val;
}

void type_t::print(ostream & os) {
	if (is_const_)
		os << "const ";
}

bool type_t::is_const() {
	return is_const_;
}

bool type_t::completed() {
	return true;
}

//--------------------------------VOID-------------------------------

bool sym_type_void_t::completed() {
	return false;
}

int sym_type_int_t::get_size() {
	return sizeof(int);
}

int sym_type_char_t::get_size() {
	return sizeof(char);
}

int sym_type_double_t::get_size() {
	return sizeof(float);
}

//--------------------------------TYPE_WITH_SIZE-------------------------------

type_with_size_t::type_with_size_t(bool is_const_) : type_t(is_const_) {}

//--------------------------------UPDATABLE_TYPE-------------------------------
updatable_sym_t::updatable_sym_t(bool is_const_) : type_t(is_const_) {}

//--------------------------------SYMBOL_VAR-------------------------------

sym_var_t::sym_var_t(token_ptr_t identifier, type_t* type, vector<node_t*> init_list) : symbol_t(type->get_name()), identifier(identifier), type(type), init_list(init_list) {}

string sym_var_t::_get_name() const {
	return static_cast<token_with_value_t<string>*>(identifier.get())->get_value();
}

void sym_var_t::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	os << "variable: \"";
	identifier->short_print(os);
	os << "\", type: " << type->get_name();
	/*type->print(os);
	if (!init_list.empty()) {
	os << ", init list: {";
	for (int i = 0; i < init_list.size(); i++) {
	init_list[i]->flat_print(os);
	if (i != init_list.size() - 1)
	os << ", ";
	}
	os << "}";
	}*/
}

//--------------------------------SYMBOL_TYPE_POINTER-------------------------------

int sym_type_ptr_t::get_size() {
	return sizeof(void*);
}

string sym_type_ptr_t::_get_name() const {
	type->update_name();
	return token_t::get_name_by_id(T_OP_MUL) + type->get_name();
}

sym_type_ptr_t::sym_type_ptr_t(bool is_const_) : type_t(is_const_) {}

void sym_type_ptr_t::set_element_type(type_t * type_, pos_t pos) {
	type = type_;
}

void sym_type_ptr_t::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	type_t::print(os);
	os << "pointer to " << type->get_name();
}

//--------------------------------SYMBOL_TYPE_ARRAY-------------------------------

sym_type_array_t::sym_type_array_t(expr_t* size, bool is_const_) : type_t(is_const_), size(size) {}

int sym_type_array_t::get_size() {
	/*if (type)
	return type->get_size() * size;
	else*/
	return sizeof(void*);
}

string sym_type_array_t::_get_name() const {
	elem_type->update_name();
	return token_t::get_name_by_id(T_SQR_BRACKET_OPEN) + token_t::get_name_by_id(T_SQR_BRACKET_CLOSE) + elem_type->get_name();
}

void sym_type_array_t::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	type_t::print(os);
	os << "array[";
	if (size)
		size->short_print(os);
	os << "] with elems: ";
	elem_type->print(os);
}

bool sym_type_array_t::completed() {
	return size;
}

void sym_type_array_t::set_element_type(type_t* type, pos_t pos) {
	if (!type)
		return;
	if (!type->completed())
		throw SemanticError("Array can contatin only completed types", pos);
	if (typeid(*type) == typeid(sym_type_func_t))
		throw SemanticError("Array can't contain functions", pos);

	elem_type = dynamic_cast<type_with_size_t*>(type);
	if (!elem_type)
		throw InvalidIncompleteType(pos);
}

//--------------------------------SYMBOL_TYPE_FUNC-------------------------------

sym_type_func_t::sym_type_func_t(vector<type_t*>& args, bool is_const_) : args(args), type_t(is_const_) {}

void sym_type_func_t::set_element_type(type_t* type, pos_t pos) {
	if (!type)
		return;
	if (typeid(*type) == typeid(sym_type_array_t))
		throw SemanticError("Function can't return array", pos);
	if (typeid(*type) == typeid(sym_type_func_t))
		throw SemanticError("Function can't return function", pos);

	ret_type = type;
}

//--------------------------------SYMBOL_TYPE_ALIAS-------------------------------

sym_type_alias_t::sym_type_alias_t(token_ptr_t identifier, type_t* type) : identifier(identifier), type(type) {}

bool sym_type_alias_t::is_const() {
	return type->is_const() || is_const_;
}

void sym_type_alias_t::print(ostream& os) {
	if (printed)
		return;
	else
		printed = true;
	type_t::print(os);
	os << "alias: \"";
	identifier->short_print(os);
	os << "\", type: " << type->get_name();
}

string sym_type_alias_t::_get_name() const {
	/*type->update_name();
	char t[20];
	_ltoa_s(T_KWRD_TYPEDEF, t, 10);
	return string(t) + type->get_name();*/
	return static_cast<token_with_value_t<string>*>(identifier.get())->get_value();

}

type_t* sym_type_alias_t::get_type() {
	return type;
}

/*int sym_type_alias_t::get_size() {
if (type)
return type->get_size();
else
throw;
}*/