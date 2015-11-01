#include "parser_symbol_node.h"
#include "exceptions.h"

size_t rot_13_hash(unsigned char byte, size_t hash) {
	hash += byte;
	hash -= (hash << 13) | (hash >> 19);
	return hash;
}

size_t rot_13_hash(unsigned int block, size_t hash) {
	unsigned char* block_ptr = (unsigned char*)(&block);
	for (int i = 0; i < 4; i++)
		hash = rot_13_hash(*(block_ptr + i), hash);
	return hash;
}

void symbol_t::update_hash() {
	cached_hash = _get_hash();
}

size_t symbol_t::get_hash() const {
	return cached_hash;
}

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

void sym_type_void_t::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	type_t::print(os);
	os << "void ";
}

bool sym_type_void_t::completed() {
	return false;
}

size_t sym_type_void_t::_get_hash() const {
	return T_KWRD_VOID;
}

bool sym_type_void_t::eq(const symbol_t* s) const {
	return s && typeid(*s) == typeid(sym_type_void_t);
}

int sym_type_int_t::get_size() {
	return sizeof(int);
}

size_t sym_type_int_t::_get_hash() const {
	return T_KWRD_INT;
}

bool sym_type_int_t::eq(const symbol_t* s) const {
	return s && typeid(*s) == typeid(sym_type_int_t);
}

void sym_type_int_t::print(ostream & os) {
	type_t::print(os);
	os << token_t::get_name_by_id(T_KWRD_INT) << ' ';
}

int sym_type_char_t::get_size() {
	return sizeof(char);
}

size_t sym_type_char_t::_get_hash() const {
	return T_KWRD_CHAR;
}

bool sym_type_char_t::eq(const symbol_t* s) const {
	return s && typeid(*s) == typeid(sym_type_char_t);
}

void sym_type_char_t::print(ostream & os) {
	type_t::print(os);
	os << token_t::get_name_by_id(T_KWRD_CHAR) << ' ';
}

int sym_type_double_t::get_size() {
	return sizeof(float);
}

size_t sym_type_double_t::_get_hash() const {
	return T_KWRD_DOUBLE;
}

bool sym_type_double_t::eq(const symbol_t* s) const {
	return s && typeid(*s) == typeid(sym_type_double_t);
}

void sym_type_double_t::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	type_t::print(os);
	os << token_t::get_name_by_id(T_KWRD_DOUBLE) << ' ';
}

type_with_size_t::type_with_size_t(bool is_const_) : type_t(is_const_) {}
updatable_sym_t::updatable_sym_t(bool is_const_) : type_t(is_const_) {}

size_t sym_var_t::_get_hash() const {
	type->update_hash();
	const string name = static_cast<token_with_value_t<string>*>(identifier.get())->get_value();
	hash<string> hash_fn;
	return hash_fn(name);
}

bool sym_var_t::eq(const symbol_t* s) const {
	const sym_var_t* var = dynamic_cast<const sym_var_t*>(s);
	return var && identifier == var->identifier;
}

int sym_type_ptr_t::get_size() {
	return sizeof(void*);
}

size_t sym_type_ptr_t::_get_hash() const {
	type->update_hash();
	return rot_13_hash((unsigned char)T_OP_MUL, type->get_hash());
}

bool sym_type_ptr_t::eq(const symbol_t* s) const {
	const sym_type_ptr_t* ptr = dynamic_cast<const sym_type_ptr_t*>(s);
	return ptr && type->eq(ptr->type);
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
	os << "pointer to ";
	type->print(os);
}

sym_type_array_t::sym_type_array_t(expr_t* size, bool is_const_) : type_t(is_const_), size(size) {}

int sym_type_array_t::get_size() {
	/*if (type)
	return type->get_size() * size;
	else*/
	return sizeof(void*);
}

size_t sym_type_array_t::_get_hash() const {
	elem_type->update_hash();
	return rot_13_hash((unsigned char)T_SQR_BRACKET_OPEN, elem_type->get_hash());
}

bool sym_type_array_t::eq(const symbol_t* s) const {
	const sym_type_array_t* array = dynamic_cast<const sym_type_array_t*>(s);
	return array && elem_type->eq(array->elem_type);
}

void sym_type_array_t::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	type_t::print(os);
	os << "array[";
	if (size)
		size->flat_print(os);
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

sym_var_t::sym_var_t(token_ptr_t identifier, type_t* type, vector<node_t*> init_list) : identifier(identifier), type(type), init_list(init_list) {}

void sym_var_t::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	os << "variable: \"";
	identifier->short_print(os);
	os << "\", type: " << type->get_hash();
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

sym_type_alias_t::sym_type_alias_t(token_ptr_t identifier, type_t* type) : identifier(identifier), type(type) {}

bool sym_type_alias_t::is_const() {
	return type->is_const();
}

void sym_type_alias_t::set_const(bool val) {
	type->set_const(val);
}

void sym_type_alias_t::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	os << "alias: \"";
	identifier->short_print(os);
	os << "\", type: " << type->get_hash();
}

size_t sym_type_alias_t::_get_hash() const {
	type->update_hash();
	return rot_13_hash((unsigned char)T_KWRD_TYPEDEF, type->get_hash());
}

bool sym_type_alias_t::eq(const symbol_t* s) const {
	const sym_type_alias_t* alias = dynamic_cast<const sym_type_alias_t*>(s);
	return alias && type->eq(alias->type);
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