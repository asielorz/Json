#include "json/value.hh"
#include "json/parser.hh"
#include "json/writer.hh"
#include "json/release.hh"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <functional>
#include <filesystem>

#include <fstream>
#include <sstream>
#include <list>
#include <forward_list>
#include <chrono>

std::vector<std::function<void()>> tests;

#if _DEBUG
#define debug_assert(expression) assert(expression)
#else
#define debug_assert(expression) static_cast<void>(expression)
#endif

struct call_on_init
{
	template <typename Callable>
	call_on_init(Callable c)
	{
		c();
	}
};

#define TEST(test_name)																	\
void test_name();																		\
::call_on_init __##test_name##__register_test{ [](){ tests.emplace_back(&test_name); }};\
																						\
void test_name()																		\
																						\

int main()
{
	for (const auto & test : tests)
		test();
}

TEST(conversion_constructors)
{
	const json::value vi = 4;
	debug_assert(vi.is_int());
	debug_assert(vi.as_int() == 4);

	const json::value vu = 0u;
	debug_assert(vu.is_int());

	json::value vf = 3.141592;
	debug_assert(vf.is_real());
	debug_assert(vf.as_double() == 3.141592);
	debug_assert(vf.as_int() == 3);

	const json::value vb = true;
	debug_assert(vb.is_bool());
	debug_assert(vb.as_bool());

	vf = 2.78f;
	debug_assert(vf.is_real());

	const json::value vs = "Hello, world!";
	debug_assert(vs.is_string());
	debug_assert(vs.as_string() == "Hello, world!");
}

TEST(constructor_from_initializer_list)
{
	const json::value va = { 1, 2, 3, 4, 5 };
	debug_assert(va.is_array());
}

TEST(array_size)
{
	const json::value va = { 1, 2, 3, 4, 5 };
	debug_assert(va.as_array().size() == 5);
}

TEST(array_begin_end)
{
	const json::value va = { 1, 2, 3, 4, 5 };
	for (const auto & val : va.as_array())
		debug_assert(val.is_int());
}

TEST(array_subscript)
{
	const json::value va = { 1, 2.25, "Hello, world!", true, {1, 2, 3} };
	debug_assert(va.as_array()[0].is_int());
	debug_assert(va.as_array()[0].as_int() == 1);

	debug_assert(va.as_array()[1].is_real());
	debug_assert(va.as_array()[1].as_double() == 2.25);

	debug_assert(va.as_array()[2].is_string());
	debug_assert(va.as_array()[2].as_string() == "Hello, world!");

	debug_assert(va.as_array()[3].is_bool());
	debug_assert(va.as_array()[3].as_bool() == true);

	debug_assert(va.as_array()[4].is_array());
	debug_assert(va.as_array()[4].as_array().size() == 3);
	for (auto & v : va.as_array()[4].as_array())
		debug_assert(v.is_int());
}

TEST(array_at)
{
	const json::value va = { 1, 2.25, "Hello, world!", true,{ 1, 2, 3 } };
	try
	{
		debug_assert(va.as_array().at(4).is_array());
		debug_assert(va.as_array().at(5).is_null()); // This should throw
		debug_assert(false);
	}
	catch(std::out_of_range &)
	{}
}

TEST(array_reserve)
{
	json::value v{ json::value_type::array };
	v.as_array().reserve(25);
	debug_assert(v.is_array());
	debug_assert(v.as_array().capacity() == 25);
	debug_assert(v.as_array().size() == 0);
}

TEST(array_push_back)
{
	json::value v{ json::value_type::array };
	for (int i = 0; i < 10; ++i)
		v.as_array().push_back(i);

	debug_assert(v.is_array());
	debug_assert(v.as_array().size() == 10);
	for (int i = 0; i < 10; ++i)
	{
		debug_assert(v.as_array()[i].is_int());
		debug_assert(v.as_array()[i].as_int() == i);
	}
}

TEST(array_resize)
{
	json::value v{ json::value_type::array };
	v.as_array().resize(5);

	debug_assert(v.is_array());
	// New elements after resize should be null
	for (auto & i : v.as_array())
		debug_assert(!i);
}

TEST(array_resize_conserves_old_elements)
{
	json::value va = { 1, 2.25, "Hello, world!", true,{ 1, 2, 3 } };
	va.as_array().resize(10);

	// Check that old elements are still the same
	debug_assert(va.as_array()[0].is_int());
	debug_assert(va.as_array()[0].as_int() == 1);

	debug_assert(va.as_array()[1].is_real());
	debug_assert(va.as_array()[1].as_double() == 2.25);

	debug_assert(va.as_array()[2].is_string());
	debug_assert(va.as_array()[2].as_string() == "Hello, world!");

	debug_assert(va.as_array()[3].is_bool());
	debug_assert(va.as_array()[3].as_bool() == true);

	debug_assert(va.as_array()[4].is_array());
	debug_assert(va.as_array()[4].as_array().size() == 3);
	for (auto & v : va.as_array()[4].as_array())
		debug_assert(v.is_int());

	// Check that new elements are null
	for (size_t i = 5; i < 10; ++i)
		debug_assert(va.as_array()[i].is_null());
}

TEST(array_remove_index)
{
	json::value va = { 1, 2.25, "Hello, world!", true,{ 1, 2, 3 } };
	va.as_array().erase(va.as_array().begin());

	debug_assert(va.as_array()[0].is_real());
	debug_assert(va.as_array()[0].as_double() == 2.25);

	debug_assert(va.as_array()[1].is_string());
	debug_assert(va.as_array()[1].as_string() == "Hello, world!");

	debug_assert(va.as_array()[2].is_bool());
	debug_assert(va.as_array()[2].as_bool() == true);

	debug_assert(va.as_array()[3].is_array());
	debug_assert(va.as_array()[3].as_array().size() == 3);
	for (auto & v : va.as_array()[3].as_array())
		debug_assert(v.is_int());
}

TEST(swap_two_values_member_swap)
{
	json::value v1 = 5;
	json::value v2 = 3.141592;

	v1.swap(v2);

	debug_assert(v1.is_real());
	debug_assert(v1.as_double() == 3.141592);

	debug_assert(v2.is_int());
	debug_assert(v2.as_int() == 5);
}

TEST(swap_two_values_std_swap)
{
	json::value v1 = 5;
	json::value v2 = 3.141592;

	std::swap(v1, v2);

	debug_assert(v1.is_real());
	debug_assert(v1.as_double() == 3.141592);

	debug_assert(v2.is_int());
	debug_assert(v2.as_int() == 5);
}

TEST(operator_less)
{
	const json::value v1 = 5;
	const json::value v2 = 6;

	debug_assert(v1 < v2);
	debug_assert(!(v2 < v1));

	const json::value v3 = 5.5;
	const json::value v4 = 6u;

	debug_assert(v3 < v4);
	debug_assert(!(v4 < v3));
}

