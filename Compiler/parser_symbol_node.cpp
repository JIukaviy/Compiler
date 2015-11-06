#include "parser_symbol_node.h"
#include "exceptions.h"
#include <vector>
#include <map>

map <SYM_TYPE, TOKEN> st_to_token;
map <TOKEN, SYM_TYPE> token_to_st;

void init_parser_symbol_node() {
	st_to_token[ST_VOID] = T_KWRD_VOID;
	st_to_token[ST_CHAR] = T_KWRD_CHAR;
	st_to_token[ST_DOUBLE] = T_KWRD_DOUBLE;
	st_to_token[ST_INTEGER] = T_KWRD_INT;
	st_to_token[ST_STRING] = T_STRING;
	st_to_token[ST_ALIAS] = T_KWRD_TYPEDEF;

	for each (auto var in st_to_token)
		token_to_st[var.second] = var.first;

	token_to_st[T_INTEGER] = ST_INTEGER;
	token_to_st[T_CHAR] = ST_CHAR;
	token_to_st[T_DOUBLE] = ST_DOUBLE;
}

type_ptr_t type_t::make_type(SYM_TYPE sym_type) {
	assert(sym_type != ST_ALIAS);
	assert(sym_type != ST_VOID);
	assert(sym_type != ST_FUNC);
	assert(sym_type != ST_VAR);
	assert(sym_type != ST_STRUCT);
	assert(sym_type != ST_ARRAY);
	type_ptr_t res;
	if (sym_type == ST_STRING) {
		sym_type_ptr_t* ptr = new sym_type_ptr_t;
		type_ptr_t ch(new sym_type_char_t);
		ptr->set_element_type(ch, pos_t());
		res = type_ptr_t(ptr);
	}
	res =
		sym_type == ST_INTEGER ? type_ptr_t(new sym_type_int_t()) :
		sym_type == ST_DOUBLE ? type_ptr_t(new sym_type_double_t()) :
		sym_type == ST_CHAR ? type_ptr_t(new sym_type_char_t()) : type_ptr_t(new sym_type_ptr_t());
	res->update_name();
	return res;
}

//--------------------------------SYMBOL-------------------------------

symbol_t::symbol_t() {}

symbol_t::symbol_t(SYM_TYPE symbol_type) : symbol_type(symbol_type) {}

void symbol_t::update_name() {
	name = _get_name();
}

const string& symbol_t::get_name() const {
	return name;
}

bool symbol_t::lower(sym_ptr_t s) const {
	return name < s->get_name();
}

bool symbol_t::equal(sym_ptr_t s) const {
	return name == s->get_name();
}

bool symbol_t::unequal(sym_ptr_t s) const {
	return name != s->get_name();
}

bool symbol_t::is(SYM_TYPE sym_type) const {
	return symbol_type == sym_type;
}

SYM_TYPE symbol_t::get_sym_type() const {
	return symbol_type;
}

SYM_TYPE symbol_t::token_to_sym_type(TOKEN token) {
	return token_to_st.at(token);
}

TOKEN symbol_t::sym_type_to_token(SYM_TYPE st) {
	return st_to_token.at(st);
}

//--------------------------------TYPE-------------------------------

type_t::type_t() {}

type_t::type_t(SYM_TYPE symbol_type) : symbol_t(symbol_type) {}

void type_t::set_const(bool val) {
	is_const_ = val;
}

void type_t::print(ostream & os) {
	if (is_const_)
		os << "const ";
}

bool type_t::is_const() const {
	return is_const_;
}

bool type_t::completed() {
	return true;
}

//--------------------------------SYMBOL_BUILT_IN_TYPE-------------------------------

string sym_built_in_type::_get_name() const {
	return token_t::get_name_by_id(st_to_token.at(symbol_type));
}

void sym_built_in_type::print(ostream & os) {
	type_t::print(os);
	os << token_t::get_name_by_id(st_to_token.at(symbol_type)) << ' ';
}

//--------------------------------SYMBOL_STRING_LITERAL-------------------------------

sym_type_str_literal_t::sym_type_str_literal_t(token_ptr_t str) : type_t(ST_STRING), str(str) {}

int sym_type_str_literal_t::get_size() {
	return static_cast<token_with_value_t<string>*>(str.get())->get_value().length();
}

void sym_type_str_literal_t::print(ostream& os) {
	os << '"';
	str->short_print(os);
	os << '"';
}

//--------------------------------SYMBOL_VOID-------------------------------

sym_type_void_t::sym_type_void_t() : type_t(ST_VOID) {}

bool sym_type_void_t::completed() {
	return false;
}

