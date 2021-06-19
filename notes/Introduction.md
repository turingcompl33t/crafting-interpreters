## Notes: Introduction

Logical operators `and` and `or` as control flow in disguise. These are really control flow because of their short-circuiting behavior.
- `x and y` -> returns `x` if `x` is `false`, `y` otherwise
- `x or y` -> returns `x` if `x` is `true`, `y` otherwise

While expressions produce a _value_, statements produce an _effect_.
- Are statements always a higher-level construct than expressions?
- A statment is typically composed of one or more expressions
- Is it possible to have the opposite? 
- An expression that is suffixed with a semicolon becomes a statement!
- What about functional languages that do not support statements at all? Does this simplify the implementation?

Object-oriented languages
- Class-based
- Prototype-based