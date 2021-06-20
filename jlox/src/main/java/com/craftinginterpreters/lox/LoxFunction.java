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
   * Construct a new LoxFunction instance.
   * @param declaration The associated declaration
   */
  public LoxFunction(final FunctionStmt declaration) {
    this.declaration = declaration;
  }

  @Override
  public int arity() {
    return declaration.params.size();
  }

  @Override
  public Object call(Interpreter interpreter, final List<Object> arguments) {
    // Create a new environment for the function, with the global
    // interpreter environment set as the enclosing environment
    Environment environment = new Environment(interpreter.getGlobals());
    // Define each of the function arguments in the environment
    for (int i = 0; i < declaration.params.size(); ++i) {
      // Bind the value passed to the function to the appropriate name
      environment.define(declaration.params.get(i).getLexeme(), arguments.get(i));
    }
    // Execute the function body on the interpreter
    interpreter.executeBlock(declaration.body, environment);
    return null;
  }

  @Override
  public String toString() {
    return "<fn " + declaration.name.getLexeme() + ">";
  }
}
