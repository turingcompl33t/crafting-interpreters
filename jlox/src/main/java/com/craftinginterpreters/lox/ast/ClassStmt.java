/**
 * ClassStmt.java
 */

package com.craftinginterpreters.lox.ast;

import java.util.List;
import com.craftinginterpreters.lox.Token;

/**
 * The ClassStmt class represents a Lox AST class declaration.
 */
public class ClassStmt extends Stmt {
  /**
   * The name of the class.
   */
  public final Token name;

  /**
   * The class superclass (if applicable).
   */
  public final VariableExpr superclass;

  /**
   * The body of the class declaration (methods).
   */
  public final List<FunctionStmt> body;

  /**
   * Construct a new ClassStmt instance.
   * @param name The class name
   * @param superclass The class superclass (if applicable)
   * @param body The class body (methods)
   */
  public ClassStmt(final Token name, final VariableExpr superclass,
                   final List<FunctionStmt> body) {
    this.name = name;
    this.superclass = superclass;
    this.body = body;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitClassStmt(this);
  }

  @Override
  public String toString() {
    return "ClassStmt";
  }
}