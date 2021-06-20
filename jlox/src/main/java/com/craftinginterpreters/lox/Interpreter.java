/**
 * Interpreter.java
 */

package com.craftinginterpreters.lox;

import java.util.List;

import com.craftinginterpreters.lox.ast.*;

/**
 * The Interpreter class implements a tree-walk interpreter for Lox.
 */
public class Interpreter implements ExprVisitor<Object>, StmtVisitor<Void> {

  /**
   * The global interpreter environment.
   */
  private Environment environment = new Environment();

  /**
   * Interpret a Lox program.
   * @param statements The list of statements that compose the program
   */
  public void interpret(final List<Stmt> statements) {
    try {
      for (final Stmt statement : statements) {
        execute(statement);
      }
    } catch (RuntimeError error) {
      Lox.runtimeError(error);
    }
  }

  /**
   * Evaluate the expression.
   * @param expression The expression to evaluate
   * @return The runtime value
   */
  private Object evaluate(final Expr expr) {
    return expr.accept(this);
  }

  /**
   * Execute a statement.
   * @param statement The statement to execute
   */
  private void execute(final Stmt statement) {
    statement.accept(this);
  }

  private void executeBlock(final List<Stmt> statements, Environment environment) {
    final Environment previous = this.environment;
    try {
      // Execute the block in the provided environment
      this.environment = environment;
      for (final Stmt statement : statements) {
        execute(statement);
      }
    } finally {
      // Restore the environment
      this.environment = previous;
    }
  }

  // --------------------------------------------------------------------------
  // Statement Visitors
  // --------------------------------------------------------------------------

  /**
   * Evaluate an expression statement.
   * @param stmt The statement
   * @return null
   */
  @Override
  public Void visitExpressionStmt(final ExpressionStmt stmt) {
    evaluate(stmt.expression);
    return null;
  }

  /**
   * Evaluate a while-statement.
   * @param stmt The statement
   * @return null
   */
  @Override
  public Void visitWhileStmt(final WhileStmt stmt) {
    // This translation from the AST to interpretation is beautiful
    while (isTruthy(evaluate(stmt.condition))) {
      execute(stmt.body);
    }
    return null;
  }

  /**
   * Evaluate an if-statement.
   * @param stmt The statement
   * @return null
   */
  @Override
  public Void visitIfStmt(final IfStmt stmt) {
    if (isTruthy(evaluate(stmt.condition))) {
      execute(stmt.thenBranch);
    } else if (stmt.elseBranch != null) {
      execute(stmt.elseBranch);
    }
    return null;
  }

  /**
   * Evaluate a print statement.
   * @param stmt The statement
   * @return null
   */
  @Override
  public Void visitPrintStmt(final PrintStmt stmt) {
    final Object value = evaluate(stmt.expression);
    System.out.println(stringify(value));
    return null;
  }

  /**
   * Evaluate a variable declaration statement.
   * @param stmt The statement
   * @return null
   */
  @Override
  public Void visitVarStmt(final VarStmt stmt) {
    final Object value = stmt.initializer != null ? evaluate(stmt.initializer) : null;
    environment.define(stmt.name.getLexeme(), value);
    return null;
  }

  /**
   * Evaluate a block statement.
   * @param stmt The statement
   * @return null
   */
  @Override
  public Void visitBlockStmt(final BlockStmt stmt) {
    executeBlock(stmt.statements, new Environment(environment));
    return null;
  }

  // --------------------------------------------------------------------------
  // Expression Visitors
  // --------------------------------------------------------------------------
  
  /**
   * Evaluate a literal expression.
   * @param expr The expression
   * @return The runtime value
   */
  @Override
  public Object visitLiteralExpr(final LiteralExpr expr) {
    return expr.value;
  }

  /**
   * Evaluate a logical expression.
   * @param expr The expression
   * @return The runtime value
   */
  @Override
  public Object visitLogicalExpr(final LogicalExpr expr) {
    // Here we see the short-circuiting behavior of
    // the logical operators in action: the right side
    // of the expression is not evaluated in the event
    // that we can determine the result without it
    final Object left = evaluate(expr.left);
    if (expr.operator.getType() == TokenType.OR) {
      if (isTruthy(left)) return left;
    } else {
      if (!isTruthy(left)) return left;
    }
    return evaluate(expr.right);
  }

  /**
   * Evaluate a grouping expression.
   * @param expr The expression
   * @return The runtime value
   */
  @Override
  public Object visitGroupingExpr(final GroupingExpr expr) {
    return evaluate(expr.expression);
  }

