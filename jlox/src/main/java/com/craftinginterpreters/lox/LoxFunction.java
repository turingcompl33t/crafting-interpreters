/**
 * LoxFunction.java
 */

package com.craftinginterpreters.lox;

import java.util.List;

import com.craftinginterpreters.lox.ast.FunctionStmt;

/**
 * The LoxFunction class represents a user-defined function.
 */
public class LoxFunction implements LoxCallable {
  /**
   * The associated function declaration.
   */
  private final FunctionStmt declaration;

  /**
   * The captured environment.
   */
  private final Environment closure;

  /**
   * Denotes whether this function is a class initializer.
   */
  private final boolean isInitializer;

  /**
   * Construct a new LoxFunction instance.
   * @param declaration The associated declaration
   * @param closure The captured environment
   */
  public LoxFunction(final FunctionStmt declaration, final Environment closure,
                     final boolean isInitializer) {
    this.declaration = declaration;
    this.closure = closure;
    this.isInitializer = isInitializer;
  }

  /**
   * Create a LoxFunction bound to a particular instance.
   * @param instance The instance to which the function is bound
   * @return The bound function
   */
  LoxFunction bind(LoxInstance instance) {
    final Environment env = new Environment(closure);
    env.define("this", instance);
    return new LoxFunction(declaration, env, isInitializer);
  }

  @Override
  public int arity() {
    return declaration.params.size();
  }

  @Override
  public Object call(Interpreter interpreter, final List<Object> arguments) {
    // Create a new environment for the function, with the global
    // interpreter environment set as the enclosing environment
    Environment environment = new Environment(closure);
    // Define each of the function arguments in the environment
    for (int i = 0; i < declaration.params.size(); ++i) {
      // Bind the value passed to the function to the appropriate name
      environment.define(declaration.params.get(i).getLexeme(), arguments.get(i));
    }
    // Execute the function body on the interpreter
    try {
      interpreter.executeBlock(declaration.body, environment);
    } catch (Return returnValue) {
      // Allow early returns in initializers
      if (isInitializer) return closure.getAt(0, "this");
      return returnValue.value;
    }

    // If the function is a class initializer, we override
    // the return value and always return `this`
    if (isInitializer) return closure.getAt(0, "this");
    return null;
  }

  @Override
  public String toString() {
    return "<fn " + declaration.name.getLexeme() + ">";
  }
}