TEST(operator_equal)
{
	const json::value v1 = 5;
	const json::value v2 = 6;
	const json::value v3 = 6;

	debug_assert(v1 == v1);
	debug_assert(v2 == v3);
	debug_assert(!(v1 == v2));
	debug_assert(!(v1 == v3));

	const json::value v4 = "Hello, world!";
	const json::value v5 = "Hello world!";

	debug_assert(v4 == v4);
	debug_assert(v4 != v5);

	try
	{
		debug_assert(v1 < v4); // This should throw
		debug_assert(false);
	}
	catch (const std::exception &)
	{}
}

TEST(object_constructor_from_initializer_list_of_pairs)
{
	const json::value v = {
		std::make_pair( "Que te vayes", 4 ),
		std::make_pair( "Que te fueres", 3.141592 ),
		std::make_pair( "Y dame dinero", "Hello, world!" )
	};

	debug_assert(v.is_object());
	debug_assert(v.as_object().size() == 3);
}

TEST(clear_array_and_object)
{
	json::value va = { 1, 2.25, "Hello, world!", true,{ 1, 2, 3 } };

	debug_assert(!va.as_array().empty());

	va.as_array().clear();

	debug_assert(va.as_array().empty());

	json::value vo = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	debug_assert(!vo.as_object().empty());

	vo.as_object().clear();
	
	debug_assert(vo.as_object().empty());
}

TEST(operator_subscript_on_object_finds_by_name)
{
	const json::value v = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	const auto & v1 = v.as_object()["Que te vayes"];
	debug_assert(v1.is_int());
	debug_assert(v1.as_int() == 4);

	const auto & v2 = v.as_object()["Que te fueres"];
	debug_assert(v2.is_real());
	debug_assert(v2.as_double() == 3.141592);

	const auto & v3 = v.as_object()["Y dame dinero"];
	debug_assert(v3.is_string());
	debug_assert(v3.as_string() == "Hello, world!");
}

TEST(const_operator_subscript_returns_null_reference_on_failure)
{
	const json::value v = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	const auto & v1 = v.as_object()["This will fail"];
	debug_assert(v1.is_null());

	try
	{
		const json::value vi = 4;

		const auto & v3 = vi.as_object()["This will throw"];
		static_cast<void>(v3);
		debug_assert(false);
	}
	catch(const std::exception &)
	{}
}

TEST(mutable_operator_subscript_inserts_new_element_on_failure)
{
	json::value v = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	debug_assert(v.as_object().size() == 3);

	auto & new_member = v.as_object()["This will fail"];
	static_cast<void>(new_member);

	debug_assert(v.as_object().size() == 4);

	auto & new_member_again = v.as_object()["This will fail"];
	static_cast<void>(new_member_again);

	debug_assert(v.as_object().size() == 4);

	json::value v2{ json::value_type::object };
	v2.as_object()["New member"] = 4;
	debug_assert(v2.is_object());
	debug_assert(v2.as_object().size() == 1);
	debug_assert(v2.as_object()["New member"].as_int() == 4);
}

TEST(overload_of_subscript_operator_for_std_string)
{
	json::value v = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	const json::value cv = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	v.as_object()[std::string("Que te vayes")] = 25;
	cv.as_object()[std::string("Que te vayes")];
}

TEST(at_finds_member_by_key_or_throws_on_failure_regardless_of_constness)
{
	json::value v = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	debug_assert(v.as_object().at("Que te vayes").as_int() == 4);
	try
	{
		v.as_object().at(std::string("Fail"));
		debug_assert(false);
	}
	catch (const std::out_of_range &)
	{}

	const json::value cv = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	debug_assert(cv.as_object().at("Que te vayes").as_int() == 4);
	try
	{
		cv.as_object().at(std::string("Fail"));
		debug_assert(false);
	}
	catch (const std::out_of_range &)
	{}
}

TEST(at_with_null_value_throws_out_of_range)
{
	json::value v{ json::value_type::object };

	try
	{
		v.as_object().at(std::string("Fail"));
		debug_assert(false);
	}
	catch (const std::out_of_range &)
	{}
}

TEST(find_returns_an_iterator_to_the_member_or_end_on_failure)
{
	{
		json::value v = {
			std::make_pair("Que te vayes", 4),
			std::make_pair("Que te fueres", 3.141592),
			std::make_pair("Y dame dinero", "Hello, world!")
		};

		json::object::iterator it = v.as_object().find("Que te vayes");
		debug_assert(it->second.as_int() == 4);

		auto it_fail = v.as_object().find("Fail");
		debug_assert(it_fail == v.as_object().end());
	}
	{
		const json::value cv = {
			std::make_pair("Que te vayes", 4),
			std::make_pair("Que te fueres", 3.141592),
			std::make_pair("Y dame dinero", "Hello, world!")
		};

		json::object::const_iterator it = cv.as_object().find("Que te vayes");
		debug_assert(it->second.as_int() == 4);

		auto it_fail = cv.as_object().find("Fail");
		debug_assert(it_fail == cv.as_object().end());
	}
}

TEST(is_member_returns_true_if_a_member_with_that_key_exists)
{
	const json::value cv = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	debug_assert(cv.as_object().is_member("Que te vayes"));
	debug_assert(!cv.as_object().is_member(std::string{ "Totally not a member" }));
}

TEST(remove_member_removes_a_member_with_the_given_key_and_returns_true_on_success)
{
	json::value v = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	debug_assert(v.as_object().size() == 3);

	const bool s1 = v.as_object().remove_member("Que te vayes");
	const bool s2 = v.as_object().remove_member(std::string{ "Que te vayes" });

	debug_assert(s1);
	debug_assert(!s2);
	debug_assert(v.as_object().size() == 2);
}

TEST(member_names_returns_the_names_of_all_members_in_alphabetical_order)
{
	const json::value cv = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 3.141592),
		std::make_pair("Y dame dinero", "Hello, world!")
	};

	const json::vector<json::string> member_names = cv.as_object().member_names();
	debug_assert(member_names.size() == 3);
	debug_assert(member_names[0] == "Que te fueres");
	debug_assert(member_names[1] == "Que te vayes");
	debug_assert(member_names[2] == "Y dame dinero");
}

TEST(insert_adds_an_element_to_an_object_value)
{
	json::value v{ json::value_type::object };
	json::object::iterator it = v.as_object().insert("Que te vayes", 4);
	debug_assert(v.is_object());
	debug_assert(v.as_object().size() == 1);
	debug_assert(it->second.as_int() == 4);

	it = v.as_object().insert("Que te vayes", "Hello");
	debug_assert(v.as_object().size() == 1);
	debug_assert(it == v.as_object().end());
}

TEST(emplace_inserts_an_element_and_constructs_it_in_place_from_variadic_arguments)
{
	json::value v{ json::value_type::object };
	json::object::iterator it = v.as_object().emplace("Que te vayes", 4);
	debug_assert(v.is_object());
	debug_assert(v.as_object().size() == 1);
	debug_assert(it->second.as_int() == 4);

	it = v.as_object().emplace("Que te vayes", "Hello");
	debug_assert(v.as_object().size() == 1);
	debug_assert(it == v.as_object().end());
}

