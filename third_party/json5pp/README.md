# json5pp
JSON (ECMA-404 and JSON5) parser &amp; stringifier written in **C++11**.

# Features

* Easy to embed to your project. Only single file `json5pp.hpp` required.
* Parse [standard JSON (ECMA-404)](https://www.json.org/) from `std::istream` or `std::string`.
* Parse [JSON5](https://json5.org/) from `std::istream` or `std::string`.
* Stringify to `std::ostream` or `std::string` as standard JSON.
* Stringify to `std::ostream` or `std::string` as JSON5.

# Requirements

* Compilers with C++11 support

# License

* MIT license

# API

## JSON value type

```cpp
namespace json5pp {
  class value {
  };
}
```
* The class holds a value which is represented in JSON.
* This class can hold all types in JSON:
  * `null`
  * boolean (`true` / `false`)
  * number
  * string
  * array (stored as `std::vector<json5pp::value>`)
  * object (stored as `std::map<std::string, json5pp::value>`)
* Provides type check with `is_xxx()` method. (`xxx` is one of `null`, `boolean`, `number`, `string`, `array` and `object`)
* Provides explicit cast to C++ type with `as_xxx()` method. (`xxx` is one of `null`, `boolean`, `number`, `integer`, `string`, `array` and `object`)
  * If cast failed, throws `std::bad_cast`
* Accepts implicit cast (by overload of `operator=`) from C++ type (`nullptr_t`, `bool`, `double` | `int`, `std::string` | `const char*`)
* See [examples](#Examples) for details

## Parse functions

```cpp
namespace json5pp {
  value parse(const std::string& str);
}
```
* Parse given string.
* String must be a valid JSON (ECMA-404 standard)
  * If not valid, throws `json5pp::syntax_error`.
* String must be a finished (closed) JSON.
  * i.e. If there are any junk data (except for spaces) after JSON, throws `json5pp::syntax_error`.

```cpp
namespace json5pp {
  value parse(std::istream& istream, bool finish = true)
}
```
* Parse JSON from given input stream.
* Stream must have a valid JSON (ECMA-404 standard).
  * If not valid, throws `json5pp::syntax_error`.
* If `finish` is true, stream must be closed by eof after JSON.

```cpp
namespace json5pp {
  value parse5(const std::string& str);
}
```
* JSON5 version of `parse(const std::string&)`

```cpp
namespace json5pp {
  value parse5(std::istream& istream, bool finish = true);
}
```
* JSON5 version of `parse(std::istream&, bool)`

## Stringify functions

```cpp
namespace json5pp {
  class value {
    template <class... T>
    std::string stringify(T... manip);
  };
}
```
* Stringify value to ECMA-404 standard JSON.
* About `T... manip`, see [iostream overloads and manipulators](#iostream-API-and-manipulators)

```cpp
namespace json5pp {
  class value {
    template <class... T>
    std::string stringify5(T... manip);
  };
}
```
* Stringify value to JSON5.
* About `T... manip`, see [iostream overloads and manipulators](#iostream-API-and-manipulators)

```cpp
namespace json5pp {
  template <class... T>
  std::string stringify(const value& v, T... manip);
}
```
* Global method version of `json5pp::value::stringify()`
* Same as `v.stringify(manip...)`

```cpp
namespace json5pp {
  template <class... T>
  std::string stringify5(const value& v, T... manip);
}
```
* Global method version of `json5pp::value::stringify5()`
* Same as `v.stringify5(manip...)`

## iostream API

### Parse by `operator>>`
```cpp
std::istream& istream = ...;
json5pp::value v;
istream >> v;
```

### Parse by `operator>>` with manipulators
```cpp
std::istream& istream = ...;
json5pp::value v;
istream >> json5pp::rule::json5() >> v; // Parse as JSON5
```

### Stringify by `operator<<`
```cpp
std::ostream& ostream = ...;
const json5pp::value& v = ...;
ostream << v;
```

### Stringify by `operator<<` with manipulators
```cpp
std::ostream& ostream = ...;
const json5pp::value& v = ...;
ostream << json5pp::rule::tab_indent<1>() << v;   // Stringify with tab indent
ostream << json5pp::rule::space_indent<2>() << v; // Stringify with 2-space indent
```

## Manipulators

### Comments
* `json5pp::rule::single_line_comment()`
* `json5pp::rule::no_single_line_comment()`
  * Allow/disallow single line comment starts with `//`
* `json5pp::rule::multi_line_comment()`
* `json5pp::rule::no_multi_line_comment()`
  * Allow/disallow multiple line comment starts with `/*` and ends with `*/`
* `json5pp::rule::comments()`
* `json5pp::rule::no_comments()`
  * Combination of `single_line_comment` and `multi_line_comment`

### Numbers
* `json5pp::rule::explicit_plus_sign()`
* `json5pp::rule::no_explicit_plus_sign()`
  * Allow/disallow explicit plus sign (`+`) before non-negative number (ex: `+123`)
* `json5pp::rule::leading_decimal_point()`
* `json5pp::rule::no_leading_decimal_point()`
  * Allow/disallow leading decimal point before number (ex: `.123`)
* `json5pp::rule::trailing_decimal_point()`
* `json5pp::rule::no_trailing_decimal_point()`
  * Allow/disallow trailing decimal point after number (ex: `123.`)
* `json5pp::rule::decimal_points()`
* `json5pp::rule::no_decimal_points()`
  * Combination of `leading_decimal_point` and `trailing_decimal_point`
* `json5pp::rule::infinity_number()`
* `json5pp::rule::no_infinity_number()`
  * Allow/disallow infinity number
* `json5pp::rule::not_a_number()`
* `json5pp::rule::no_not_a_number()`
  * Allow/disallow NaN
* `json5pp::rule::hexadecimal()`
* `json5pp::rule::no_hexadecimal()`
  * Allow/disallow hexadecimal number (ex: `0x123`)

### Strings
* `json5pp::rule::single_quote()`
* `json5pp::rule::no_single_quote()`
  * Allow/disallow single quoted string (ex: `'foobar'`)
* `json5pp::rule::multi_line_string()`
* `json5pp::rule::no_multi_line_string()`
  * Allow/disallow multiple line string escaped by `\`
  * Example:
    ```json
    "test\
    2nd line"
    ```

### Arrays and objects
* `json5pp::rule::trailing_comma()`
* `json5pp::rule::no_trailing_comma()`
  * Allow/disallow trailing comma at the end of arrays or objects.
  * Example for arrays: `[1,2,3,]`
  * Example for objects: `{"a":123,}`

### Objects
* `json5pp::rule::unquoted_key()`
* `json5pp::rule::no_unquoted_key()`
  * Allow/disallow unquoted keys in objects. (ex: `{a:123}`)

### Rule sets
* `json5pp::rule::ecma404()`
  * ECMA-404 standard rule set.
* `json5pp::rule::json5()`
  * JSON5 rule set.

### Parse options
* `json5pp::rule::finished()`
  * Parse as finished (closed) JSON. If any junk data follows after JSON, parse fails.
  * Opposite to `json5pp::rule::streaming()`
* `json5pp::rule::streaming()`
  * Parse as non-finished (non-closed) JSON. Parse will succeed at the end of JSON.
  * Opposite to `json5pp::rule::finished()`

### Stringify options
* `json5pp::rule::lf_newline()`
  * When indent is enabled, use LF(`\n`) as new-line code.
  * Opposite to `json5pp::rule::crlf_newline`
* `json5pp::rule::crlf_newline()`
  * When indent is enabled, use CR+LF(`\r\n`) as new-line code.
  * Opposite to `json5pp::rule::lf_newline`
* `json5pp::rule::no_indent()`
  * Disable indent. All arrays and objects will be stringified as one-line.
* `json5pp::rule::tab_indent<L>()`
  * Enable indent with tab character(s).
  * `L` means a number of tab (`\t`) characters for one-level indent.
  * If `L` is omitted, treat as `L=1`.
* `json5pp::rule::space_indent<L>()`
  * Enable indent with space character(s).
  * `L` means a number of space (` `) characters for one-level indent.
  * If `L` is omitted, treat as `L=2`.

## Examples

### Constructing `value` object

```cpp
using namespace std;

// Construct "null" value
json5pp::value a;               // Default constructor
cout << a.is_null() << endl;    // => 1
cout << a << endl;              // => null

json5pp::value b(nullptr);      // Constructor with std::nullptr_t argument
cout << b.is_null() << endl;    // => 1
cout << b << endl;              // => null

// json5pp::value c(NULL);      // Compile error
                                // NULL cannot be used instead of nullptr

// Construct boolean value
json5pp::value d(true);         // Constructor with bool argument
cout << d.is_boolean() << endl; // => 1
cout << d << endl;              // => true

// Construct number value
json5pp::value e(123.45);       // Constructor with double argument
cout << e.is_number() << endl;  // => 1
cout << e << endl;              // => 123.45

json5pp::value f(789);          // Constructor with int argument
cout << f.is_number() << endl;  // => 1
cout << f << endl;              // => 789

// Construct string value
std::string str("foo");
json5pp::value g(str);          // Constructor with const std::string& argument
cout << g.is_string() << endl;  // => 1
cout << g << endl;              // => "foo"

json5pp::value h("bar");        // Constructor with const char* argument
cout << h.is_string() << endl;  // => 1
cout << h << endl;              // => "bar"

// Construct array value
json5pp::value i {1, false, "baz", nullptr};
                                // Construct with std::initializer_list
cout << i.is_array() << endl;   // => 1
cout << i << endl;              // => [1,false,"baz",null]

auto j = json5pp::array({1, false, "baz", nullptr});
                                // Construct with utility function: json5pp::array()
cout << j.is_array() << endl;   // => 1
cout << j << endl;              // => [1,false,"baz",null]

// json5pp::value k {{"foo", 123}};
                                // Compile error (*1)

// Construct object value
auto m = json5pp::object({{"bar", 123}, {"foo", "baz"}});
                                // Construct with utility function: json5pp::object()
cout << m.is_object() << endl;  // => 1
cout << m << endl;              // => {"bar":123,"foo":"baz"}

// json5pp::value n{{"foo", 123}};
                                // Compile error (*1)

// (*1)
// These forms are rejected because an implicit conversion
// for arrays and objects makes ambiguousness, for example:
// json5pp::value x{{"foo", 123}};  // Ambiguous!
//                                  // candidate: [["foo",123]]
//                                  // candidate: {"foo":123}
//
// Use utility functions (json5pp::array(),json5pp::object()) to remove
// ambiguousness.
```

### Changing `value` object
```cpp
using namespace std;

json5pp::value x;       // Default constructor makes null value
cout << x << endl;      // => null
x = false;              // Assign boolean with bool value
cout << x << endl;      // => false
x = 123.45;             // Assign number with double value
cout << x << endl;      // => 123.45
x = 789;                // Assign number with int value
cout << x << endl;      // => 789
std::string str("bar");
x = str;                // Assign string with const std::string& value
cout << x << endl;      // => "bar"
                        // Because assignment copies the content of string,
                        // changing a source string does not affect `x`:
str += "123";
cout << x << endl;      // => "bar" (Contents does NOT change)
x = "foo";              // Assign string with const char* value
cout << x << endl;      // => "foo"
x = nullptr;            // Assign null with nullptr_t value
cout << x << endl;      // => null

x = json5pp::array({1});// Assign array with utility function: json5pp::array()
cout << x << endl;      // => [1]

auto& a = x.as_array(); // You can get container object by as_array() method
                        // decltype(a) => std::vector<json5pp::value>&
a.push_back("foo");     // Add value at the end of array
cout << x << endl;      // => [1,"foo"]

x = json5pp::object({{"foo","bar"}});
                        // Assign object with utility function: json5pp::object()
cout << x << endl;      // => {"foo":"bar"}

// Note: Do not access `a` (array container) after replacing the content of `x`
//       with a new object.

auto& o = x.as_object();// You can get container object by as_object() method
                        // decltype(o) => std::map<std::string, json5pp::value>&
                        // Note: Do not forget `&` when you use `auto` keyword!
o.emplace("baz", 123);  // Add value with key "baz"
cout << x << endl;      // => {"baz":123,"foo":"bar"}

json5pp::value y;
y = x;                  // You can copy the value by "=" operator
cout << y << endl;      // => {"baz":123,"foo":"bar"}

                        // Because assignment arrays/objects is a deep copy,
                        // changing a source value `x` does not affect `y`:
o.erase("foo");         // Remove key "foo" from the content of `x` object
cout << x << endl;      // => {"baz":123}
cout << y << endl;      // => {"baz":123,"foo":"bar"} (Contents does NOT change)
```

### Accessing `value` object
```cpp
using namespace std;

// Access boolean (See also: Truthy/falsy tests)
json5pp::value a(true);
auto a_value = a.as_boolean();    // decltype(a_value) => bool
cout << a_value << endl;          // => 1

// Access number
json5pp::value b(123.45);
auto b_value1 = b.as_number();    // decltype(b_value1) => double
cout << b_value1 << endl;         // => 123.45
auto b_value2 = b.as_integer();   // decltype(b_value2) => int
cout << b_value2 << endl;         // => 123

// Access string
json5pp::value c("foo");
auto c_value = c.as_string();     // decltype(c_value) => std::string
cout << c_value << endl;          // => foo

// Access array
json5pp::value d{1, "foo", false};
auto& d_value = d.as_array();     // decltype(d_value) => std::vector<json5pp::value>&
cout << d_value.size() << endl;           // => 3
cout << d_value[0].as_number() << endl;   // => 1
cout << d_value[1].as_string() << endl;   // => foo
cout << d_value[2].as_boolean() << endl;  // => 0

// Access array with indexer
cout << d[0].as_number() << endl;         // => 1
cout << d[1].as_string() << endl;         // => foo
cout << d[2].as_boolean() << endl;        // => 0
// d[1] = 123;                            // Compile error (indexer is read-only)

// Access object
auto e = json5pp::object({{"bar", 123}, {"foo", true}});
auto& e_value = e.as_object();    // decltype(e_value) => std::map<std::string, json5pp::value>&
cout << e_value.size() << endl;                 // => 2
cout << e_value.at("bar").as_number() << endl;  // => 123
cout << e_value.at("foo").as_boolean() << endl; // => 1

// Access object with indexer
cout << e["bar"].as_number() << endl;     // => 123
cout << e["foo"].as_boolean() << endl;    // => 1
// e["baz"] = 123;                        // Compile error (indexer is read-only)

// Invalid cast
// (If type does not match, as_xxx() method throws std::bad_cast())
json5pp::value f(123);    // type is number
// f.as_null();           // throws std::bad_cast()
json5pp::value g();       // type is null
// f.as_boolean();        // throws std::bad_cast()

// Truthy/falsy test
// falsy: null, false, 0, -0, NaN, ""
// truthy: other values
json5pp::value truthy1(true);
json5pp::value truthy2(1);
json5pp::value truthy3("foo");
json5pp::value truthy4{};
auto truthy5 = json5pp::object({});
cout << (bool)truthy1 << endl;  // => 1
cout << (bool)truthy2 << endl;  // => 1
cout << (bool)truthy3 << endl;  // => 1
cout << (bool)truthy4 << endl;  // => 1
cout << (bool)truthy5 << endl;  // => 1

json5pp::value falsy1();        // null
json5pp::value falsy2(false);
json5pp::value falsy3(0);
json5pp::value falsy4(numeric_limits<double>::quiet_NaN());     // NaN
json5pp::value falsy5(numeric_limits<double>::signaling_NaN()); // NaN
json5pp::value falsy6("");
cout << (bool)falsy1 << endl;  // => 0
cout << (bool)falsy2 << endl;  // => 0
cout << (bool)falsy3 << endl;  // => 0
cout << (bool)falsy4 << endl;  // => 0
cout << (bool)falsy5 << endl;  // => 0
cout << (bool)falsy6 << endl;  // => 0
```

### Parse
```cpp
using namespace std;

auto x = json5pp::parse("{\"foo\":[123,\"baz\"]}");
cout << x.is_object() << endl;            // => 1
cout << x["foo"].is_array() << endl;      // => 1
cout << x["foo"][0].as_number() << endl;  // => 123
cout << x["foo"][1].as_string() << endl;  // => baz

auto y = json5pp::parse5("{\"foo\"://this is comment\n[123,\"baz\"/*trailing comma-->*/,],}");
cout << y.is_object() << endl;            // => 1
cout << y["foo"].is_array() << endl;      // => 1
cout << y["foo"][0].as_number() << endl;  // => 123
cout << y["foo"][1].as_string() << endl;  // => baz
```

### Stringify
```cpp
using namespace std;

// Make some example object...
json5pp::value x = json5pp::object({
  {"foo", 123},
  {"bar",
    json5pp::array({
      1, "baz", true,
    })
  },
});

// Stringify to output stream by "<<" operator
cout << x << endl;

// Stringify to std::string by stringify() method
auto s = x.stringify(); // decltype(s) => std::string
cout << s << endl;      // => {"bar":[1,"baz",true],"foo":123}

// Stringify to output stream with indent specification
cout << json5pp::rule::space_indent<>() << x << endl; /* =>
{
  "bar": [
    1,
    "baz",
    true
  ],
  "foo": 123
}
*/

// Stringify to std::string with indent specification
auto s2 = x.stringify(json5pp::rule::tab_indent<>());
cout << s2 << endl; /* =>
{
>       "bar": [
>       >       1,
>       >       "baz",
>       >       true,
>       ],
>       "foo": 123
}
(`>` means tab) */
```

## Limitation

* Not fully compatible with unquoted keys in JSON5 (Some unicode will be rejected as keys)
* All strings are assumed to be stored in UTF-8 encoding.

## ToDo

* More tests
