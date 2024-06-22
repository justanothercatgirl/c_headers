# What is this?
This is just a collection of (usually single-purpos) header files that I use refularly in my code
# Common conventions
* As these are header-only libraries, there is a macro in form `HEADERNAME_IMPLEMENTATION` that has to be defined before including the header. A header can only be included once while this macro is defined, otherwise you get multiple definition errors.
* Names starting with underscores are kind of private-use for this libraries, but they may be declared not `static` because they are used in macros that expose API.
* `snake_case`
* Function-like macros are not all-caps.
* For now i only support GCC-based compilers. I will consider removing this dependency in future (e.g. getting rid of all `typeof` uses)
# Descriptions for heders:
## [`container.h`](include/container.h)
* Summary: originally was several headers, but since `hash_map.h` used `dynarray.h`, which itself used `utility.h`, it was too complex to work with. So i combined everything in a single header.
* How to use: Define `CONTAINER_IMPLEMENTATION` macro before including `container.h`. It's probably better to `#undef` it after inclusion as well
* Examples: See [tests](tests)
## [`rstypes.h`](include/rstypes.h)
* Summary: rust type aliases (like `u32`, `f128` e.t.c.)
* How to use: if you don't need `i128` or `u128`, then just include the header. If you do need them, define macro `RS_TYPES_USE_128`.
* Examples: See [tests](tests/types.c).
* Notes: Not many compilers support 128-bit wide integers, and ones which DO support them tend to warn you that these are non ISO-C. That's why I've put them behind a macro. If they are not supported, their usage will fail with compilation error.
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
I did not decide on it yet, but I am inclined towards MIT or even public domain. Don't want to restrict the usage with LGPL, because it's not like this code is a big deal, anyone could write it. Also, I don't want to deal with license issues that I can create for my future self, so yeah, that's how it is. For now, public domain.

