/**
 * TokenTest.java
 */

package com.craftinginterpreters.lox;

import org.junit.Test;

import static org.junit.Assert.assertTrue;

public class TokenTest {
  @Test
  public void tokensAreEqual() {
    final Token t0 = new Token(TokenType.AND, "and", null, 1);
    final Token t1 = new Token(TokenType.AND, "and", null, 1);
    assertTrue(t0.equals(t1));
  }
}