  /**
   * Evaluate a unary expression.
   * @param expr The expression
   * @return The runtime value
   */
  @Override
  public Object visitUnaryExpr(final UnaryExpr expr) {
    final Object right = evaluate(expr.expression);
    switch (expr.operator.getType()) {
      case BANG:
        return !isTruthy(right);
      case MINUS:
        checkNumberOperand(expr.operator, right);
        return -(double)right;
    }

    // Unreachable
    return null;
  }

  /**
   * Evaluate a binary expression.
   * @param expr The expression
   * @return The runtime value
   */
  @Override
  public Object visitBinaryExpr(final BinaryExpr expr) {
    final Object left = evaluate(expr.left);
    final Object right = evaluate(expr.right);

    switch (expr.operator.getType()) {
      case GREATER:
        checkNumberOperands(expr.operator, left, right);
        return (double)left > (double)right;
      case GREATER_EQUAL:
        checkNumberOperands(expr.operator, left, right);
        return (double)left >= (double)right;
      case LESS:
        checkNumberOperands(expr.operator, left, right);
        return (double)left < (double)right;
      case LESS_EQUAL:
        checkNumberOperands(expr.operator, left, right);
        return (double)left <= (double)right;
      case MINUS:
        checkNumberOperands(expr.operator, left, right);
        return (double)left - (double)right;
      case PLUS:
        // Numeric addition
        if (left instanceof Double && right instanceof Double) {
          return (double)left + (double)right;
        }
        // String concatenation
        if (left instanceof String && right instanceof String) {
          return (String)left + (String)right;
        }
        throw new RuntimeError(expr.operator, "Operands must both be numeric or string.");
      case SLASH:
        checkNumberOperands(expr.operator, left, right);
        return (double)left / (double)right;
      case STAR:
        checkNumberOperands(expr.operator, left, right);
        return (double)left * (double)right;
      case BANG_EQUAL:
        return !isEqual(left, right);
      case EQUAL_EQUAL:
        return isEqual(left, right);
    }

    // Unreachable
    return null;
  }

  /**
   * Evaluate a variable expression.
   * @param expr The expression
   * @return The runtime value
   */
  @Override
  public Object visitVariableExpr(final VariableExpr expr) {
    return environment.get(expr.name);
  }

  /**
   * Evaluate an assignment expression.
   * @param expr The expression
   * @return The runtime value
   */
  @Override
  public Object visitAssignExpr(final AssignExpr expr) {
    final Object value = evaluate(expr.value);
    environment.assign(expr.name, value);
    return value;
  }

  // --------------------------------------------------------------------------
  // TODO
  // --------------------------------------------------------------------------

  /**
   * Determine if a value is "truthy" in Lox.
   * @param value The value
   * @return `true` if the Lox runtime value is truthy, `false` otherwise
   */
  private boolean isTruthy(final Object value) {
    // Lox adopts the convention from Ruby:
    // `nil` and `false` are falsey, everything else is truthy

    if (value == null) return false;
    if (value instanceof Boolean) return (boolean)value;
    return true;
  }

  /**
   * Determine the equality of two runtime values.
   * @param a Runtime value
   * @param b Runtime value
   * @return `true` if the runtime values are equal, `false` otherwise
   */
  private boolean isEqual(final Object a, final Object b) {
    if (a == null && b == null) return true;
    if (a == null) return false;
    return a.equals(b);
  }

  /**
   * Check that the unary operator is applicable to the operand.
   * @param operator The operator
   * @param operand The operand
   */
  private void checkNumberOperand(final Token operator, final Object operand) {
    if (operand instanceof Double) return;
    throw new RuntimeError(operator, "Operand must be numeric.");
  }

  /**
   * Check that the binary operator is applicable to the operands.
   * @param operator The operator
   * @param left The left operand
   * @param right The right operand
   */
  private void checkNumberOperands(final Token operator, final Object left, final Object right) {
    if (left instanceof Double && right instanceof Double) return;
    throw new RuntimeError(operator, "Operands must be numeric.");
  }

  /**
   * Create a string representation of a Lox runtime value.
   * @param value The runtime value
   * @return The string representation
   */
  private String stringify(final Object value) {
    if (value == null) return "nil";
    if (value instanceof Double) {
      final String text = value.toString();
      return text.endsWith(".0") ? text.substring(0, text.length() - 2) : text;
    }
    return value.toString();
  }
}