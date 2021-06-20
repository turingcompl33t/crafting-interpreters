/**
 * VarStmt.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The VarStmt class represents a Lox AST variable declaration statement.
 */
public class VarStmt extends Stmt {
  /**
   * The name of the variable that is declared.
   */
  public final Token name;

  /**
   * The expression that initializes the variable.
   */
  public final Expr initializer;

  /**
   * Construct a new VarStmt instance.
   * @param name The name of the declared variable
   * @param initializer The expression used to initialize the variable
   */
  public VarStmt(final Token name, final Expr initializer) {
    this.name = name;
    this.initializer = initializer;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitVarStmt(this);
  }

  @Override
  public String toString() {
    return "VarStmt";
  }
}
