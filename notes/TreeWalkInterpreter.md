## Notes: Tree Walk Interpreter

The error handling scheme that we set up at the entry point to the interpreter (`Lox.java`) bothers me quite a bit. We end up referencing the static error-handling methods quite a bit later (e.g. the parser implementation in `Parser.java` calls these) and this bothers me because of the dependency this introduces between these two logically-distinct components. As the author notes in the text, a more appropriate (i.e. better design) way to handle this situation would be with dependency injection - pass something that implements an `ErrorReporter` interface to the components (the `Scanner` and the `Parser` come to mind) and delegate to this injected functionality whenever an error occurs.

**Scanner**

I didn't find any aspects of the scanner implementation particularly suprising. This is a good thing - it is nice to know that the scanning / lexing phase, at least for a language as simple as Lox, is accordingly simple to implement!

**Grammar Design**

The design of the grammar for Lox makes senses to me, but I find myself wondering: if I were designing this language from scratch, how would I ensure that the grammar I designed is sufficiently expressive? It seems simple enough to express a grammar formally in BNF, but then how do we ensure that we will be able to construct all of the language constructs that we want to construct?

Perhaps this is an iterative process, and we would need to update our grammar as we discover additional language constructs that we want to support?

**Parser**

TODO

**Evaluting Expressions**

The discussion regarding the way in which language constructs are represented in the underlying representation is an interesting one. For instance, all values in Lox are represented as Java `Object`s. The fact that Java allows us to determine the dynamic type of an `Object` at runtime comes in real handy here, not to mention the fact that all of the memory is managed for us.

In the section on evaluating literals the author points out that we _eagerly_ evaluated the literal during scanning and stuffed it into the corresponding token. This makes me wonder if there is a way to defer this evaluation of the literal - to make it lazy? Perhaps this doesn't even matter for literals?

The "corners" of the implementation - those places where the fact that we represent Lox values as Java objects matters. Example: testing for equality and inequality of Lox values. Lox's notion of equality and inequality may be different from Java's notion, and we need to ensure that we implement Lox's semantics in the Java world.