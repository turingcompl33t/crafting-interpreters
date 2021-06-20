/**
 * Interpreter.java
 */

package com.craftinginterpreters.lox;

import com.craftinginterpreters.lox.ast.*;

/**
 * The Interpreter class implements a tree-walk interpreter for Lox.
 */
public class Interpreter implements ExprVisitor<Object> {
  /**
   * Construct a new Interpreter instance.
   */
  Interpreter() {

  }

  /**
   * Interpret the expression.
   * @param expression The expression
   */
  public void interpret(final Expr expression) {
    try {
      final Object value = evaluate(expression);
      System.out.println(stringify(value));
    } catch (RuntimeError error) {
      Lox.runtimeError(error);
    }
  }

  /**
   * Evaluate the expression; a simple way to recurse within the interpreter.
   * @param expression The expression to evaluate
   * @return The runtime value
   */
  private Object evaluate(final Expr expr) {
    return expr.accept(this);
  }
  
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
   * Evaluate a grouping expression.
   * @param expr The expression
   * @return The runtime value
   */
  @Override
  public Object visitGroupingExpr(final GroupingExpr expr) {
    return evaluate(expr.expression);
  }

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