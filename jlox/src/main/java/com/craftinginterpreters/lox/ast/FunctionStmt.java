/**
 * FunctionStmt.java
 */

package com.craftinginterpreters.lox.ast;

import java.util.List;
import com.craftinginterpreters.lox.Token;

/**
 * The FunctionStmt class represents a Lox AST function.
 */
public class FunctionStmt extends Stmt {
  /**
   * The function name.
   */
  public final Token name;

  /**
   * The function (formal) parameters.
   */
  public final List<Token> params;

  /**
   * The function body.
   */
  public final List<Stmt> body;

  /**
   * Construct a new FunctionStmt instance.
   * @param name The function name
   * @param params The function parameters
   * @param body The function body
   */
  public FunctionStmt(final Token name, final List<Token> params, final List<Stmt> body) {
    this.name = name;
    this.params = params;
    this.body = body;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitFunctionStmt(this);
  }

  @Override
  public String toString() {
    return "FunctionStmt";
  }
}
