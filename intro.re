= Introduction

This book is not intended for people who are just beginning programming.
It is a compilation of messages for programmers who leave beginners and move on.

This book assumes C ++ 17 unless otherwise noted.

To give advice that "more like this" while working,
There are many opportunities to explain, "This is how this function works."

As I explained it many times, I thought I should write it in a book, so I took the brush.

In the latter half, most of the comments in the sample code are in English.
I was careless.
Well, I believe most C ++ programmers can read English.
If you can't read it, please feel free from the code (sorry).

== Maeki

My name is Inamu Mitama.

My area of ​​expertise is library programming.
To create a library, you have to pile up a lot of C ++ features that you wouldn't use for the rest of your application.
So I'm a little more familiar with C ++ than a regular C ++ programmer.

As a guardian of C ++, I am bullying C ++ compilers every day.
It lives on Twitter under the name Inamu no Mitama no Kakami (@mitama_rs) and is working to save C ++ programmers who are lost on Twitter.

By the way, suddenly, this is my story.
It has been a year since I was hired by a venture company and wrote C ++ and Rust.
The C ++ programmer who had just changed jobs had some troubles.

 * Source code that uses malloc, not even smart pointers, even new
 * Completely backward compatible STL, oleore implementation
 * A towering library with over 30,000 headers
 * No documentation or meaningless past heritage
 * People who say "use templates as much as possible because they are difficult"
 * CMakeLists not automated at all

Most of the suffering is caused by a library called Open Cascade Technology.

Not just C ++, dirty code is very dirty.
Sometimes I want to say "Don't write C ++ anymore".

Programmers who write dirty code aren't in any way out.
On the contrary, it looks like I'm writing time code many times more than I do.
If you use the library, you can write the processing by hand,
I wrote a lot of similar, slightly different processing,
Writing meaningless conditional branches,
Write unused variables,
It's all about doing all the wrong techniques to complicate coding.

Worse, we don't have the time to clean up the CMakeLists or enrich our tests.
It's full of implementation.

Fortunately venture companies are very mobile, so I'm going to burn out the shit code.
It's unbearable if a newcomer looks at the existing code and imitates it.
I'm thinking that if you leave one line of shit code, it will be 100 lines of it.
This book is a collection of tears and sighs that I shed at that time.
Fortunately, the sequel to this book will not be written because the team has been stable enough to destroy the fucking code before it is born in education and review.

=== @ <b> {Is C ++ difficult? }

C ++ is difficult, but in most cases I find it necessary.

C ++ is extremely sophisticated.
It may not be suitable for easy coding of simple things.
But from the perspective of me writing the library, it's a heavenly language.
Whenever I wanted to write a library with great run-time performance and a flexible interface, C ++ was the best language.
I use Python for some reason, but there was a mysterious reassurance that I could rewrite it in C ++ if the runtime performance was not sufficient (although the Python library is excellent, of course).

If you don't need good run-time performance, have a dedicated interface, and don't have a legacy of C ++,
C ++ may seem wastefully complex and difficult.

Most easy-to-write languages ​​are made simple by truncating something.
In that sense, C ++ is difficult.
Various functions are open to users.
C ++ can help you when you really need them.
Conversely, choosing C ++ as your development language may be wrong if you don't really need them.
Those people should go to the next Python room.

@ <b> {C ++ standards can be difficult to understand, though! }