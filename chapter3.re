= Clever inheritance usage

Inheritance is convenient, but if used incorrectly it can lead to terrible code.

// image [dot_schema_all_modules] [Complex inheritance example] [scale = 0.5] { 
//}

== Can't master inheritance

Even professionals who write C ++ at work can't master inheritance.

Specifically, you would write code like this:

 * Class without final designation does not have virtual destructor
 * Write a virtual function that does not specify override
 * Inheritance relationship is too complicated

final and override are context-sensitive keywords imported from C ++ 11.
Basically, all virtual functions should be specified as overrides, nothing is skipped.
Also, if a virtual class that is not finalized has no virtual destructor,
The slicing problem leaks resources, creating bugs that are difficult to debug.
The only thing I can't do around here is lack of study.

// emlist [] [cpp] {
namespace my_lib {
  struct base {
    virtual ~ base () = default;
    virtual void method1 () = 0;
    virtual void method2 () = 0;
  };

  struct derived1: base {
    virtual ~ derived1 () = default;
    // do not specify final because it is still overriding
    virtual void method1 () override {/*...*/}
    // Prohibit override with final because it will not be overridden anymore
    virtual void method2 () override final {/*...*/}
  };

  struct derived2 final: derived1 {
    // need not be virtual as it is final
    ~ derived2 () = default;

    virtual void method1 () override {/*...*/}

    // cannot be overridden because final1 is specified in derived1
    // virtual void method2 () override final {/*...*/}
  };
}
//}

== God class advent, succession dance

I've been working and have seen code with dozens of derived classes that inherit from the base class.

This is the so-called "God class" anti-pattern.
It is a way to design commonly used functions as pure virtual functions in the base class.

As you can see from a moment, this method makes the base and derived classes tightly coupled and very hard to change.
In most cases something is wrong.

Before we solve the "God class" problem, let's talk about composition.

== Composition

To briefly explain the composition, it is designed to embed a function with a class from the outside.

It is often said that having something as a member is like a composition.
In C ++, the method called the Policy class, which specifies a class that has a function as a template argument, is generally used.
If the policy class needs a state as a member variable,
If you don't need state, it's normal to embed it in a template as a class with static functions.

This method is also used in std :: map introduced in Chapter 2.

// emlist [] [cpp] {
namespace std {
    template <class Key, class T,
        class Compare = less <Key>,
        class Allocator = allocator <pair <const Key, T>>
    > class map;
}
//}

In Compare and Allocator, the member map has an object as a variable (may be inherited due to EBO).
Specify the customized comparison class or allocator class in the template and pass the instance to the constructor.
This allowed us to control the behavior of different classes without using inheritance!

@ <b> {Because the types are different, they can be stored in the same container! }

Yes, the classes customized in the template composition are different types.
Even if the allocator is a little different.
Isn't it difficult to use because it can't be assigned to another container because the allocator is different?

== Local dynamic polymorphism

If the types are different, use a dynamic polymorphic allocator!

@ <b> {Use a dynamic polymorphic Policy class without making the class itself a derived class. }

In the case of a container, dynamic polymorphism of @ <code> {Allocator} of @ <code> {std :: vector <T, Allocator>} will solve the problem.

The dynamic polymorphic allocator class @ <code> {polymorphic_allocator} has been added to the C ++ 17 header @ <code> {<polymorphic_resource>}.
A container aliased to the namespace @ <code> {pmr} specifies @ <code> {polymorphic_allocator} as the allocator class.

// emlist [] [cpp] {
namespace std {
  // From C ++ 17
  namespace pmr {
    template <class T>
      using vector = std :: vector <T, polymorphic_allocator <T >>;
  }
}
//}

It does not inherit the God class.
We should break down the function and make each one dynamic polymorphism.

It should be divided into policy classes in units that can be independent.
What you can do is to have a detailed policy for the human class such as foot class and arm class.
The function will be different between the artificial arm and the biological arm.

== Static injection

I will introduce it.
Static injection is a technique where the implementation is automatically injected or not.
Consider the case where you want to create a class called @ <code> {Option} that develops a convenient function for @ <code> {std :: optional}.

I want to create a monadic function that flattens @ <code> {Option <Option <T >>} into @ <code> {Option <T>}.
To implement this function only in @ <code> {Option} which has @ <code> {Option <T>} in @ <code> {T}, use the C ++ 20 requires expression ...
I want to use it!

Up to C ++ 17, the implementation is injected by inheritance as follows.

// emlist [] [cpp] {
namespace option {// forward declaration
template <class T> class Option;
} //! namespace option

namespace option :: detail {
template <class> struct flatten_injector {void flatten () = delete;};

template <class T>
struct flatten_injector <Option <Option <T >>> {
    auto flatten () const & {
        return static_cast <const Option <Option <T >> *> (this)-> storage.has_value ()
            ? static_cast <const Option <Option <T >> *> (this)-> storage.value ()
            : Option <T> {};
    }
};
} //! namespace option :: detail

#include <optional>
namespace option {
template <class T>
class Option: public detail :: flatten_injector <Option <T >> {
    template <class> friend class detail :: flatten_injector;
    std :: optional <T> storage;
  public:
    Option () = default;
    Option (T x): storage (x) {}
};
} //! namespace option

int main () {
    option :: Option <option :: Option <int >> opt = option :: Option (1);
    opt.flatten ();
}
//}

== Summary of this chapter

 * Break down functionality into smaller pieces
 * Localize inheritance into policy
 * Be aware of the smallest type constraints
