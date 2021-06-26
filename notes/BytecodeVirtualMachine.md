## Notes: Bytecode Virtual Machine

**Chunks of Bytecode**

The first interesting design note in this chapter is hwo to handle immediate instructions. The approach we take uses a _constant pool_ in each chunk of bytecode. This constant pool is essentially another dynamic array of constant values to which we can write data that is used by the bytecode instructions within the chunk. In our implementation, we use the constant pool for ALL constant values - primitive operands as well as non-primitive operands like string constants. This simplifies the engineering, but an alternative approach might be to simply inline the constant for some immediate values directly into the instruction. This would increase locality for what might be very common operations.

The constant pool approach is used by the [Java Virtual Machine](https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html#jvms-4.4).

Another interesting design consideration is source information: how do we encode source information in our bytecode stream? In the tree-walk interpreter this was simplified because each token stored its source location, and the tokens were readily available while walking the syntax tree during interpretation, so we always had source information available. Naturally, we ditch all of the information stored in the syntax tree when we compile to bytecode, so we need a new mechanism to store source information. The approach we take in this chapter is to add another auxiliary dynamic array of source information to each bytecode chunk. Now we maintain two parallel arrays - one for the bytecode stream and one for the source information. This has pros and cons:

Pros:
- Simple to implement
- Keeps the byetcode stream and source information separate; we don't sacrifice locality of the bytecode stream for the source information

Cons:
- Extremely wasteful: for every single byte we write to the bytecode stream, we write a 4-byte integer to the source information array. This is even worse than it initially appears because it means that we write multiple values to the source information array for instructions that might take up more than one byte (e.g. any instruction with operands).

A suggested optimization is run-length encoding. Are there any other alternatives?

I find myself tempted to increase the levels of abstraction in this implementation already. For instance, I don't like the fact that the `Chunk` type itself is managing its dynamic arrays (at least some of them). I want to pull the dynamic array implementation out into a separate class, write some tests for it, and only then utilize it the `Chunk` class. This is something I will do when I get to my own language implementation...

**A Virtual Machine**

We make an obvious design concession when we choose to use a global VM instance instead of passing a pointer to the VM around in the `vm` module. If I were writing this in C++, would I implement this as a stateful class with instance methods?

We store the instruction pointer for the VM as part of the `vm` structure. The author notes that if we wanted better performance, we would actually pass this value around to each interpretation function in order to allow the C compiler to take advantage of registers more effectively (also a locality concern?).

Our virtual machine run loop is pretty simple. The alternative approaches to doing this are worth taking a look at:
- Direct threaded
- Indirect threaded
- etc.

The simplicity of the virtual machine implementation is also its virtue - the less work it does, or the less overhead it imposes over the code itself, the faster it can do it!

The concept of distinguishing between operations that accept operands and instructions that accept operands. For instance, the `OP_CONSTANT` instruction accepts an operand that tells it the index in the constant pool to read. In contrast, the instructions that implement binary arithmetic do not take any operands - instead they always look for both of their operands at the top of the runtime stack.

Is there latitude here for choosing? I suppose this would not be possible to encode the operand values for the binary operators directly into the instruction because the value is not known at compile time! This is the beauty of the stack-based virtual machine.

"That is the magic of the stack. It lets us freely compose instructions without them needed any complexity or awareness of data flow."

The choice of a stack-based virtual machine over a register-based implementation is another important design decision that comes out of this chapter. The difference I noted above is a result of the stack based VM rather than a register-based VM. The `add` instruction is a nice example of this:

In a stack-based VM:

```
load <a>   // Read local a and push onto stack
load <b>   // Read local b and push onto stack
add        // Perform addition, push result onto stack
store <c>  // Read the value at the top of stack, store in local c
```

In a register-based VM:

```
// Read the operands at <a> and <b>, store the result in <c>
add <a> <b> <c>
```

The author calls out the paper [The Implementation of Lua 5.0](https://www.lua.org/doc/jucs05.pdf) as an example of a case study in transitioning from a stack-based VM to a register-based implementation.

**Scanning on Demand**

The big design feature in this chapter is the fact that (as the title suggests) we do not scan all of the tokens in the input source ahead of time and pass these to the compiler. Instead, we expose a function that provides the next token from the source stream on-demand. This saves us the trouble of managing the memory associated with all of the tokens (making this approach more scalable). The only limitation is that we must ensure that the source string has a lifetime that lasts at least through the duration of compilation. In practice this is easy, but I am generally never a fan of coupling lifetimes across components like this.

**Compiling Expressions**

The use of a single-pass compiler implementation. This is in contrast to a multi-pass approach in which we first generate the AST (completely) and then use the completed AST to generate bytecode. Instead, we simply generate bytecode on the fly as we construct the AST.

The Pratt parsing technique. Did not make sense to me initially, but after walking through some examples, it is beautiful. Use the `-1 + 2` example from the source.

Think about it in terms of binding power instead of precedence. Reference [this post](https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html). Original paper from Pratt is [here](https://web.archive.org/web/20151223215421/http://hall.org.ua/halls/wizzard/pdf/Vaughan.Pratt.TDOP.pdf).

**Types of Values**

The idea of representing Lox values in C.

Another important concept from this chapter is the idea of translating the user's source code to the virtual machine bytecode instruction set. We do not necessarily need an bytecode instruction for each of the operations in the user's source code, but rather just a way to model the computation on our virtual machine. Example in this chapter is the use of OP_GREATER and OP_NOT to implement OP_LESS_EQUAL, and vice versa for OP_GREATER_EQUAL. Keeps the instruction set smaller and the code somewhat simpler, at the cost of potential performance?

**Strings**

We adopt a two-level representation for Lox values. Small values are stored inline in the `Value` representation. In contrast, types like strings, instances, and functions will have their memory allocated on the heap and the `Value` itself will only maintain a pointer to the object itself. Thus we have two different "kinds" of types in our Lox implementation: primitive types and object types (need better identifiers here?).

This was a super fun chapter. The ability to represent arbitrary object-style values in Lox is a huge addition to the language, and the choices we make here are very consequential ones for performance.

**Hash Tables**

I recoil a little bit from the use of a hashtable implementation that is not totally generic - we hardcode the use of Lox StringObjects as the key type in the table, and use Lox Values as the mapped value.

String interning is a new concept.

Thinking about what operations are common in language implementation, and how to make them fast.

Why are string lookups common in Lox? Is this the case for any dynamic language?

**Global Variables**

The idea of multiple implementation strategies for the same high-level feature based on the level of optimization required.