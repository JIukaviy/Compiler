#include "symbol_table.h"
#include "exceptions.h"
#include "parser.h"
#include "type_conversion.h"
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

type_base_ptr type_base_t::make_type(SYM_TYPE sym_type) {
	assert(sym_type != ST_ALIAS);
	assert(sym_type != ST_FUNC);
	assert(sym_type != ST_VAR);
	assert(sym_type != ST_STRUCT);
	assert(sym_type != ST_ARRAY);
	assert(sym_type != ST_QL);
	type_base_ptr res;
	if (sym_type == ST_STRING) {
		sym_type_ptr_t* ptr = new sym_type_ptr_t;
		type_base_ptr ch(new sym_type_char_t);
		ptr->set_element_type(type_ptr(new type_t(ch, true)));
		res = type_base_ptr(ptr);
	} else 
		res =
			sym_type == ST_INTEGER ? type_base_ptr(new sym_type_int_t()) :
			sym_type == ST_DOUBLE ? type_base_ptr(new sym_type_double_t()) :
			sym_type == ST_CHAR ? type_base_ptr(new sym_type_char_t()) :
			sym_type == ST_VOID ? type_base_ptr(new sym_type_void_t()) : type_base_ptr(new sym_type_ptr_t());
	res->update_name();
	return res;
}

//--------------------------------SYMBOL-------------------------------

symbol_t::symbol_t(SYM_TYPE symbol_type) : symbol_type(symbol_type) {}

symbol_t::symbol_t(SYM_TYPE symbol_type, token_ptr token) : symbol_type(symbol_type), token(token) {}

void symbol_t::update_name() {
	name = _get_name();
}

const string& symbol_t::get_name() const {
	return name;
}

bool symbol_t::lower(sym_ptr s) const {
	return get_name() < s->get_name();
}

bool symbol_t::equal(sym_ptr s) const {
	return get_name() == s->get_name();
}

bool symbol_t::unequal(sym_ptr s) const {
	return get_name() != s->get_name();
}

bool symbol_t::is(SYM_TYPE sym_type) const {
	return symbol_type == sym_type;
}

bool symbol_t::is(sym_ptr sym_type) const {
	return name == sym_type->get_name();
}

SYM_TYPE symbol_t::get_sym_type() const {
	return symbol_type;
}

token_ptr symbol_t::get_token() {
	return token;
}

void symbol_t::set_token(token_ptr token_) {
	token = token_;
}

void symbol_t::short_print_l(ostream& os, int level) {
	os << get_name();
}

SYM_TYPE symbol_t::token_to_sym_type(TOKEN token) {
	return token_to_st.at(token);
}

TOKEN symbol_t::sym_type_to_token(SYM_TYPE st) {
	return st_to_token.at(st);
}

//--------------------------------BASE_TYPE-------------------------------

type_base_t::type_base_t() : symbol_t(ST_VOID) {}

bool type_base_t::completed() {
	return true;
}

int type_base_t::get_size() {
	throw SemanticError("Can't get size of the symbol");
}

bool type_base_t::is_integer(SYM_TYPE sym_type) {
	return (sym_type == ST_CHAR) || (sym_type == ST_INTEGER);
}

bool type_base_t::is_arithmetic(SYM_TYPE sym_type) {
	return is_integer(sym_type) || sym_type == ST_DOUBLE;
}

bool type_base_t::is_integer() {
	return is_integer(symbol_type);
}

bool type_base_t::is_arithmetic() {
	return is_arithmetic(symbol_type);
}

//--------------------------------TYPE-------------------------------

string type_t::_get_name() const {
	if (_is_const) {
		string res("c ");
		return res + type->get_name();
	}
	return type->get_name();
}

type_t::type_t(type_base_ptr type_) : symbol_t(ST_QL) {
	set_base_type(type_);
}

type_t::type_t(type_base_ptr type_, bool is_const) : symbol_t(ST_QL) {
	set_base_type(type_);
	_is_const = is_const;
}

