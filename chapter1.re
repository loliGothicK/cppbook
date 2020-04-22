= Ah! This branch is "deep"! !!

Perhaps you've seen code with unusually complex branching and deep nesting?
It is no exaggeration to say that complicated control flow code is a typical example of bad code.

If you have to do complicated things, your code will be complicated.
But I think it's a mistake to put too many cases into one function.

This chapter discusses how to code complex branches concisely and how to separate functions.

== Branch Ranbu

 * Negative number check for non-negative integers
 * Double check for pointers that have already been Null checked

For example, see a branch that never goes through.

// emlist [Negative check for non-negative integers] [cpp] {
// sizeof returns std :: size_t so it can never be true
if constexpr (sizeof (foo) <0) {
  // ...
}
//}

// emlist [Double check for pointers that have already been Null checked] [cpp] {
int * ptr;
assert (ptr); // first check
if (ptr) {/ Either assert or handle
  // ...
} else {
  // ...
}
//}

In case of non-negative integer negative check, it is easy to fix and just remove it.

I'm not sure if the pointer double check is right to assert or right to handle.
Therefore, there is no choice but to ask the person who wrote git blame and read it, or to think by reading the code yourself.

== Normal system and abnormal system

The fact that the normal system and the abnormal system are not written separately is one of the reasons why the code is difficult to understand.
It is a well-known method that the abnormal system is checked first, the abnormal value is returned, and then the normal system is processed.

In the first pull request that the newcomer first wrote, he showed me the code using switch and if.
Unfortunately, it's so complicated that it takes up space, so I have to give up posting pseudo code.

A better answer to this question seems to be the introduction of monads.
Below is the Rust code.
I will write a function that parses the command input and doubles it if it is a number.

// emlist [Command line parse written using the monad function of Result] [rust] {
fn twice_arg (mut argv: env :: Args)-> Result <i32, CliError> {
    argv.nth (1)
        .ok_or (CliError :: NotEnoughArgs)?
        .parse :: <i32> ()
        .map (| x | x * 2)
}
//}

This method chain does not write error handling!
I won't go into a detailed explanation of Rust, but if you use monadic functions, you don't have to bother writing error handling.
Synthesize the error!

Functions such as @ <code> {ok_or} and @ <code> {map} are called combinators.
@ <code> {nth} or @ <code> {parse} is returning @ <code> {Option} or @ <code> {Result},
The combinator is composing the error.

I have a disappointing news.
C ++ doesn't have such advanced features and libraries.
But if you don't, you can create a monad library.

Don't have time to write a template library?
However, combinators give us an important suggestion of separating functions in "units that can fail".
At the very least, I want to keep in mind that the function should be cut out in units where an error occurs.

== Summary of this chapter

No language does not have a monad.
Make a monadic function.

Do not write a function that causes many kinds of errors.
Divide the function by the minimum unit where an error occurs.
