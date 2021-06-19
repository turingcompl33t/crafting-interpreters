/**
 * Token,java
 */

package com.craftinginterpreters.lox;

/**
 * The Token class represents a single token parsed from input source.
 */
public class Token {
  /**
   * The TokenType for the token.
   */
  final TokenType type;

  /**
   * The token lexeme.
   */
  final String lexeme;

  /**
   * The token literal.
   */
  final Object literal;

  /**
   * The line number on which the token appears.
   */
  final int line;

  /**
   * Construct a new Token instance.
   * @param type The type of the token
   * @param lexeme The token lexeme
   * @param literal The token literal
   * @param line The token line number
   */
  Token(final TokenType type, final String lexeme, final Object literal, final int line) {
    this.type = type;
    this.lexeme = lexeme;
    this.literal = literal;
    this.line = line;
  }

  /**
   * @return The token type
   */
  public TokenType getType() {
    return type;
  }

  /**
   * @return The token lexeme
   */
  public String getLexeme() {
    return lexeme;
  }

  /**
   * @return The token literal
   */
  public Object getLiteral() {
    return literal;
  }

  /**
   * @return The token line number
   */
  public int getLine() {
    return line;
  }

  /**
   * Create a string representation.
   * @return A string representation of the token.
   */
  public String toString() {
    StringBuilder builder = new StringBuilder();
    builder.append(type);
    builder.append(" ");
    builder.append(lexeme);
    builder.append(" ");
    builder.append(literal);
    return builder.toString();
  }

  /**
   * Determine if two tokens are equivalent.
   * @param other
   * @return
   */
  public boolean equals(final Token other) {
    // All tokens must have a type and a lexeme
    if (!type.equals(other.getType())) {
      return false;
    }
    if (!lexeme.equals(other.getLexeme())) {
      return false;
    }

    // Compare literals (gross logic)
    if (literal == null) {
      if (other.getLiteral() != null) {
        return false;
      }
    } else {
      if (other.getLiteral() == null) {
        return false;
      }
      if (literal.equals(other.getLiteral())) {
        return false;
      }
    }

    // Finally, compare line numbers
    return line == other.getLine();
  }
}