void type_t::print_l(ostream& os, int level) {
	if (_is_const)
		os << "const ";
	type->print_l(os, level);
}

type_base_ptr type_t::get_base_type() {
	return type;
}

void type_t::update_name() {
	type->update_name();
	type_base_t::update_name();
}

void type_t::set_base_type(type_base_ptr type_) {
	assert(type_);
	if (type_ == ST_QL) {
		type_ptr ql = static_pointer_cast<type_t>(type_);
		type_ = ql->get_base_type();
		set_is_const(is_const() || ql->is_const());
	}
	type = type_;
	set_token(type->get_token());
	name = type->get_name();
}

bool type_t::completed() {
	return type->completed();
}

bool type_t::is(SYM_TYPE sym_type) const {
	return type_base_t::is(sym_type) || type->is(sym_type);
}

bool type_t::is(type_ptr type_) const {
	return type_base_t::is(type);
}

int type_t::get_size() {
	return type->get_size();
}

bool type_t::is_const() {
	return _is_const;
}

bool type_t::is_integer() {
	return type->is_integer();
}

bool type_t::is_arithmetic() {
	return type->is_arithmetic();
}

void type_t::set_is_const(bool is_const) {
	_is_const = is_const;
}

token_ptr type_t::get_token() {
	return type->get_token();
}

type_ptr type_t::make_type(type_base_ptr base_type, bool is_const) {
	auto res = type_ptr(new type_t(base_type, is_const));
	res->update_name();
	return res;
}

type_ptr type_t::make_type(SYM_TYPE sym_type, bool is_const) {
	return make_type(type_base_t::make_type(sym_type), is_const);
}

type_ptr type_t::make_type(type_base_t* base_type, bool is_const) {
	return make_type(type_base_ptr(base_type), is_const);
}

void type_t::set_token(token_ptr token) {
	type->set_token(token);
}

//--------------------------------UPDATABLE_BASE_TYPE-------------------------------

updatable_base_type_t::updatable_base_type_t() : symbol_t(ST_VOID) {}

void updatable_base_type_t::set_element_type(type_ptr type) {
	elem_type = type;
}

type_ptr updatable_base_type_t::get_element_type() {
	return elem_type;
}

void updatable_base_type_t::update_name() {
	if (elem_type)
		elem_type->update_name();
	type_base_t::update_name();
}

//--------------------------------SYMBOL_BUILT_IN_TYPE-------------------------------

string sym_built_in_type::_get_name() const {
	return token_t::get_name_by_id(st_to_token.at(symbol_type));
}

sym_built_in_type::sym_built_in_type() : symbol_t(ST_VOID) {}

void sym_built_in_type::print_l(ostream& os, int level) {
	os << token_t::get_name_by_id(st_to_token.at(symbol_type));
}

//--------------------------------SYMBOL_STRING_LITERAL-------------------------------

sym_type_str_literal_t::sym_type_str_literal_t(token_ptr str) : symbol_t(ST_STRING), str(str) {
	update_name();
}

int sym_type_str_literal_t::get_size() {
	return static_pointer_cast<token_with_value_t<string>>(str)->get_value().length();
}

void sym_type_str_literal_t::print_l(ostream& os, int level) {
	os << '"';
	str->short_print(os);
	os << '"';
}

//--------------------------------SYMBOL_VOID-------------------------------

sym_type_void_t::sym_type_void_t() : symbol_t(ST_VOID) {
	update_name();
}

bool sym_type_void_t::completed() {
	return false;
}

sym_type_int_t::sym_type_int_t() : symbol_t(ST_INTEGER) {
	update_name();
}

int sym_type_int_t::get_size() {
	return sizeof(int);
}

sym_type_char_t::sym_type_char_t() : symbol_t(ST_CHAR) {
	update_name();
}

int sym_type_char_t::get_size() {
	return sizeof(char);
}

sym_type_double_t::sym_type_double_t() : symbol_t(ST_DOUBLE) {
	update_name();
}

int sym_type_double_t::get_size() {
	return sizeof(float);
}

//--------------------------------SYMBOL_WITH_TYPE-------------------------------