TEST(begin_and_end_work_on_objects)
{
	const json::value cv = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 5),
		std::make_pair("Y dame dinero", 6)
	};

	for (auto & i : cv.as_object())
		debug_assert(i.second.is_int());
}

TEST(iterators_on_objects_are_bidirectional)
{
	const json::value cv = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 5),
		std::make_pair("Y dame dinero", 6)
	};

	for (auto it = cv.as_object().rbegin(); it != cv.as_object().rend(); ++it)
		debug_assert(it->second.is_int());

	auto it = cv.as_object().end();
	--it;
	debug_assert(it->second.is_int());
}

TEST(can_access_key_of_a_member_from_object_iterator)
{
	const json::value cv = {
		std::make_pair("Que te vayes", 4),
		std::make_pair("Que te fueres", 5),
		std::make_pair("Y dame dinero", 6)
	};

	auto it = cv.as_object().begin();
	debug_assert(it->first == "Que te fueres");
	++it;
	debug_assert(it->first == "Que te vayes");
	++it;
	debug_assert(it->first == "Y dame dinero");
}

TEST(assignment_operator_from_the_value_types)
{
	json::value v;

	v = 4;
	debug_assert(v.is_int());
	debug_assert(v.as_int() == 4);

	v = 42u;
	debug_assert(v.is_int());
	debug_assert(v.as_uint() == 42u);

	v = 3.141592;
	debug_assert(v.is_real());
	debug_assert(v.as_double() == 3.141592);

	v = "Hello, world!";
	debug_assert(v.is_string());
	debug_assert(v.as_string() == "Hello, world!");

	v = json::string("Foobar");
	debug_assert(v.is_string());
	debug_assert(v.as_string() == "Foobar");
}

TEST(reader_can_tokenize_structural_characters)
{
	std::string source = "{}[]:,";
	std::vector<json::parser::token<const char *>> tokens = json::parser::tokenize(source);
	debug_assert(tokens.size() == 6);
	
	debug_assert(tokens[0].type == json::parser::token_type::open_curly_brace);
	debug_assert(tokens[0].begin - source.c_str() == 0);
	debug_assert(tokens[0].end - source.c_str() == 1);

	debug_assert(tokens[1].type == json::parser::token_type::close_curly_brace);
	debug_assert(tokens[1].begin - source.c_str() == 1);
	debug_assert(tokens[1].end - source.c_str() == 2);
	
	debug_assert(tokens[2].type == json::parser::token_type::open_square_bracket);
	debug_assert(tokens[3].type == json::parser::token_type::close_square_bracket);
	debug_assert(tokens[4].type == json::parser::token_type::colon);
	debug_assert(tokens[5].type == json::parser::token_type::comma);
}

TEST(reader_can_tokenize_cpp_comments)
{
	const std::string source = "//{}[]:,\n{}";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 3);
	debug_assert(tokens[0].type == json::parser::token_type::comment);
	debug_assert(tokens[0].begin - source.c_str() == 0);
	debug_assert(tokens[0].end - source.c_str() == 8);

	debug_assert(tokens[1].type == json::parser::token_type::open_curly_brace);
	debug_assert(tokens[1].begin - source.c_str() == 9);
	debug_assert(tokens[2].type == json::parser::token_type::close_curly_brace);
	debug_assert(tokens[2].begin - source.c_str() == 10);
}

TEST(reader_can_tokenize_c_comments)
{
	const std::string source = "/*{}[]:,*/{}";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 3);
	debug_assert(tokens[0].type == json::parser::token_type::comment);
	debug_assert(tokens[0].begin - source.c_str() == 0);
	debug_assert(tokens[0].end - source.c_str() == 10);

	debug_assert(tokens[1].type == json::parser::token_type::open_curly_brace);
	debug_assert(tokens[1].begin - source.c_str() == 10);
	debug_assert(tokens[2].type == json::parser::token_type::close_curly_brace);
	debug_assert(tokens[2].begin - source.c_str() == 11);
}

TEST(a_slash_that_is_not_part_of_a_comment_throws)
{
	try
	{
		const std::string source = "/{}[]:,*/{}";
		const auto tokens = json::parser::tokenize(source); // This should throw
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch(const json::parser::syntax_error &)
	{}
}

TEST(an_unclosed_c_comment_throws)
{
	try
	{
		const std::string source = "/*{}[]:,{}";
		const auto tokens = json::parser::tokenize(source); // This should throw
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}
}

TEST(cpp_style_comment_length_is_correctly_detected_when_the_last_line_is_commented)
{
	const std::string source = "//{}[]:,{}";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 1);
	debug_assert(tokens[0].type == json::parser::token_type::comment);
	debug_assert(tokens[0].begin - source.c_str() == 0);
	debug_assert(tokens[0].end - source.c_str() == 10);
}

TEST(can_tokenize_strings_without_control_characters)
{
	const std::string source = "{}  \"En un lugar de la Mancha\"   {}";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 5);
	debug_assert(tokens[0].type == json::parser::token_type::open_curly_brace);
	debug_assert(tokens[1].type == json::parser::token_type::close_curly_brace);
	debug_assert(tokens[2].type == json::parser::token_type::string);
	debug_assert(tokens[3].type == json::parser::token_type::open_curly_brace);
	debug_assert(tokens[4].type == json::parser::token_type::close_curly_brace);

	debug_assert(tokens[2].begin - source.c_str() == 4);
	debug_assert(tokens[2].end - source.c_str() == 30);
}

TEST(can_tokenize_strings_with_control_characters)
{
	const std::string source = "\"En un lugar \\n de \\\" la Mancha\"";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 1);
	debug_assert(tokens[0].type == json::parser::token_type::string);
	debug_assert(tokens[0].end - tokens[0].begin == 32);
}

TEST(an_unfinished_string_throws)
{
	try
	{
		const std::string source = "{}  \"En un lugar de la Mancha   {}";
		const auto tokens = json::parser::tokenize(source); // This should throw
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}
}

TEST(unfinished_string_with_control_character_quotation_should_throw)
{
	try
	{
		const std::string source = "{}  \"En un lugar \\\" de la Mancha \\\"  {}";
		const auto tokens = json::parser::tokenize(source); // This should throw
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}
}

TEST(character_not_corresponding_to_any_token_type_throws)
{
	try
	{
		const std::string source = "{}  \"En un lugar de la Mancha\" unsigned {}";
		const auto tokens = json::parser::tokenize(source); // This should throw
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}
}

TEST(can_parse_unsigned_integer_number_tokens)
{
	const std::string source = "37 23 2048 1";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 4);
	for (auto & token : tokens)
		debug_assert(token.type == json::parser::token_type::number);

	debug_assert(std::string(tokens[0].begin, tokens[0].end) == "37");
	debug_assert(std::string(tokens[1].begin, tokens[1].end) == "23");
	debug_assert(std::string(tokens[2].begin, tokens[2].end) == "2048");
	debug_assert(std::string(tokens[3].begin, tokens[3].end) == "1");
}

