= Overload resolution

@ <b> {I use it better than the template, but I don't understand it better than the template! }

C ++ has a feature called overloading that allows you to implement functions of the same name with different types of arguments.
"Overload resolution" is the name of the process that selects which implementation to call when a function call expression is given.

// emlist [] [cpp] {
void func (int); // # 1
void func (double); // # 2

int main () {
    func (1); // calls # 1
    func (0.0); // calls # 2
}
//}

This appendix details overloading.

== Overload resolution ranking

Overload resolution priorities are ordered in the order Partial Order.
Partial Order is @ <code> {a <b} or @ <code> {a> b} or @ <code> {false}
There is a condition where @ <code> {a == b} is not @ <code> {true}, that is, incomparable.

When the overload candidates of a function are incomparable,
Overload resolution becomes ambiguous and fails to compile.

// emlist [] [cpp] {
void func (int, double); // # 1
void func (long, int); // # 2

int main () {
    func (1, 1); // ambiguous
}
//}

In this example, @ <b> {# 1} is the first argument, and @ <b> {# 2} is the second argument, which is an int.
However, @ <b> {# 1} is the second argument call to int double,
@ <b> {# 2} has a long 1st argument call to int.
As a result, the compiler can't figure out which to call, and the overload resolution is ambiguous.

Some readers might think that long is closer to int than double, but
C ++ doesn't take things like overload distance into account, so it's all ambiguous when it's not completely prioritized.

By the way, I used it without defining the word priority.
How are the overload priorities set?
Here are some simplified priorities.
No templates are included in this ranking (discussed later).

 * 1. Exact match: When all argument types are exact matches, including modifiers.
 * 2. Match by fine adjustment: In addition to 1. Allowing conversion from non-const to const, conversion from array to pointer to the first element of the array, etc.
 * 3. Match by promotion: In addition to 2., match from int to long, float to double, etc. by allowing safe and implicit conversion.
 * 4. Match by standard conversion only: In addition to 3. Matching by allowing conversion from int to float or public base class. Does not include conversions by implicit conversion operators or conversions by callable constructors.
 * 5. Match by user-defined conversion: In addition to 4., all conversions such as conversion by implicit conversion operator and conversion by constructor are allowed and matched.
 * 6. ellipsis (...): Almost all types match ellipsis (but not classes with non-trivial copy constructors).

It's pretty simplistic, but you're unlikely to encounter it if you can't think of it in this order (except for initializer_list, which I'll omit here).

Here are some small examples:

// emlist [] [cpp] {
int f1 (int); // # 1
int f1 (double); // # 2

f1 (4); // calls # 1
// # 1: perfect match
// # 2: needs a standard conversion
//}

This is a simple example that requires no explanation.
Perfect match has priority.

// emlist [] [cpp] {
int f2 (int); // # 3
int f2 (char); // # 4

f2 (true); // calls # 3
// # 3: matches with promotion
// # 4: requires stronger promotion
//}

Integer promotions have a priority among them.

// emlist [] [cpp] {
struct X {
    X (int);
};

int f3 (X); // # 5
int f3 (...); // # 6

f3 (1); // # calls # 5
// # 5: matches with user-defined conversion
// # 6: requires a match with ellipsis
//}

== Implicit arguments of member functions

A member function has an implicit argument @ <code> {* this} as its first argument ("implicit argument" is a coined word that can only be found here).
The type of the implicit first argument that the member function of @ <code> {MyClass} has is @ <code> {MyClass &} for non-const member function,
@ <code> {MyClass const &} for const member functions.
From C ++ 11, an rvalue version has been added.
The type of the implicit first argument that the member function of rvalue @ <code> {MyClass} has is @ <code> {MyClass &&} if it is a non-const member function,
@ <code> {MyClass const &&} for const member functions (though you probably won't use the const version).

If you take an rvalue reference as an argument, it will not match the actual lvalue, but only the rvalue.

// emlist [] [cpp] {
struct S {};

void func (S &); // # 1
void func (S &&); // # 2

int main () {
    S s {};
    void func (s); // calls # 1
    void func (S {}); // calls # 2
}
//}

You can do the same with member functions.
In the member function, write the modifier after @ <code> {()} as follows.
This modifier is the overloading syntax for implicit arguments of member functions.

// emlist [] [cpp] {
struct watch_t {
    void tick () &; // # 1
    void tick () const &; // # 2
    void tick () &&; // # 3
    void tack () const &; // # 4
};
//}

Calling this will select the overload as follows:

// emlist [] [cpp] {
struct watch_t {
    void tick () &; // # 1
    void tick () const &; // # 2
    void tick () &&; // # 3
    void tack () const &; // # 4
};

int main () {
    watch_t watch {};
    const watch_t const_watch {};
    watch.tick (); // calls # 1
    const_watch.tick (); // calls # 2
    watch_t {}. tick (); // calls # 3
    // calls # 4, because const lvalue ref accepts rvalue
    watch_t {}. tack (); 
}
//}

== Template VS Specialization VS Non-template

// emlist [] [cpp] {
template <class T> void f (T); // # 1
void f (int); // # 2
template <> void f <double> (double); // # 3

int main () {
    f (1); // calls # 2
    f (1.0); // calls # 3
    f ("hoge"); // calls # 1
}
//}

It's often said that non-templates take precedence when overloaded candidates are basically templates and non-templates.
Also, any template that is explicitly specialized has higher priority than templates that are not.
As we've seen in Chapter 4, this has some pitfalls.
Non-templates take precedence over templates only if the non-templates are a perfect match.
To get rid of the generic function template, and to match a more specialized template, you need to include the modifiers as well.

// emlist [] [cpp] {
template <typename T> void f (T &&);

// Vectors are overloads written with a firm intention to call this
template <typename T> void f (std :: vector <T> &);
template <typename T> void f (const std :: vector <T> &);
template <typename T> void f (std :: vector <T> &&);
template <typename T> void f (const std :: vector <T> &&);
//}

== initializer_list

From C ++ 11, @ <code> {std :: initializer_list} has been added.

// emlist [] [cpp] {
#include <initializer_list>

void func (std :: initializer_list <int>);

int main () {
    func ({1, 2, 3});
}
//}

@ <code> {std :: initializer_list} does overload resolution to minimize type conversions in braced-init-list.

// emlist [] [cpp] {
#include <initializer_list>

void func (std :: initializer_list <int>); // # 1
void func (std :: initializer_list <char>); // # 2

int main () {
    func ({'a', 'a', 'a'}); // calls # 2
    func ({'a', 'a', 'a', 1}); // calls # 1
}
//}

@ <code> {func ({'a', 'a', 'a'})} is an exact match for @ <code> {std :: initializer_list <char>}.
@ <code> {func ({'a', 'a', 'a', 1}))} requires type conversion.
@ <code> {int-> char} requires standard conversion, but @ <code> {char-> int} can be promoted, so this is the minimum type conversion.
So the overload resolves to @ <code> {std :: initializer_list <int>}.

The following overloads
@ <code> {int-> double},
Both @ <code> {double-> int} are standard conversions, so overloading is ambiguous and cannot be solved.


// emlist [] [cpp] {
#include <initializer_list>

void func (std :: initializer_list <int>); // # 1
void func (std :: initializer_list <double>); // # 2

int main () {
    func ({1,2,1.0}); // ambiguous
}
//}


What do you get when you run the following code?

// emlist [] [cpp] {
#include <string>
#include <iostream>

int main () {
    std :: cout << std :: string (33, 'a') << std :: endl;
    std :: cout << std :: string {33, 'a'} << std :: endl; 
}
//}


The first outputs @ <b> {aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa}.
The second is not, @ <b> {33} is interpreted as a character (assuming ASCII, the output will be @ <b> {! A}).
This is due to the specification that the constructor with @ <code> {std :: initializer_list} as an argument is called preferentially when initialized with braces.
@ <code> {std :: string} has a constructor that takes a list of characters at @ <code> {std :: initializer_list <char>}, which is called first.

== Overload ranking with template

 * 1. Perfect match
 * 2. An exact match for an explicitly specialized template
 * 3. Perfect match for more specialized templates
 * 4. Perfect match by instantiating template
 * 5. Match by fine adjustment
 * 6. Promotional match
 * 7. Match by standard conversion only
 * 8. Match by user-defined conversion
 * 9. ellipsis (...)

As you can see,
Templates can inline exact matching functions, so
Do not write function templates that are too generic.

Never think of coexisting generic function templates and non-template overloads, it's just hell.
