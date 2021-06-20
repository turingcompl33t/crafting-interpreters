/**
 * StmtVisitor.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The StmtVisitor class implements the visitor pattern for Lox AST statements.
 */
public interface StmtVisitor<R> {
  public R visitBlockStmt(BlockStmt stmt);
  public R visitExpressionStmt(ExpressionStmt stmt);
  public R visitPrintStmt(PrintStmt stmt);
  public R visitVarStmt(VarStmt stmt);
}