sym_with_type_t::sym_with_type_t() : symbol_t(ST_VOID, token_ptr()) {}

sym_with_type_t::sym_with_type_t(type_ptr type) : symbol_t(ST_VOID, token_ptr()), type(type) {}

type_ptr sym_with_type_t::get_type() {
	return type;
}

string sym_with_type_t::_get_name() const {
	return static_pointer_cast<token_with_value_t<string>>(token)->get_value();
}

//--------------------------------SYMBOL_VAR-------------------------------

sym_var_t::sym_var_t(token_ptr identifier) : symbol_t(ST_VAR, identifier) {
	update_name();
}

void sym_var_t::set_type_and_init_list(type_ptr type_, vector<expr_t*> init_list_) {
	type = type_;
	if (init_list_.size() > 0) {
		auto arr = dynamic_pointer_cast<sym_type_array_t>(type_->get_base_type());
		if (type_ != ST_STRUCT && !arr && init_list_.size() > 1)
			throw InvalidInitListSize(init_list_[1]->get_pos());
		if (arr) {
			if (arr->get_element_type() == ST_CHAR && 
				typeid(*init_list_[0]) == typeid(expr_const_t) &&
				static_cast<expr_const_t*>(init_list_[0])->get_token() == T_STRING) 
			{
				size_t str_literal_size = init_list_[0]->get_type()->get_size();
				if (arr->completed() && arr->get_len() < str_literal_size)
					throw SemanticError("Initializer string for array of char is too long", init_list_[0]->get_pos());
				arr->set_len(str_literal_size);
				return;
			}
			if (arr->completed() && arr->get_len() < init_list_.size())
				throw InvalidInitListSize(init_list_[1]->get_pos());
			if (!arr->completed())
				arr->set_len(init_list_.size());
			init_list.resize(init_list_.size());
			for (int i = 0; i < init_list_.size(); i++)
				init_list[i] = auto_convert(init_list_[i], arr->get_element_type());
		} else
			init_list.push_back(auto_convert(init_list_[0], type));
	}
	if (!type_->completed())
		throw InvalidIncompleteType(token->get_pos());
}

void sym_var_t::print_l(ostream& os, int level) {
	os << "variable: ";
	token->short_print(os);
	os << ", type: ";
	type->print_l(os, level);
	if (!init_list.empty()) {
		os << ", init list: {";
		for (int i = 0; i < init_list.size(); i++) {
			init_list[i]->short_print(os);
			if (i != init_list.size() - 1)
				os << ", ";
		}
		os << "}";
	}
}

void sym_var_t::short_print_l(ostream& os, int level) {
	os << "variable: \"";
	token->short_print(os);
	os << "\", type: ";
	type->short_print_l(os, level);
}

const vector<expr_t*>& sym_var_t::get_init_list() {
	return init_list;
}

//--------------------------------SYMBOL_TYPE_POINTER-------------------------------

sym_type_ptr_t::sym_type_ptr_t() : symbol_t(ST_PTR) {}

int sym_type_ptr_t::get_size() {
	return sizeof(void*);
}

string sym_type_ptr_t::_get_name() const {
	return token_t::get_name_by_id(T_OP_MUL) + elem_type->get_name();
}

void sym_type_ptr_t::print_l(ostream& os, int level) {
	os << "pointer to ";
	elem_type->print_l(os, level);
}

type_ptr sym_type_ptr_t::get_element_type() {
	return elem_type;
}

type_ptr sym_type_ptr_t::dereference(type_ptr type) {
	auto ptr = dynamic_pointer_cast<sym_type_ptr_t>(type->get_base_type());
	assert(ptr);
	return ptr->get_element_type();
}

type_ptr sym_type_ptr_t::make_ptr(type_ptr type, bool is_const) {
	auto ptr = shared_ptr<sym_type_ptr_t>(new sym_type_ptr_t);
	ptr->set_element_type(type);
	return type_t::make_type(ptr, is_const);
}

//--------------------------------SYMBOL_TYPE_ARRAY-------------------------------