TEST(can_parse_signed_integer_number_tokens)
{
	const std::string source = "37 -23 2048 -1";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 4);
	for (auto & token : tokens)
		debug_assert(token.type == json::parser::token_type::number);

	debug_assert(std::string(tokens[0].begin, tokens[0].end) == "37");
	debug_assert(std::string(tokens[1].begin, tokens[1].end) == "-23");
	debug_assert(std::string(tokens[2].begin, tokens[2].end) == "2048");
	debug_assert(std::string(tokens[3].begin, tokens[3].end) == "-1");
}

TEST(can_parse_floating_point_number_tokens)
{
	const std::string source = "3.141592\n-2.25";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 2);
	for (auto & token : tokens)
		debug_assert(token.type == json::parser::token_type::number);

	debug_assert(std::string(tokens[0].begin, tokens[0].end) == "3.141592");
	debug_assert(std::string(tokens[1].begin, tokens[1].end) == "-2.25");
}

TEST(floating_point_with_two_dots_is_an_error)
{
	try
	{
		const std::string source = "3.141592.25";
		const auto tokens = json::parser::tokenize(source); // This should throw
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch(const json::parser::syntax_error &)
	{}
}

TEST(number_literal_ending_in_non_delimiter_character_is_a_syntax_error)
{
	try
	{
		const std::string source = "3.141592f 25u";
		const auto tokens = json::parser::tokenize(source); // This should throw because f is not a delimiter
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}
}

TEST(can_parse_boolean_literals_true_and_false)
{
	const std::string source = "true 25 [ } , : false []";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 9);
	debug_assert(tokens[0].type == json::parser::token_type::boolean);
	debug_assert(std::string(tokens[0].begin, tokens[0].end) == "true");
	debug_assert(tokens[6].type == json::parser::token_type::boolean);
	debug_assert(std::string(tokens[6].begin, tokens[6].end) == "false");
}

TEST(boolean_literals_not_followed_by_a_delimiter_are_a_syntax_error)
{
	try
	{
		const std::string source = "trueue";
		const auto tokens = json::parser::tokenize(source); // This should throw because
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}

	try
	{
		const std::string source = "false.24";
		const auto tokens = json::parser::tokenize(source); // This should throw because
		static_cast<void>(tokens);
		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}
}

TEST(can_parse_null_keyword)
{
	const std::string source = "true 25 [ } null , : false []";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 10);
	debug_assert(tokens[4].type == json::parser::token_type::null);
	debug_assert(std::string(tokens[4].begin, tokens[4].end) == "null");
}

TEST(can_parse_exponential_number_literals)
{
	const std::string source = "37e24 -1.367E-1 9e+3";
	const auto tokens = json::parser::tokenize(source);

	debug_assert(tokens.size() == 3);
	for (auto & token : tokens)
		debug_assert(token.type == json::parser::token_type::number);

	debug_assert(std::string(tokens[0].begin, tokens[0].end) == "37e24");
	debug_assert(std::string(tokens[1].begin, tokens[1].end) == "-1.367E-1");
	debug_assert(std::string(tokens[2].begin, tokens[2].end) == "9e+3");
}

TEST(can_construct_empty_object_from_string)
{
	const std::string source = "{}";
	const json::value val = json::parser::parse(source);

	debug_assert(val.is_object());
	debug_assert(val.as_object().empty());
}

TEST(can_construct_empty_array_from_string)
{
	const std::string source = "[]";
	const json::value val = json::parser::parse(source);

	debug_assert(val.is_array());
	debug_assert(val.as_array().empty());
}

TEST(can_construct_boolean_value_from_string)
{
	const json::value val = json::parser::parse("true");

	debug_assert(val.is_bool());
	debug_assert(val.as_bool() == true);

	const json::value val2 = json::parser::parse("false");

	debug_assert(val2.is_bool());
	debug_assert(val2.as_bool() == false);
}

TEST(can_construct_null_value_from_string)
{
	const json::value val = json::parser::parse("null");

	debug_assert(val.is_null());
}

TEST(can_construct_integer_value_from_string)
{
	const json::value val = json::parser::parse("256");

	debug_assert(val.is_int());
	debug_assert(val.as_int() == 256);

	const json::value val2 = json::parser::parse("-256");

	debug_assert(val2.is_int());
	debug_assert(val2.as_int() == -256);
}

TEST(can_construct_floating_point_value_from_string)
{
	const json::value val = json::parser::parse("3.141592");

	debug_assert(val.is_real());
	debug_assert(val.as_double() == 3.141592);
}

TEST(can_read_numeric_value_with_exponent)
{
	const json::value val = json::parser::parse("3.141592e4");

	debug_assert(val.is_real());
	debug_assert(val.as_double() == 3.141592e4);
}

TEST(can_construct_string_value_without_control_characters)
{
	const json::value val = json::parser::parse("\"Que te vayes\"");

	debug_assert(val.is_string());
	debug_assert(val.as_string() == "Que te vayes");
}

TEST(can_construct_string_value_with_control_characters)
{
	const json::value val = json::parser::parse("\"Que\\\\ te\\t vayes\\n\"");

	debug_assert(val.is_string());
	debug_assert(val.as_string() == "Que\\ te\t vayes\n");
}

TEST(can_construct_array_with_one_element)
{
	const json::value val = json::parser::parse("[ 3]");

	debug_assert(val.is_array());
	debug_assert(val.as_array().size() == 1);
	debug_assert(val.as_array()[0].as_int() == 3);
}

TEST(can_construct_array_with_n_elements)
{
	const json::value val = json::parser::parse("[ 3, 4, 5, 6]");

	debug_assert(val.is_array());
	debug_assert(val.as_array().size() == 4);
	debug_assert(val.as_array()[0].as_int() == 3);
	debug_assert(val.as_array()[1].as_int() == 4);
	debug_assert(val.as_array()[2].as_int() == 5);
	debug_assert(val.as_array()[3].as_int() == 6);
}

TEST(throws_syntax_error_on_invalid_arrays)
{
	try
	{
		// Missing end ]
		json::parser::parse("[ 1, 2, 3, 4");
	}
	catch (const json::parser::syntax_error &)
	{}

	try
	{
		// Missing ,
		json::parser::parse("[ 1, 2 3, 4 ]");
	}
	catch (const json::parser::syntax_error &)
	{}

	try
	{
		// Missing end ] for empty array
		json::parser::parse("[");
	}
	catch (const json::parser::syntax_error &)
	{}
}

TEST(parse_array_that_contains_an_array)
{
	const std::string source =
		"[ 1, false, null, "
		"[ 1, 3, 4 ],"
		"\"Hello!\" ]";

	const json::value val = json::parser::parse(source);

	debug_assert(val.is_array());
	debug_assert(val.as_array().size() == 5);

	debug_assert(val.as_array()[0].as_int() == 1);
	debug_assert(val.as_array()[1].as_bool() == false);
	debug_assert(val.as_array()[2].is_null());
	debug_assert(val.as_array()[3].is_array());
	debug_assert(val.as_array()[3].as_array().size() == 3);
	debug_assert(val.as_array()[4].as_string() == "Hello!");
}

