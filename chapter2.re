= Reduce wasted code

The more code you write, the more bugs you introduce.
Avoiding unnecessary code duplication and complexity, and keeping your code simple and beautiful is the key to successful design.

== Copy and Dance

The first strategy to reduce wasted code is to standardize common operations as functions.
Alas, there is a lot of code that was copied and copied by Copy and Paste.
If you change a part of such code, other changes are leaked out, and it is easy to cause a bug.

In the first place, it's best to use a trusted library without writing your own code.
This chapter introduces how to use the customization points of the C ++ standard library.

== Customization points

Many languages ​​have established methods for customizing common operations such as stringification and object comparison.
Remembering the customization allows you to write code with common resources.

=== Stringifying

Let's wait for the string format entered from C ++ 20 (the real intention).

Actually, until C ++ 17, @ <code> {operator <<} which outputs to << code> {std :: ostream} as below is often used as a customization point.
Output to @ <code> {std :: stringstream} and extract the string with @ <code> {str ()}.

// emlist [] [cpp] {
class Widget {
    int a, b;
public:
    friend std :: ostream &
    operator << (std :: ostream & os, const Widget & w) const {
        return os << a << "" << b;
    }
};
//}

=== Comparison

Wait for the three-way comparison operator @ <code> {operator <=>} that comes in from C ++ 20 (the real intention). In C ++ 17, there are several types of comparison operation CPO.

@ <b> {Method 1: Operator overloading}

Properly overload the six operators @ <code> {==,! =, <,>, <=,> =}.

@ <b> {Method 2: CPO (Customization Point Object)}

In @ <code> {std :: map} and @ <code> {std :: set}, the third template argument is the customization point object.
You can customize the key comparison of @ <code> {std :: map} by specializing std :: less or passing your own class.

// customize emlist [std :: map in descending order] [cpp] {
#include <map>
#include <functional>

int main () {
    std :: map <std :: string, int, std :: greater <>> dict {};
}
//}

This way we separate the customization points from the classes.
If nothing is written, the default implementation will be selected.
Users can statically inject implementation details by explicitly specifying the class.
The design of this customization point is called a policy (discussed in the next chapter).

Functions with comparisons are supposed to receive a function object for comparison.
That is the third argument of std :: sort, std :: min, std :: max. If nothing is specified, the operator will be used.
This is a way to keep the customization points that are often used in functions.

// emlist [std :: sort customization] [cpp] {
#include <algorithm>
#include <vector>

int main () {
    std :: vector <std :: pair <int, int >> vec {{1, 2}, {2, 3}, {3, 4}};
    std :: sort (vec.begin (), vec.end (), [] (auto && p1, auto && p2) {
        return p1.second <p2.second;
    });
}
//}

== Summary of this chapter

 * Make the code common
 * Search for a library without writing the code yourself
 * Take advantage of standard library customization points
