= C ++ trap that attacks a beginner

You finally escape the C ++ beginner and aim for better code.
It is the difficult specifications of C ++ that will strike you.

In this chapter, we will explain a number of traps that beginners who fall into the hands of advanced features will fall into.

== Forwarding Reference

@ <b> {Forwarding Reference} is a function that realizes a dream-like behavior that overloads of rvalue and lvalue can be handled at the same time.
Due to its characteristics, there is a fact that overloads using Forwarding Reference are suitable for overloads because they support all modifications.

Functions that take arguments in a Forwarding Reference should be type-constrained in SFINAE to avoid unintended calls.
But SFINAE is difficult.
Because it is difficult, it is not possible to write a type constraint in Forwarding Reference at the stage of post-beginner.

Then the following code is born.

// emlist [] [cpp] {
template <typename T>
void f (const std :: vector <T> &) {std :: cout << "vector <T>";}

template <typename T>
void f (T &&) {std :: cout << "T &&";}
//}

The above function @ <code> {f (const std :: vector <T> &)} is called only if the argument is an lvalue of @ <code> {const std :: vector <T>} .
In all other cases, the below function @ <code> {f (T &&)} is called.

When overloading, if you do not add type constraints, it will be selected by ADL and it will be an obstacle.
Don't write generic functions, you must put type constraints on them.
Up to C ++ 14, use the enable_if SFINAE technique as follows.

// emlist [] [cpp] {

template <class T>
struct is_vec: std :: false_type {};

template <class T>
struct is_vec <std :: vector <T >>: std :: true_type {};

// for vector
template <typename T>
f (const std :: vector <T> &) {std :: cout << "vector <T>";}

// default
template <typename T>
std :: enable_if_t <! std :: is_same_v <is_vecstd :: decay_t <T >> :: value>
f (T &&) {std :: cout << "T &&";}
//}


If you are using C ++ 17 and don't want to overload, you can use @ <code> {constexpr if} like this:

// emlist [] [cpp] {
template <typename T>
void f (T &&) {
  if constexpr (is_vec <std :: decay_t <T >> :: value) {
    std :: cout << "vector <T>";
  }
  else {
    std :: cout << "other";
  }
}
//}

== Structured binding

Structured binding is a feature added in C ++ 17.
It is very convenient because it can decompose pairs, tuples, arrays, and structures to retrieve each element.

What type does @ <code> {a, b} have when written as @ <code> {auto [a, b] = x}?
I can't count how many people explained this.

First, let me tell you something important.
Check out @ <code> {cppreference.com} once before using the new features.
Please do not use it in an atmosphere, please.

This function has a syntax equivalent to a declaration, you cannot assign to a variable that has already been declared.
Also, the type declaration can only write @ <code> {auto} with cvr modification.
Also, there is no convenient function that the value is ignored when you write something like @ <code> {_}. (Of course, @ <code> {std :: ignore} cannot be used because assignment is not possible. ).
Structured bindings cannot be nested.

Since it was written as @ <code> {auto const & [a, b] = x}, @ <code> {a, b} becomes const ...

Not necessarily @ <b> {! }

Think of the type declaration of structured binding @ <code> {auto const &} as a parameter declaration when receiving the target of decomposition @ <code> {x}.

When @ <code> {x} is an array, subscript access is only repeated in order.
If @ <code> {std :: tuple_size <T> :: value} is a class that is not a valid expression, it will be member access.

In these two cases, the type of @ <code> {a, b} is written as @ <code> {auto const & [a, b] = x}, so @ <code> {a, b} becomes const Is true.

In the case of tuple-like type, the story is different because you can have a reference type.
This is done by calling @ <code> {get <i> (x)} for classes where @ <code> {std :: tuple_size <T> :: value} is a valid expression.
Therefore, in case of tuple-like type, the variable type of i-th structured binding is @ <code> {std :: tuple_element <i, E> :: type}.

// emlist [] [cpp] {
float x {};
char y {};
int z {};
 
std :: tuple <float &, char &&, int> tpl (x, std :: move (y), z);

const auto & [a, b, c] = tpl;

// a that refers to x; decltype (a) is float &
// b that refers to y; decltype (b) is char &&
// c that refers to the 3rd element of tpl; decltype (c) is const int
//}

If the element is a reference type, the element type of tuple is available as is.

== Two Phase Lookup

C ++ has a feature called two-stage name search.
Intuitive name resolution is achieved through a two-step name lookup.

=== Two-stage name search and its necessity

The two-step name search is necessary in the following situations.

// emlist [] [cpp] {
#include <cstdio>

void func (void *) {std :: puts ("The call resolves to void *");}

template <typename T> void g (T x) {
    func (0);
}

void func (int) {std :: puts ("The call resolves to int");}

int main () {
    g (3.14);
}
//}

This @ <code> {g (3.14)}, at the time of template definition, the person who wrote the template would expect the already known @ <code> {func (void *)}.
Now, let's assume a new programmer added @ <code> {func (int)}.
If there is no two-step name search, all functions visible from the point of @ <code> {g (3.14)} will be searched and resolved to @ <code> {func (int)}.
The first search will search for the name visible from @ <code> {g (T)}, and the second search will materialize it.