TEST(parse_object_that_containts_one_member)
{
	const json::value val = json::parser::parse("{ \"foo\" : 45 }");

	debug_assert(val.is_object());
	debug_assert(val.as_object().size() == 1);
	debug_assert(val.as_object()["foo"].as_int() == 45);
}

TEST(parse_object_with_n_members)
{
	const std::string source = "{ "
		"\"foo\" : 3, "
		"\"bar\" : true, "
		"\"baz\" : null, "
		"\"zzz\" : [ 1, 2, 3 ] "
		"}";

	const json::value val = json::parser::parse(source);

	try
	{
		debug_assert(val.is_object());
		debug_assert(val.as_object().at("foo").as_int() == 3);
		debug_assert(val.as_object().at("bar").as_bool() == true);
		debug_assert(val.as_object().at("baz").is_null());
		const json::value zzz = val.as_object().at("zzz");
		debug_assert(zzz.is_array());
		debug_assert(zzz.as_array().size() == 3);
		for (const auto & i : zzz.as_array())
			debug_assert(i.is_int());
	}
	catch (const std::out_of_range &)
	{
		debug_assert(false);
	}
}

TEST(parse_from_stream)
{
	const std::string source = "{ "
		"\"foo\" : 3, "
		"\"bar\" : true, "
		"\"baz\" : null, "
		"\"zzz\" : [ 1, 2, 3 ] "
		"}";

	std::istringstream is{ source };
	const json::value val = json::parser::parse(is);

	try
	{
		debug_assert(val.is_object());
		debug_assert(val.as_object().at("foo").as_int() == 3);
		debug_assert(val.as_object().at("bar").as_bool() == true);
		debug_assert(val.as_object().at("baz").is_null());
		const json::value zzz = val.as_object().at("zzz");
		debug_assert(zzz.is_array());
		debug_assert(zzz.as_array().size() == 3);
		for (const auto & i : zzz.as_array())
			debug_assert(i.is_int());
	}
	catch (const std::out_of_range &)
	{
		debug_assert(false);
	}
}

TEST(correctly_detect_syntax_errors_on_objects)
{
	try
	{
		// Missing end }
		const std::string source = "{ "
			"\"foo\" : 3, "
			"\"bar\" : true, "
			"\"baz\" : null, "
			"\"zzz\" : [ 1, 2, 3 ] ";

		json::parser::parse(source);

		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}

	try
	{
		// Missing :
		const std::string source = "{ "
			"\"foo\" : 3, "
			"\"bar\" true, "
			"\"baz\" : null, "
			"\"zzz\" : [ 1, 2, 3 ] "
			"}";

		json::parser::parse(source);

		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}

	try
	{
		// int as key
		const std::string source = "{ "
			"\"foo\" : 3, "
			"23      : true, "
			"\"baz\" : null, "
			"\"zzz\" : [ 1, 2, 3 ] "
			"}";

		json::parser::parse(source);

		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}

	try
	{
		// Incomplete source
		const std::string source = "{ "
			"\"foo\" : 3, "
			"\"bar\" : ";

		json::parser::parse(source);

		debug_assert(false);
	}
	catch(const json::parser::syntax_error &)
	{}
}

TEST(parse_empty_string)
{
	try
	{
		json::parser::parse("");
		debug_assert(false);
	}
	catch (const json::parser::syntax_error &)
	{}
}

TEST(comments_dont_break_parsing)
{
	const json::value val = json::parser::parse("/* This number represents the meaning of life */ 42");
	
	debug_assert(val.is_int());
	debug_assert(val.as_int() == 42);
}

TEST(comments_when_parsing_object_dont_break_parsing)
{
	const std::string source = 
		"// This is a test object with comments\n"
		"{ "
		"/* foo is a meaningless name often given to test or throwaway code */ \"foo\" : 3, "
		"\"bar\" : /* This must be true */ true, "
		"\"baz\" : null /* It's important to have a comma after a key-value pair */, "
		"\"zzz\" /* Not having the colon breaks too */ : [ 1, 2, 3 ] "
		"}";

	try
	{
		const json::value val = json::parser::parse(source);

		try
		{
			debug_assert(val.is_object());
			debug_assert(val.as_object().at("foo").as_int() == 3);
			debug_assert(val.as_object().at("bar").as_bool() == true);
			debug_assert(val.as_object().at("baz").is_null());
			const json::value zzz = val.as_object().at("zzz");
			debug_assert(zzz.is_array());
			debug_assert(zzz.as_array().size() == 3);
			for (const auto & i : zzz.as_array())
				debug_assert(i.is_int());
		}
		catch (const std::out_of_range &)
		{
			debug_assert(false);
		}
	}
	catch (const json::parser::syntax_error &)
	{
		debug_assert(false);
	}
}

TEST(comments_when_parsing_array_dont_break_parsing)
{
	const std::string source =
		"// We are parsing an array\n"
		"[ /* This is a number */ 1 /* Comma after the value */, false, null, "
		"[ 1, 3, 4 ],"
		"\"Hello!\" /* End with closing square bracket */ ]";

	const json::value val = json::parser::parse(source);

	debug_assert(val.is_array());
	debug_assert(val.as_array().size() == 5);

	debug_assert(val.as_array()[0].as_int() == 1);
	debug_assert(val.as_array()[1].as_bool() == false);
	debug_assert(val.as_array()[2].is_null());
	debug_assert(val.as_array()[3].is_array());
	debug_assert(val.as_array()[3].as_array().size() == 3);
	debug_assert(val.as_array()[4].as_string() == "Hello!");
}

TEST(parse_control_character_u_in_string)
{
	const json::value val = json::parser::parse("\"\u0020\"");

	debug_assert(val.is_string());
	debug_assert(val.as_c_string()[0] == 0x20);
}

TEST(can_write_int_value_to_string)
{
	const json::value val = -356;
	const json::string str = json::writer::write(val);

	debug_assert(str == "-356");
}

TEST(can_use_any_iterator_for_build_value)
{
	const auto tokens = json::parser::tokenize("[ 1, 2, 3, 4, 5 ]");

	// Build value from vector<token>::iterator
	const json::value v1 = json::parser::build_value(tokens.begin(), tokens.end());
	// Build value from token *
	const json::value v2 = json::parser::build_value(&tokens.front(), &tokens.back() + 1);
	
	const std::list<json::parser::token<const char *>> token_list{ tokens.begin(), tokens.end() };
	// Build value form list<token>::iterator
	const json::value v3 = json::parser::build_value(token_list.begin(), token_list.end());

	const std::forward_list<json::parser::token<const char *>> token_flist{ tokens.begin(), tokens.end() };
	// Build value form forward_list<token>::iterator
	const json::value v4 = json::parser::build_value(token_flist.begin(), token_flist.end());

	debug_assert(v1 == v2);
	debug_assert(v1 == v3);
	debug_assert(v1 == v4);
}

