/**
 * Scanner.java
 */

package com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * The Scanner class implements the Lox source scanner.
 */
public class Scanner {
  /**
   * The input source.
   */
  private final String source;

  /**
   * The collection of tokens produced during scanning.
   */
  private final List<Token> tokens = new ArrayList<>();

  /**
   * Collection of reserved words and corresponding token types.
   */
  private static final Map<String, TokenType> KEYWORDS;

  static {
    KEYWORDS = new HashMap<>();
    KEYWORDS.put("and", TokenType.AND);
    KEYWORDS.put("class", TokenType.CLASS);
    KEYWORDS.put("else", TokenType.ELSE);
    KEYWORDS.put("false", TokenType.FALSE);
    KEYWORDS.put("for", TokenType.FOR);
    KEYWORDS.put("fun", TokenType.FUN);
    KEYWORDS.put("if", TokenType.IF);
    KEYWORDS.put("nil", TokenType.NIL);
    KEYWORDS.put("or", TokenType.OR);
    KEYWORDS.put("print", TokenType.PRINT);
    KEYWORDS.put("return", TokenType.RETURN);
    KEYWORDS.put("super", TokenType.SUPER);
    KEYWORDS.put("this", TokenType.THIS);
    KEYWORDS.put("true", TokenType.TRUE);
    KEYWORDS.put("var", TokenType.VAR);
    KEYWORDS.put("while", TokenType.WHILE);
  }

  /**
   * The start position of the current lexeme.
   */
  private int start = 0;

  /**
   * The current position of the current lexeme.
   */
  private int current = 0;

  /**
   * The current line number.
   */
  private int line = 1;

  /**
   * Construct a new Scanner instance.
   * @param source The source to be scanned
   */
  Scanner(final String source) {
    this.source = source;
  }

  /**
   * Scan all tokens from the provided source input.
   * @return The collection of tokens
   */
  List<Token> scanTokens() {
    while (!isAtEnd()) {
      // We are the beginning of the next lexeme
      start = current;
      scanToken();
    }

    tokens.add(new Token(TokenType.EOF, "", null, line));
    return tokens;
  }

  /**
   * Determine if the end of input has been reached.
   * @return `true` if at EOF, `false` otherwise
   */
  private boolean isAtEnd() {
    return current >= source.length();
  }

  private void scanToken() {
    final char c = advance();
    switch (c) {
      // Single character lexemes
      case '(': addToken(TokenType.LEFT_PAREN); break;
      case ')': addToken(TokenType.RIGHT_PAREN); break;
      case '{': addToken(TokenType.LEFT_BRACE); break;
      case '}': addToken(TokenType.RIGHT_BRACE); break;
      case ',': addToken(TokenType.COMMA); break;
      case '.': addToken(TokenType.DOT); break;
      case '-': addToken(TokenType.MINUS); break;
      case '+': addToken(TokenType.PLUS); break;
      case ';': addToken(TokenType.SEMICOLON); break;
      case '*': addToken(TokenType.STAR); break;
      
      // Single or double character lexemes
      case '!':
        addToken(match('=') ? TokenType.BANG_EQUAL : TokenType.BANG);
        break;
      case '=':
        addToken(match('=') ? TokenType.EQUAL_EQUAL : TokenType.EQUAL);
        break;
      case '<':
        addToken(match('=') ? TokenType.LESS_EQUAL : TokenType.LESS);
        break;
      case '>':
        addToken(match('=') ? TokenType.GREATER_EQUAL : TokenType.GREATER);
        break;
      
        // Special case for '/' because of comments
      case '/':
        if (match('/')) {
          // Comment goes until the end of the current line
          while (peek() != '\n' && !isAtEnd()) advance();
        } else {
          addToken(TokenType.SLASH);
        }
        break;
      
      // Ignore whitespace
      case ' ':
      case '\r':
      case '\t':
        break;
      
      // Handle line breaks
      case '\n':
        line++;
        break;
      
      // String literals
      case '"': string(); break;

      // Numeric literals and identifiers
      default:
        if (isDigit(c)) {
          number();
        } else if (isAlpha(c)) {
          identifier();
        } else {
          Lox.error(line, "Unexpected character.");
        }
        break;
    }
  }

  /**
   * Handle string literal tokens.
   */
  private void string() {
    while (peek() != '"' && !isAtEnd()) {
      if (peek() == '\n') line++;
      advance();
    }

    if (isAtEnd()) {
      Lox.error(line, "Unterminated string");
      return;
    }

    // The closing "
    advance();

    // Don't include the quotations marks themselves in the literal
    final String value = source.substring(start + 1, current - 1);
    addToken(TokenType.STRING, value);
  }

  /**
   * Handle numeric literal tokens.
   */
  private void number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part
    if (peek() == '.' && isDigit(peekNext())) {
      // Consume the '.'
      advance();
      while (isDigit(peek())) advance();
    }

    // Rely on Java Double to parse the full floating point value
    addToken(TokenType.NUMBER, Double.parseDouble(source.substring(start, current)));
  }

  /**
   * Handle identifier literals and reserved words.
   */
  private void identifier() {
    while (isAlphaNumeric(peek())) advance();
    final String text = source.substring(start, current);
    final TokenType type = KEYWORDS.containsKey(text) ? KEYWORDS.get(text) : TokenType.IDENTIFER;
    addToken(type);
  }

  /**
   * Return the next character without consuming it (lookahead).
   * @return The next character in the source
   */
  private char peek() {
    if (isAtEnd()) return '\0';
    return source.charAt(current);
  }

  /**
   * Return the character after the next character (double-lookahead).
   * @return The character after the next character in the source
   */
  private char peekNext() {
    if (current + 1 >= source.length()) return '\0';
    return source.charAt(current + 1);
  }

  /**
   * Determine if the next character matches `expected`, consuming this character if so.
   * @param expected The expected character
   * @return `true` if the characters match, `false` otherwise
   */
  private boolean match(final char expected) {
    if (isAtEnd()) {
      return false;
    }
    if (source.charAt(current) != expected) {
      return false;
    }
    current++;
    return true;
  }

  /**
   * Advance the scanner and return the current character.
   * @return The current character
   */
  private char advance() {
    return source.charAt(current++);
  }

  /**
   * Determine if the given character is a decimal digit.
   * @param c The character
   * @return `true` if `c` is decimal digit, `false` otherwise
   */
  private boolean isDigit(final char c) {
    return c >= '0' && c <= '9';
  }

  /**
   * Determine if the given character is alphabetical.
   * @param c The character
   * @return `true` if `c` is alphanumeric, `false` otherwise
   */
  private boolean isAlpha(final char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
  }

  /**
   * Determine if the given character is alphanumeric.
   * @param c The character
   * @return `true` if `c` is alphanumeric, `false` otherwise
   */
  private boolean isAlphaNumeric(final char c) {
    return isAlpha(c) || isDigit(c);
  }

  /**
   * Add a token to the list of tokens.
   * @param type The token type
   */
  private void addToken(final TokenType type) {
    addToken(type, null);
  }

  /**
   * Add a token to the list of tokens.
   * @param type The token type
   * @param literal The associated literal
   */
  private void addToken(final TokenType type, final Object literal) {
    final String text = source.substring(start, current);
    tokens.add(new Token(type, text, literal, line));
  }
}