This mechanism is called @ <b> {Two Phase Lookup}.

=== Exploitation of materialization delay

Consider an @ <code> {interval <T>} type that represents a closed interval.
Implements addition on this type.

// emlist [] [cpp] {
template <class T> struct interval {T low, up;};

template <class T>
interval <T> operator + (interval <T> lhs, interval <T> rhs)
    {return {lhs.low + rhs.low, lhs.up + rhs.up};}
//}

The number @ <m> {x} can be thought of as a one-point interval @ <m> {[x, x]}.
Therefore, you'll also want to create @ <code> {interval <T> + T} and @ <code> {T + interval <T>}.

// emlist [] [cpp] {
template <class T>
interval <T> operator + (T lhs, interval <T> rhs)
    {return {lhs + rhs, lhs.up + rhs.up};}
template <class T>
interval <T> operator + (interval <T> lhs, T rhs)
    {return {lhs.low + rhs, lhs.up + rhs};}
//}

Let's take a deep breath here.
@ <Code> {1 + 1.0} is allowed in C ++ as a built-in operation.

We extend this behavior a bit to include @ <code> {interval <T> + U} and @ <code> {U + interval <T>}
I want to do something like @ <code> {interval <T>} (the result type matches the interval type).

There is no need for complicated metaprogramming if you exploit the two-step name lookup.
However, some preparation is required.

First, add the dependent type name to @ <code> {interval} as follows:

// emlist [] [cpp] {
template <class T> struct interval {
    using value_type = T;
    T low, up;
};
//}

Something like @ <code> {interval <T> :: value_type} is called @ <b> {dependent name (dependent type name)}.
If @ <code> {T} is a template, @ <code> {value_type} is dependent on @ <code> {interval <T>} and derived when @ <code> {interval <T>} determines Will be done.
However, @ <code> {interval <double> :: value_type} is not a dependent name, @ <code> {interval <double>} immediately changes @ <code> {value_type} to @ <code> {double} To understand.

// emlist [] [cpp] {
template <class T> struct interval {
    using value_type = T;
    T low, up;
};
//}

Then correct the addition.

// emlist [] [cpp] {
template <class T>
interval <T> operator + (interval <T> lhs, interval <T> rhs)
    {return {lhs.low + rhs.low, lhs.up + rhs.up};}

template <class T>
interval <T> operator + (typename interval <T> :: value_type lhs, interval <T> rhs)
    {return {lhs + rhs, lhs.up + rhs.up};}
template <class T>
interval <T> operator + (interval <T> lhs, typename interval <T> :: value_type rhs)
    {return {lhs.low + rhs, lhs.up + rhs};}
//}

This is because @ <code> {typename interval <T> :: value_type} is a dependent name and is not the target of type inference from actual argument of function template.
Consider the following code:

// emlist [] [cpp] {
  interval <int> itv {1, 2};
  auto res = itv + 1.0;
//}

First, @ <code> {operator + (interval <T> lhs, typename interval <T> :: value_type rhs)} is searched.
From the left operand, @ <code> {interval <T>} is inferred as @ <code> {interval <int>}.
Then, the two-step name lookup resolves the dependent name and derives @ <code> {typename interval <T> :: value_type} as @ <code> {int}.

Finally, @ <code> {1.0 (double)} can be converted to @ <code> {int}, so overload resolution will succeed!

Abuse of language features is a lot of fun!

=== constexpr if

It's not a difficult function, but I decided to explain it because there are a lot of people who are addicted to the two-step name search (cpprefjp can be read though ...).

The code in question looks like this:

// emlist [] [cpp] {
template <class T>
void func () {
  if constexpr (! std :: integral_v <T>) {
    // Only evaluate if T is not an integer
    // actually always evaluated
    static_assert (false);
  }
}
//}

The constexpr if only prevents non-executable template materialization (does not perform dependency name validation), independent names are validated.
Since the conditional expression passed to static_assert in this example does not depend on the template parameter, it will be validated and an error will occur when the template is declared.

The solution is simple, and if the conditional expression passed to static_assert is a dependent name, it will not be validated when declaring the template, and evaluation can be delayed until the materialization of the template.

// emlist [] [cpp] {
template <class ...> inline constexpr bool always_false_v = false;

template <class T>
void func () {
  if constexpr (std :: integral_v <T>) {
    // ...
  }
  else {
    static_assert (always_false_v <T>);
  }
}
//}

@ <code> {always_false_v <T>} is a dependent name because it is an expression that depends on @ <code> {T}.

It is also possible to use an operation that delays evaluation by using a lambda expression as follows (this is possible from C ++ 17 in which compile-time evaluation of lambda expressions is allowed).

// emlist [] [cpp] {
template <typename T>
void f (T) {
  if constexpr (std :: is_same_v <T, int>) {
    static_assert ([] {return false;} ());
  }
}
//}

== Summary of this chapter

 * When using Forwarding Reference, set type constraint for overload
 * Check once with @ <code> {cppreference.com} before using new features
 * When using @ <code> {static_assert} in constexpr if, use dependency expression