TEST(conversion_constructor_from_json_array)
{
	json::array a = { 1, true, 3.141592, "Hello" };
	const json::value va = a;
	debug_assert(!a.empty());
	
	debug_assert(va.is_array());
	const json::array & ra = va.as_array();
	debug_assert(ra.size() == 4);
	debug_assert(ra[0].as_int() == 1);
	debug_assert(ra[1].as_bool() == true);
	debug_assert(ra[2].as_double() == 3.141592);
	debug_assert(ra[3].as_string() == "Hello");

	const json::value va2 = std::move(a);
	debug_assert(va == va2);
	debug_assert(a.empty());
}

TEST(conversion_constructor_from_json_object)
{
	json::object o;
	o.insert("a", 3);
	o.insert("b", false);
	o.insert("c", { 1, 2, 3 });

	const json::value vo = o;
	debug_assert(vo.is_object());
	const json::object & obj = vo.as_object();

	debug_assert(!o.empty());
	debug_assert(obj.at("a").as_int() == 3);
	debug_assert(obj.at("b").as_bool() == false);
	debug_assert(obj.at("c") == json::value({ 1, 2, 3 }));

	const json::value vo2 = std::move(o);
	debug_assert(o.empty());
	debug_assert(vo == vo2);
}

TEST(assignment_from_json_array)
{
	json::array a = { 1, true, 3.141592, "Hello" };
	json::value va;
	va = a;
	debug_assert(!a.empty());

	debug_assert(va.is_array());
	const json::array & ra = va.as_array();
	debug_assert(ra.size() == 4);
	debug_assert(ra[0].as_int() == 1);
	debug_assert(ra[1].as_bool() == true);
	debug_assert(ra[2].as_double() == 3.141592);
	debug_assert(ra[3].as_string() == "Hello");

	json::value va2;
	va2 = std::move(a);
	debug_assert(va == va2);
	debug_assert(a.empty());
}

TEST(assignment_from_json_object)
{
	json::object o;
	o.insert("a", 3);
	o.insert("b", false);
	o.insert("c", { 1, 2, 3 });

	json::value vo;
	vo = o;
	debug_assert(vo.is_object());
	const json::object & obj = vo.as_object();

	debug_assert(!o.empty());
	debug_assert(obj.at("a").as_int() == 3);
	debug_assert(obj.at("b").as_bool() == false);
	debug_assert(obj.at("c") == json::value({ 1, 2, 3 }));

	json::value vo2;
	vo2 = std::move(o);
	debug_assert(o.empty());
	debug_assert(vo == vo2);
}

TEST(type_returns_an_element_from_the_value_type_enum_representing_the_type_of_the_value)
{
	const json::value v = 5;
	debug_assert(v.type() == json::value_type::integer);

	const json::value v2 = 3.141592;
	debug_assert(v2.type() == json::value_type::real);

	const json::value v3 = false;
	debug_assert(v3.type() == json::value_type::boolean);

	const json::value v4 = "Howdy!";
	debug_assert(v4.type() == json::value_type::string);

	const json::value v5 = { 1, 2, 3 };
	debug_assert(v5.type() == json::value_type::array);

	const json::value v6{ json::value_type::object };
	debug_assert(v6.type() == json::value_type::object);

	const json::value v7;
	debug_assert(v7.type() == json::value_type::null);
}

TEST(write_can_correctly_write_real_values)
{
	json::value val = 3.141592;
	json::string str = json::writer::write(val);
	debug_assert(str == "3.141592");
	debug_assert(atof(str.c_str()) == val.as_double());

	val = -2.25;
	str = json::writer::write(val);
	debug_assert(str == "-2.25");
	debug_assert(atof(str.c_str()) == val.as_double());

	val = 3.0;
	str = json::writer::write(val);
	debug_assert(str == "3.");
	debug_assert(atof(str.c_str()) == val.as_double());
}

TEST(write_works_on_booleans)
{
	json::value val = true;
	json::string str = json::writer::write(val);
	debug_assert(str == "true");

	val = false;
	str = json::writer::write(val);
	debug_assert(str == "false");
}

TEST(write_works_on_null_values)
{
	const json::string str = json::writer::write(json::value::null);
	debug_assert(str == "null");
}

TEST(write_works_on_strings_without_control_characters)
{
	const json::value val = "Hello, world!";
	const json::string str = json::writer::write(val);
	debug_assert(str == "\"Hello, world!\"");
}

TEST(write_works_on_strings_with_control_characters)
{
	const json::value val = "\"Hello, world!\"";
	const json::string str = json::writer::write(val);
	debug_assert(str == "\"\\\"Hello, world!\\\"\"");
}

TEST(write_works_on_strings_with_control_characters_representing_non_printable_characters)
{
	const json::value val = "\tHello, world!\n";
	const json::string str = json::writer::write(val);
	debug_assert(str == "\"\\tHello, world!\\n\"");
}

TEST(con_write_array_of_non_array_non_object_values)
{
	const json::value val = { 1, 2.25, true, "Hello!" };
	const json::string str = json::writer::write(val);

	const auto expected_output =
		"[\n"
		"\t1,\n"
		"\t2.25,\n"
		"\ttrue,\n"
		"\t\"Hello!\"\n"
		"]";

	debug_assert(str == expected_output);

	// Check for empty array
	debug_assert(json::writer::write(json::value{ json::value_type::array }) == "[]");
}

TEST(can_write_array_that_contains_an_array)
{
	const json::value val = { 1, 2.25, true, "Hello!", { 1, 2, 3 } };
	const json::string str = json::writer::write(val);

	const auto expected_output =
		"[\n"
		"\t1,\n"
		"\t2.25,\n"
		"\ttrue,\n"
		"\t\"Hello!\",\n"
		"\t[\n"
		"\t\t1,\n"
		"\t\t2,\n"
		"\t\t3\n"
		"\t]\n"
		"]";

	debug_assert(str == expected_output);
}

TEST(can_configure_indentation_so_that_people_that_prefer_spaces_over_tabs_can_be_happy_in_their_mistake)
{
	const json::value val = { 1, 2.25, true, "Hello!", { 1, 2, 3 } };
	const json::string str = json::writer::write(val, "    ");

	const auto expected_output =
		"[\n"
		"    1,\n"
		"    2.25,\n"
		"    true,\n"
		"    \"Hello!\",\n"
		"    [\n"
		"        1,\n"
		"        2,\n"
		"        3\n"
		"    ]\n"
		"]";

	debug_assert(str == expected_output);
}