sym_type_int_t::sym_type_int_t() : type_t(ST_INTEGER) {}

int sym_type_int_t::get_size() {
	return sizeof(int);
}

sym_type_char_t::sym_type_char_t() : type_t(ST_CHAR) {}

int sym_type_char_t::get_size() {
	return sizeof(char);
}

sym_type_double_t::sym_type_double_t() : type_t(ST_DOUBLE) {}

int sym_type_double_t::get_size() {
	return sizeof(float);
}

//--------------------------------SYMBOL_VAR-------------------------------

sym_var_t::sym_var_t(token_ptr_t identifier, type_ptr_t type, vector<expr_t*> init_list) : symbol_t(ST_VAR), identifier(identifier), type(type), init_list(init_list) {}

type_ptr_t sym_var_t::get_type() {
	return type;
}

string sym_var_t::_get_name() const {
	return static_cast<token_with_value_t<string>*>(identifier.get())->get_value();
}

void sym_var_t::print(ostream & os) {
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

sym_type_ptr_t::sym_type_ptr_t() : type_t(ST_PTR) {}

int sym_type_ptr_t::get_size() {
	return sizeof(void*);
}

string sym_type_ptr_t::_get_name() const {
	type->update_name();
	return token_t::get_name_by_id(T_OP_MUL) + type->get_name();
}

void sym_type_ptr_t::set_element_type(type_ptr_t type_, pos_t pos) {
	type = type_;
}

void sym_type_ptr_t::print(ostream & os) {
	type_t::print(os);
	os << "pointer to " << type->get_name();
}

type_ptr_t sym_type_ptr_t::get_element_type() {
	return type;
}

//--------------------------------SYMBOL_TYPE_ARRAY-------------------------------

sym_type_array_t::sym_type_array_t(expr_t* size, bool is_const_) : type_t(ST_ARRAY), size(size) {}

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

type_ptr_t sym_type_array_t::get_element_type() {
	return elem_type;
}

void sym_type_array_t::set_element_type(type_ptr_t type, pos_t pos) {
	if (!type)
		return;
	if (!type->completed())
		throw SemanticError("Array can contatin only completed types", pos);
	if (type == ST_FUNC)
		throw SemanticError("Array can't contain functions", pos);

	elem_type = dynamic_pointer_cast<type_with_size_t>(type);
	if (!elem_type)
		throw InvalidIncompleteType(pos);
}

//--------------------------------SYMBOL_TYPE_FUNC-------------------------------

sym_type_func_t::sym_type_func_t(vector<expr_t*>& args, bool is_const_) : type_t(ST_FUNC), args(args) {}

void sym_type_func_t::set_element_type(type_ptr_t type, pos_t pos) {
	if (!type)
		return;
	if (type == ST_ARRAY)
		throw SemanticError("Function can't return array", pos);
	if (type == ST_FUNC)
		throw SemanticError("Function can't return function", pos);

	ret_type = type;
}

//--------------------------------SYMBOL_TYPE_ALIAS-------------------------------

sym_type_alias_t::sym_type_alias_t(token_ptr_t identifier, type_ptr_t type) : type_t(ST_ALIAS), identifier(identifier), type(type) {}

bool sym_type_alias_t::is_const() const {
	return type->is_const() || is_const_;
}

void sym_type_alias_t::print(ostream& os) {
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

type_ptr_t sym_type_alias_t::get_type() {
	return type;
}

/*int sym_type_alias_t::get_size() {
if (type)
return type->get_size();
else
throw;
}*/

//--------------------------------SYMBOL_POINTER-------------------------------

bool sym_ptr_t::operator==(SYM_TYPE sym_type) const {
	return get()->is(sym_type);
}

bool type_ptr_t::operator==(SYM_TYPE sym_type) const {
	return get()->is(sym_type);
}

bool type_ptr_t::operator!=(SYM_TYPE sym_type) const {
	return !(*this == sym_type);
}

bool type_ptr_t::operator==(type_ptr_t type) const {
	return (*this)->equal(type);
}

bool type_ptr_t::operator!=(type_ptr_t type) const {
	return (*this)->unequal(type);
}

bool updt_sym_ptr_t::operator==(SYM_TYPE sym_type) const {
	return get()->is(sym_type);
}

//--------------------------------SYMBOL_STRUCT-------------------------------

sym_type_struct::sym_type_struct(sym_table_t * sym_table) : sym_table(sym_table) {}

sym_table_t * sym_type_struct::get_sym_table() {
	return sym_table;
}

bool sym_type_struct::completed() {
	return sym_table;
}

int sym_type_struct::get_size() {
	return 0;
}

void sym_type_struct::print(ostream & os) {

}
