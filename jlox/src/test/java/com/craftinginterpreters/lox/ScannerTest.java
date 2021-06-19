/**
 * ScannerTest.java
 */

package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Arrays;

import org.junit.Test;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertEquals;

public class ScannerTest {
  /**
   * Create a new EOF token on the given line.
   * @param line The line number
   * @return The EOF token
   */
  private static Token eofTokenWithLine(final int line) {
    return new Token(TokenType.EOF, "", null, line);
  }

  /**
   * Determine if the tokens contained in two token lists are equivalent.
   * @param tokens0 Input list of tokens
   * @param tokens1 Input list of tokens
   * @return `true` if the two lists contain the same tokens in the same order, `false` otherwise
   */
  private static boolean tokenListsAreEqual(final List<Token> tokens0, final List<Token> tokens1) {
    if (tokens0.size() != tokens1.size()) {
      return false;
    }
    for (int i = 0; i < tokens0.size(); ++i) {
      final Token t0 = tokens0.get(i);
      final Token t1 = tokens1.get(i);
      if (!t0.equals(t1)) {
        return false;
      }
    }
    return true;
  }

  @Test
  public void scansExpectedTokens0() {
    final String source = "123";
    final List<Token> expected = Arrays.asList(
      new Token(TokenType.NUMBER, "123", Double.valueOf(source), 1),
      eofTokenWithLine(1)
      );
    
    Scanner scanner = new Scanner(source);
    final List<Token> scanned = scanner.scanTokens();
    assertEquals(2, scanned.size());
    assertTrue(tokenListsAreEqual(scanned, expected));
  }

  @Test
  public void scansExpectedTokens1() {
    final String source = "xyz";
    final List<Token> expected = Arrays.asList(
      new Token(TokenType.IDENTIFER, "xyz", null, 1),
      eofTokenWithLine(1)
      );
    
    Scanner scanner = new Scanner(source);
    final List<Token> scanned = scanner.scanTokens();
    assertEquals(2, scanned.size());
    assertTrue(tokenListsAreEqual(scanned, expected));
  }

  @Test
  public void scansExpectedTokens2() {
    final String source = "x = 1";
    final List<Token> expected = Arrays.asList(
      new Token(TokenType.IDENTIFER, "x", null, 1),
      new Token(TokenType.EQUAL, "=", null, 1),
      new Token(TokenType.NUMBER, "1", Double.valueOf("1"), 1),
      eofTokenWithLine(1)
      );
    
    Scanner scanner = new Scanner(source);
    final List<Token> scanned = scanner.scanTokens();
    assertEquals(4, scanned.size());
    assertTrue(tokenListsAreEqual(scanned, expected));
  }
}
