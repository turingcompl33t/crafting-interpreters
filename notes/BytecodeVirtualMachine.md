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
