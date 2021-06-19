/**
 * AstPrinterTest.java
 */

package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Arrays;

import org.junit.Test;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertEquals;


public class AstPrinterTest {
  @Test
  public void itWorks() {
    // -123 * 45.67
    Expr expression = new BinaryExpr(
      new UnaryExpr(
        new Token(TokenType.MINUS, "-", null, 1),
        new LiteralExpr(123)),
      new Token(TokenType.STAR, "*", null, 1),
      new GroupingExpr(
        new LiteralExpr(45.67)));
    final String actual = new AstPrinter().print(expression);
    final String expected = "(* (- 123) (group 45.67))";
    assertEquals(expected, actual);
  }
}