sym_type_array_t::sym_type_array_t(expr_t* size_expr) : symbol_t(ST_ARRAY), size_expr(size_expr) {
	if (size_expr)
		if (!size_expr->get_type()->is_integer())
			throw SemanticError("Size of array has non-integer type", size_expr->get_pos());
		else
			len = var_pointer_cast<int>(size_expr->eval())->get_val();
}

int sym_type_array_t::get_size() {
	return len * elem_type->get_size();
}

int sym_type_array_t::get_len() {
	return len;
}

string sym_type_array_t::_get_name() const {
	elem_type->update_name();
	return token_t::get_name_by_id(T_SQR_BRACKET_OPEN) + token_t::get_name_by_id(T_SQR_BRACKET_CLOSE) + elem_type->get_name();
}

void sym_type_array_t::print_l(ostream& os, int level) {
	os << "array[";
	if (len)
		os << len;
	os << "] with elems: ";
	elem_type->print(os);
}

bool sym_type_array_t::completed() {
	return len;
}

void sym_type_array_t::set_len(size_t len_) {
	len = len_;
}

type_ptr sym_type_array_t::get_element_type() {
	return elem_type;
}

type_ptr sym_type_array_t::get_ptr_to_elem_type() {
	return sym_type_ptr_t::make_ptr(elem_type);
}

void sym_type_array_t::set_element_type(type_ptr type) {
	if (!type)
		return;
	if (type == ST_FUNC)
		throw SemanticError("Array can't contain functions", type->get_token()->get_pos());
	if (!type->completed())
		throw SemanticError("Array type has incomplete element type", type->get_token()->get_pos());

	elem_type = type;
}

//--------------------------------SYMBOL_TYPE_FUNC-------------------------------

sym_type_func_t::sym_type_func_t(const vector<type_ptr> &at) : symbol_t(ST_FUNC), arg_types(at) {}

void sym_type_func_t::update_name() {
	elem_type->update_name();
	for each (auto var in arg_types)
		var->update_name();
	name = _get_name();
}

string sym_type_func_t::_get_name() const {
	string res;
	res += '(';
	for (int i = 0; i < arg_types.size(); i++) {
		res += arg_types[i]->get_name();
		if (i < arg_types.size() - 1)
			res += ',';
	}
	res += ')';
	res += elem_type->get_name();
	return res;
}

void sym_type_func_t::set_arg_types(const vector<type_ptr> &at) {
	arg_types = at;
}

vector<type_ptr> sym_type_func_t::get_arg_types() {
	return arg_types;
}

void sym_type_func_t::set_element_type(type_ptr type) {
	if (!type)
		return;
	if (type == ST_ARRAY)
		throw SemanticError("Function can't return array", type->get_token()->get_pos());
	if (type == ST_FUNC)
		throw SemanticError("Function can't return function", type->get_token()->get_pos());

	elem_type = type;
}

void sym_type_func_t::print_l(ostream& os, int level) {
	os << "function(";
	for (int i = 0; i < arg_types.size(); i++) {
		arg_types[i]->print(os);
		if (i < arg_types.size() - 1)
			os << ", ";
	}
	os << ") that returns: ";
	elem_type->print_l(os, level);
}

//--------------------------------SYMBOL_FUNCTION-------------------------------

sym_func_t::sym_func_t(token_ptr identifier, shared_ptr<sym_type_func_t> func_type, sym_table_ptr  sym_table) : sym_with_type_t(type_ptr(new type_t(func_type))), sym_table(sym_table), symbol_t(ST_FUNC, identifier) {
	update_name();
}

shared_ptr<sym_type_func_t> sym_func_t::get_func_type() {
	return static_pointer_cast<sym_type_func_t>(type->get_base_type());
}

void sym_func_t::set_block(stmt_ptr b) {
	block = b;
}

void sym_func_t::set_sym_table(sym_table_ptr sym_table_) {
	sym_table = sym_table_;
}

sym_table_ptr  sym_func_t::get_sym_table() {
	return sym_table;
}

