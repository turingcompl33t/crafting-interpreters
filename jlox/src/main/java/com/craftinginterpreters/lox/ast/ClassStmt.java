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
   * The body of the class declaration (methods).
   */
  public final List<FunctionStmt> body;

  /**
   * Construct a new ClassStmt instance.
   * @param name The class name
   * @param body The class body (methods)
   */
  public ClassStmt(final Token name, final List<FunctionStmt> body) {
    this.name = name;
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