TEST(write_object_with_no_arrays_or_objects)
{
	const json::value val = {
		std::make_pair("foo", 3),
		{ "bar", false },
		{ "baz", 3.141592 }
	};
	const json::string str = json::writer::write(val, "    ");

	const auto expected_output =
		"{\n"
		"    \"bar\" : false,\n"
		"    \"baz\" : 3.141592,\n"
		"    \"foo\" : 3\n"
		"}";

	debug_assert(str == expected_output);

	// Test for empty object
	debug_assert(json::writer::write(json::value{ json::value_type::object }) == "{}");
}

TEST(write_object_that_containt_an_array)
{
	const json::value val = {
		std::make_pair("baz", 3.141592),
		{ "bar", false },
		{ "foo", { 1, 2, 3 } }
	};
	const json::string str = json::writer::write(val, "    ");

	const auto expected_output =
		"{\n"
		"    \"bar\" : false,\n"
		"    \"baz\" : 3.141592,\n"
		"    \"foo\" : \n"
		"    [\n"
		"        1,\n"
		"        2,\n"
		"        3\n"
		"    ]\n"
		"}";

	debug_assert(str == expected_output);
}

TEST(write_directly_to_stream)
{
	const json::value val = {
		std::make_pair("baz", 3.141592),
		{ "bar", false },
		{ "foo",{ 1, 2, 3 } }
	};

	const auto expected_output =
		"{\n"
		"    \"bar\" : false,\n"
		"    \"baz\" : 3.141592,\n"
		"    \"foo\" : \n"
		"    [\n"
		"        1,\n"
		"        2,\n"
		"        3\n"
		"    ]\n"
		"}";

	std::ostringstream oss;
	json::writer::write(oss, val, "    ");

	debug_assert(oss.str() == expected_output);
}

TEST(streaming_operators_as_alternative_to_write_and_parse_functions)
{
	const json::value val = {
		std::make_pair("baz", 3.141592),
		{ "bar", false },
		{ "foo",{ 1, 2, 3 } }
	};

	json::value other_val;

	std::stringstream ss;
	ss << val;
	ss >> other_val;

	debug_assert(val == other_val);
}

TEST(counting_number_of_elements_in_an_array)
{
	const auto tokens1 = json::parser::tokenize("[ 1, 2, 3, 4, 5 ]");
	debug_assert(json::parser::impl::count_array_elements(tokens1.begin() + 1, tokens1.end()) == 5);

	const auto tokens2 = json::parser::tokenize("[ 1 ]");
	debug_assert(json::parser::impl::count_array_elements(tokens2.begin() + 1, tokens2.end()) == 1);

	const auto source =
		"[ 1, false, null, "
		"[ 1, 3, 4 ],"
		"\"Hello!\", "
		"{ \"foo\" : null, \"bar\" : true }"
		"]";

	const auto tokens3 = json::parser::tokenize(source);
	debug_assert(json::parser::impl::count_array_elements(tokens3.begin() + 1, tokens3.end()) == 6);
}

TEST(insertion_via_subscript_with_static_string_performs_no_allocation)
{
	json::object obj;
	obj[json::static_string("foo")] = 4;
	obj["bar"] = 4;
	obj["some pretty long string"] = 4;

	debug_assert(obj.find("foo")->first.is_view() == true);
	debug_assert(obj.find("bar")->first.is_view() == false);
	debug_assert(obj.find("some pretty long string")->first.is_view() == false);
}

TEST(points_inside_returns_true_if_a_pointer_points_inside_an_array)
{
	const char array[] = "Imagine all the people, living for today";
	const char * const p1 = array + 5;
	const char * const p2 = array;
	const char * const p3 = array + sizeof(array) - 1;
	const char * const p4 = array + sizeof(array);
	const char * const p5 = array + 2048;
	const char * const p6 = nullptr;

	debug_assert(json::points_inside(p1, array) == true);
	debug_assert(json::points_inside(p2, array) == true);
	debug_assert(json::points_inside(p3, array) == true);
	debug_assert(json::points_inside(p4, array) == false);
	debug_assert(json::points_inside(p5, array) == false);
	debug_assert(json::points_inside(p6, array) == false);
}

TEST(release_returns_the_content_of_a_string_and_makes_it_empty)
{
	std::string str = "A pretty long string that doesn't fit short string optimization";
	char * p = json::release(str);

	debug_assert(str.empty());
	debug_assert(p == std::string{ "A pretty long string that doesn't fit short string optimization" });

	delete[] p;
}

TEST(release_returns_null_with_a_string_optimized_with_short_string_optimization)
{
	std::string str = "Short";
	char * p = json::release(str);

	debug_assert(!str.empty());
	debug_assert(p == nullptr);
}

TEST(release_returns_the_content_of_a_vector_and_makes_it_empty)
{
	const std::initializer_list<int> ilist = { 1, 2, 3, 4, 5 };
	std::vector<int> v = ilist;
	auto p = json::release(v);

	debug_assert(v.empty());
	debug_assert(std::equal(ilist.begin(), ilist.end(), p, p + ilist.size()));

	delete[] p;
}

TEST(release_returns_a_null_pointer_for_an_empty_vector)
{
	std::vector<int> v;
	auto p = json::release(v);

	debug_assert(p == nullptr);
}

TEST(release_with_unique_ptr_calls_member_release_function)
{
	std::unique_ptr<int> up{ new int(3) };
	auto p = json::release(up);

	debug_assert(up == nullptr);
	debug_assert(*p == 3);

	delete p;
}

TEST(release_with_unique_ptr_of_arrays_works_correctly)
{
	std::unique_ptr<int[]> up{ new int[4] };
	up[2] = 4;
	int * p = json::release(up);

	debug_assert(up == nullptr);
	debug_assert(p[2] == 4);

	delete[] p;
}

TEST(a_string_key_that_is_not_a_short_string_is_a_short_empty_string_after_move)
{
	const std::string src = "There's a lady who's sure all that glitters is gold and she's buying a stairway to heaven";
	json::string_key s = src;
	json::string_key s2 = std::move(s);

	debug_assert(s2 == src);
	debug_assert(s.empty());
	debug_assert(s.size() == 0);
	debug_assert(s.capacity() == 15);

	debug_assert(sizeof(json::string_key) == sizeof(std::string));
}

TEST(a_string_key_and_a_std_string_of_the_same_allocator_have_the_same_size)
{
	debug_assert(sizeof(json::string_key) == sizeof(json::string));
}

TEST(a_string_key_with_wchar_also_implements_short_string_optimization)
{
	using wstring_key = json::basic_string_key<wchar_t>;

	const wstring_key s1 = L"short";
	wstring_key s2 = L"A longer string";

	debug_assert(json::points_inside(s1.c_str(), s1) == true);
	debug_assert(json::points_inside(s2.c_str(), s2) == false);
	
	wstring_key s3 = std::move(s2);

	debug_assert(json::points_inside(s2.c_str(), s2) == true);
	debug_assert(json::points_inside(s3.c_str(), s3) == false);
}

