# What is this?
This is just a collection of (usually single-purpos) header files that I use refularly in my code
# Common conventions
* As these are header-only libraries, there is a macro in form `HEADERNAME_IMPLEMENTATION` that has to be defined before including the header. A header can only be included once while this macro is defined, otherwise you get multiple definition errors.
* Names starting with underscores are kind of private-use for this libraries, but they may be declared not `static` because they are used in macros that expose API.
* `snake_case`
* Function-like macros are not all-caps.
* For now i only support GCC-based compilers. I will consider removing this dependency in future (e.g. getting rid of that [`__qsort_cmps` declaration](include/utility.h)).
# Descriptions for heders:
## [`dynarray.h`](include/dynarray.h)
* Summary: A flexible and easy-to-use dynamically-sized array implementation based on the idea of storing metadata about array to the left of actual data. this allows it to be used interchangabely as a regular C array and as dynamic array. 
* How to use: Define `DYNARRAY_IMPLEMENTATION` macro before including `dynarray.h`. If you are using functions to sort the array (`array_qsort_integral`), you also have to define `UTILITY_IMPLEMENTATION` (as it uses integer comparison functions from there)
* Examples: See [tests](tests/dynarray.c)
## [`utility.h`](include/utility.h)
* Summary: Some commonly-used macros and functions in this library, could as well be called `miscellaneous`. 
* How to use: Define `UTILITY_IMPLEMENTATION` macro before including `utility.h`. If you want it to expose helper functions/variables (which you may want, check the source), define macro `UTILITY_EXPOSE_HELPERS`.
* Examples: there are kind of no examples for this, i guess you could search for the usages in [`dynarray.h`](include/dynarray.h)
## [`embed.h`](include/embed.h)
* Summary: Code generator for embedding resources directly into an executable. 
* How to use: It exposes C interface, so in order to use it, you will need to have a C program that builds resources for you. It has it's own repo, but i will be adding CLI to there soon.
* Examples: see [this repo](https://github.com/justanothercatgirl/embed_test.c/)
## `build.h`
nearest TODO for now.
* Summary: a build system based on C. To compile something under it, you do something like `cc -o builder builder.c && ./builder`. The idea is stolen from [Tsoding](https://example.com)
# Warning!
Everything here is written by a relatively inexperienced student (me), so I guarantee basically nothing. Memory leaks? I am sorry for them, but also not responsible. Security? Haven't heard of that either. That's just how it is. But I try to make everything as good as possible, so you can use the code after a careful review.
# License
I did not decide on it yet, but I am inclined towards MIT or even public domain. Don't want to restrict the usage with LGPL, because it's not like this code is a big deal, anyone could write it. Also, I don't want to deal with license issues that I created for myself in future, so yeah, that's how it is. For now, public domain.

