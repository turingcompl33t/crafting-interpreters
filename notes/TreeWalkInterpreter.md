## Notes: Tree Walk Interpreter

The error handling scheme that we set up at the entry point to the interpreter (`Lox.java`) bothers me quite a bit. We end up referencing the static error-handling methods quite a bit later (e.g. the parser implementation in `Parser.java` calls these) and this bothers me because of the dependency this introduces between these two logically-distinct components. As the author notes in the text, a more appropriate (i.e. better design) way to handle this situation would be with dependency injection - pass something that implements an `ErrorReporter` interface to the components (the `Scanner` and the `Parser` come to mind) and delegate to this injected functionality whenever an error occurs.

**Scanner**

I didn't find any aspects of the scanner implementation particularly suprising. This is a good thing - it is nice to know that the scanning / lexing phase, at least for a language as simple as Lox, is accordingly simple to implement!

**Grammar Design**

The design of the grammar for Lox makes senses to me, but I find myself wondering: if I were designing this language from scratch, how would I ensure that the grammar I designed is sufficiently expressive? It seems simple enough to express a grammar formally in BNF, but then how do we ensure that we will be able to construct all of the language constructs that we want to construct?

Perhaps this is an iterative process, and we would need to update our grammar as we discover additional language constructs that we want to support?

**Parser**

I was genuinely surprised by the simplicitly and power of the recursive descent parser. There really weren't any surprises in the implementation of the parser, and I actually really enjoyed the process because we let the Java type system take care of so much of the heavy lifting. The AST is verbose in the sense that we implement a distinct Java class for each node type in the AST, but this definitely pays off in making the parser comprehensible and straightforward to implement.

The one aspect of the parser that took me some time to wrap my head around was the inverse relationship between the precedence of a parsed object (i.e. an expression or a statement) and the order in which we attempt to parse these objects from the input sequence of tokens. Intuitively, however, this does make sense: we look for the highest-level structures first and descend through these higher-level structures to progressively lower-level structures until we hit the highest-precedence productions - the "atomic units" of our syntax like primary expressions.

The other really interesting aspect of the parser implementation is the recovery mechanism: how do we recover from syntax errors in the parser? This is a difficult question. The solution that we come up with in this implementation is to (essentially) enter a panic mode when we encounter a syntax error and continue in this error state until we complete the current statement. This is an intuitive solution because we suspect that syntax errors are contained to a single line? What are alternatives to this approach? For instance, would there be a way to recover sooner than at the end of the statement?

**Evaluting Expressions**

The discussion regarding the way in which language constructs are represented in the underlying representation is an interesting one. For instance, all values in Lox are represented as Java `Object`s. The fact that Java allows us to determine the dynamic type of an `Object` at runtime comes in real handy here, not to mention the fact that all of the memory is managed for us.

In the section on evaluating literals the author points out that we _eagerly_ evaluated the literal during scanning and stuffed it into the corresponding token. This makes me wonder if there is a way to defer this evaluation of the literal - to make it lazy? Perhaps this doesn't even matter for literals?

The "corners" of the implementation - those places where the fact that we represent Lox values as Java objects matters. Example: testing for equality and inequality of Lox values. Lox's notion of equality and inequality may be different from Java's notion, and we need to ensure that we implement Lox's semantics in the Java world.

**Statements and State**

The big design note in this chapter comes from the answer to the question: how do we implement persistent state in our tree-walk interpreter? Evaluating expressions is one thing, and perhaps in a pure functional language this would be the _only_ thing, but because Lox is (loosely) and imperative language, we need some way to track the state manipulated by our program as it is executed.

The high-level principle that we use for state management are _environments_ - essentially just maps from identifiers to runtime values. The concept is simple enough, but some interesting nuances arise in the implementation.

The final implementation we arrive at is (basically) a linked hierarchy of environments that correspond to nested lexical scopes in the program source. When we encounter a "read" on a variable within a certain scope, we consult its corresponding environment and attempt to read the runtime value that corresponds to the identifier. If the identifier is not present in this environment, we repeat the process up the environment hierarchy until we reach the top-level environment which corresponds to the global scope. If the variable is not found at this point, it is a reference to an undefined variable. Beginning from the lowest-level environment ensures that we respect variable _shadowing_ semantics.

**Control Flow**

Conditional control flow and while-loops are straightforward.

The interesting aspect of this chapter is the implementation of for-loops. As we are all taught in our introduction to programming course, a `for`-loop is really just syntactic sugar for a `while`-loop - there is nothing that one can express with a `for`-loop that cannot be equivalently expressed with a `while`-loop. We take advantage of this fact in Lox by simply desugaring (converting) a `for`-loop in the program source to an equivalent `while`-loop construct during the parsing phase. This allows us to take advantage of the implementation of the `while`-loop interpretation code to execute `for`-loop constructs - stay DRY!

**Functions**

The magic in this chapter is the transformation of a call expression in the Lox syntax tree to a runtime callable object (in this case a `LoxFunction` instance).

To do this, we essentially package up the environment in which the environment executes with the code that composes the body of the function. We do this by evaluating each of the arguments to the function, defining these in the function's environment, and then essentially just transferring control flow into the body of the function.

Another interesting aspect of the implementation that arises in this chapter is the implementation of `return` statements. We managed control flow in these cases with Java exceptions which allows us to teleport up the Java runtime call stack to the point that we need to get to.

**Resolving and Binding**

In this chapter we essentially implement a simple semantic analysis pass over the AST that is run prior to interpretation. The resolver pass walks the syntax tree and resolves each variable reference. This is necessary to avoid instances in which we have closures that capture variables with values that change between invocations. In these cases, we expect (from the programmer's point of view) that the captured value will remain the same, even after reassignment of the variable. However, in our previous implementation this is not the case.

The resolver pass is one way to avoid this phenomenon. An alternative would be to make environments immutable, and effectively create a new environment with each new variable declaration. We opt for the resolver pass in this implementation to take advantage of the code we already wrote.