TEST(string_key_move_constructor_from_std_string)
{
	std::string short_str = "Short";
	std::string long_str = "A quite longer string to avoid short string optimization";

	const json::string_key s1 = std::move(short_str);
	const json::string_key s2 = std::move(long_str);

	debug_assert(s1 == "Short");
	debug_assert(s2 == "A quite longer string to avoid short string optimization");
	debug_assert(short_str.empty() == false);
	debug_assert(long_str.empty() == true);
}

TEST(substr_with_no_arguments_returns_a_copy_of_a_string_key)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.substr() == s);
}

TEST(first_argument_of_substr_defines_starting_position)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.substr(7) == "ugar de la Mancha");
}

TEST(second_argument_of_substr_defines_the_length_of_the_substring)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.substr(7, 6) == "ugar d");
}

TEST(a_substr_that_requires_past_the_end_characters_stops_at_the_end_of_the_string)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.substr(10, 200) == "r de la Mancha");
}

TEST(substr_with_pos_out_of_range_returns_empty_string)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.substr(200, 5) == "");
}

TEST(find_returns_index_of_the_first_occurrence_of_a_substring_in_a_string_key)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.find("En un") == 0);
	debug_assert(s.find(std::string{ "lugar" }) == 6);
	debug_assert(s.find('M') == 18);
	debug_assert(s.find('n', 3) == 4);
	debug_assert(s.find("foo") == json::string_key::npos);
}

TEST(rfind_returns_index_of_the_last_occurrence_of_a_substring_in_a_string_key)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.rfind("En un") == 0);
	debug_assert(s.rfind(std::string{ "lugar" }) == 6);
	debug_assert(s.rfind('M') == 18);
	debug_assert(s.rfind("foo") == json::string_key::npos);
	debug_assert(s.rfind('a', 1) == 19);
}

TEST(find_first_of_returns_the_first_character_equal_to_any_of_the_given)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.find_first_of("ijk f") == 2);
	debug_assert(s.find_first_of("?)M") == 18);
	debug_assert(s.find_first_of("ijk f", 7) == 11);
	debug_assert(s.find_first_of("?") == json::string_key::npos);
}

TEST(find_last_of_returns_the_last_character_equal_to_any_of_the_given)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.find_last_of("ijk f") == 17);
	debug_assert(s.find_last_of("?)M") == 18);
	debug_assert(s.find_last_of("ijk f", 7) == 14);
	debug_assert(s.find_last_of("?") == json::string_key::npos);
}

TEST(find_first_not_of_returns_the_first_character_not_equal_to_any_of_the_given)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.find_first_not_of("Enu ") == 6);
	debug_assert(s.find_first_not_of("lugar", 6) == 11);
	debug_assert(s.find_first_not_of("En un lugar de la Mancha") == json::string_key::npos);
}

TEST(find_last_not_of_returns_the_last_character_not_equal_to_any_of_the_given)
{
	const json::string_key s = "En un lugar de la Mancha";

	debug_assert(s.find_last_not_of("Manch ") == 15);
	debug_assert(s.find_last_not_of("lugar", 13) == 5);
	debug_assert(s.find_last_not_of("En un lugar de la Mancha") == json::string_key::npos);
}

TEST(static_string_literals)
{
	using namespace json::string_key_literals;

	const auto ss = "Hello"_ss;
	static_assert(std::is_same<std::decay_t<decltype(ss)>, json::static_string>::value, "");
	debug_assert(ss.size() == 5);
	debug_assert(ss.c_str() == std::string{ "Hello" });

	const auto wss = L"Hello"_ss;
	static_assert(std::is_same<std::decay_t<decltype(wss)>, json::static_wstring>::value, "");
	debug_assert(wss.size() == 5);
	debug_assert(wss.c_str() == std::wstring{ L"Hello" });

	const auto u16ss = u"Hello"_ss;
	static_assert(std::is_same<std::decay_t<decltype(u16ss)>, json::static_u16string>::value, "");
	debug_assert(u16ss.size() == 5);
	debug_assert(u16ss.c_str() == std::u16string{ u"Hello" });

	const auto u32ss = U"Hello"_ss;
	static_assert(std::is_same<std::decay_t<decltype(u32ss)>, json::static_u32string>::value, "");
	debug_assert(u32ss.size() == 5);
	debug_assert(u32ss.c_str() == std::u32string{ U"Hello" });
}

TEST(string_key_literals)
{
	using namespace json::string_key_literals;

	const auto sk = "Hello"_sk;
	static_assert(std::is_same<std::decay_t<decltype(sk)>, json::string_key>::value, "");
	debug_assert(sk.size() == 5);
	debug_assert(sk == "Hello");
	debug_assert(sk.is_view());

	const auto wsk = L"Hello"_sk;
	static_assert(std::is_same<std::decay_t<decltype(wsk)>, json::wstring_key>::value, "");
	debug_assert(wsk.size() == 5);
	debug_assert(wsk == L"Hello");
	debug_assert(wsk.is_view());

	const auto u16sk = u"Hello"_sk;
	static_assert(std::is_same<std::decay_t<decltype(u16sk)>, json::u16string_key>::value, "");
	debug_assert(u16sk.size() == 5);
	debug_assert(u16sk == u"Hello");
	debug_assert(u16sk.is_view());

	const auto u32sk = U"Hello"_sk;
	static_assert(std::is_same<std::decay_t<decltype(u32sk)>, json::u32string_key>::value, "");
	debug_assert(u32sk.size() == 5);
	debug_assert(u32sk == U"Hello");
	debug_assert(u32sk.is_view());
}

TEST(tokenize_algorithm_can_take_a_sequence_of_chars_which_is_not_in_contiguous_memory)
{
	const std::string source = "[ 1, false, null, "
		"[ 1, 3, 4 ],"
		"\"Hello!\", "
		"{ \"foo\" : null, \"bar\" : true }"
		"]";

	const std::forward_list<char> source2{ source.begin(), source.end() };

	const auto tokens1 = json::parser::tokenize(source);
	const auto tokens2 = json::parser::tokenize(source2.begin(), source2.end());

	debug_assert(tokens1.size() == tokens2.size());
	debug_assert(std::equal(tokens1.begin(), tokens1.end(), tokens2.begin(), tokens2.end(), [](const auto & a, const auto & b)
	{
		return a.type == b.type;
	}));
}

TEST(parse_algorithm_can_take_a_sequence_of_chars_which_is_not_in_contiguous_memory)
{
	const std::string source = "[ 1, false, null, "
		"[ 1, 3, 4 ],"
		"\"Hello!\", "
		"{ \"foo\" : null, \"bar\" : true }"
		"]";

	const std::forward_list<char> source2{ source.begin(), source.end() };

	const auto val1 = json::parser::parse(source);
	const auto val2 = json::parser::parse(source2.begin(), source2.end());

	debug_assert(val1 == val2);
}

TEST(deleteme_visualizers)
{
	json::value v0;
	json::value v1 = 4;
	json::value v2 = 4.5;
	json::value v3 = true;
	json::value v4 = "Hello";
	json::value v5 = { 4, "Foobar", false };
	json::value v6 = { std::make_pair("hello", 4), {"foo", "bar"} };
}