void sym_func_t::clear_sym_table() {
	sym_table = nullptr;
}

bool sym_func_t::defined() {
	return (bool)block;
}

void sym_func_t::short_print_l(ostream& os, int level) {
	os << _get_name() + ": ";
	get_func_type()->print_l(os, level);
}

void sym_func_t::print_l(ostream& os, int level) {
	os << _get_name() + ": ";
	get_func_type()->print_l(os, level);
	if (block) {
		os << ' ';
		block->print_l(os, level);
	}
}

//--------------------------------SYMBOL_TYPE_ALIAS-------------------------------

sym_type_alias_t::sym_type_alias_t(token_ptr identifier, type_ptr type) : sym_with_type_t(type), symbol_t(ST_ALIAS, identifier) {
	update_name();
}

void sym_type_alias_t::update_name() {
	type->update_name();
	name = _get_name();
}

void sym_type_alias_t::print_l(ostream& os, int level) {
	os << "alias: \"";
	token->short_print(os);
	os << "\", type: ";
	type->print_l(os, level);
}

bool sym_type_alias_t::completed() {
	return type->completed();
}

/*int sym_type_alias_t::get_size() {
if (type)
return type->get_size();
else
throw;
}*/

//--------------------------------SYMBOL_POINTER-------------------------------

bool sym_ptr::operator==(SYM_TYPE sym_type) const {
	return get()->is(sym_type);
}

bool sym_ptr::operator!=(SYM_TYPE sym_type) const {
	return !(*this == sym_type);
}

bool type_ptr::operator==(SYM_TYPE sym_type) const {
	return get()->is(sym_type);
}

bool type_ptr::operator!=(SYM_TYPE sym_type) const {
	return !(*this == sym_type);
}

bool type_ptr::operator==(type_ptr type) const {
	return (*this)->equal(type);
}

bool type_ptr::operator!=(type_ptr type) const {
	return (*this)->unequal(type);
}

bool type_base_ptr::operator==(SYM_TYPE sym_type) const {
	return get()->is(sym_type);
}

bool type_base_ptr::operator!=(SYM_TYPE sym_type) const {
	return !(*this == sym_type);
}

bool type_base_ptr::operator==(type_ptr type) const {
	return (*this)->equal(type);
}

bool type_base_ptr::operator!=(type_ptr type) const {
	return (*this)->unequal(type);
}

//--------------------------------SYMBOL_STRUCT-------------------------------

sym_type_struct_t::sym_type_struct_t(sym_table_ptr  sym_table, token_ptr identifier) : symbol_t(ST_STRUCT), sym_table(sym_table), identifier(identifier) {
	update_name();
}

sym_type_struct_t::sym_type_struct_t(token_ptr identifier) : symbol_t(ST_STRUCT), identifier(identifier), sym_table(0) {
	update_name();
}

string sym_type_struct_t::_get_name() const {
	return "struct " + static_pointer_cast<token_with_value_t<string>>(identifier)->get_value();
}

void sym_type_struct_t::set_sym_table(sym_table_ptr  s) {
	sym_table = s;
}

sym_table_ptr  sym_type_struct_t::get_sym_table() {
	return sym_table;
}

shared_ptr<sym_var_t> sym_type_struct_t::get_member(token_ptr member) {
	auto res = dynamic_pointer_cast<sym_var_t>(sym_table->find_global(member));
	if (!res)
		throw StructHasNoMember(this, member);
	return res;
}

bool sym_type_struct_t::completed() {
	return (bool)sym_table;
}

int sym_type_struct_t::get_size() {
	return 0;
}

void sym_type_struct_t::print_l(ostream& os, int level) {
	short_print(os);
	if (sym_table) {
		os << " {";
		if (!sym_table->empty()) {
			os << endl;
			sym_table->short_print_l(os, level + 1);
			print_level(os, level);
		}
		os << '}';
	}
}

void sym_type_struct_t::short_print_l(ostream& os, int level) {
	os << "struct " << static_pointer_cast<token_with_value_t<string>>(identifier)->get_value();
}
