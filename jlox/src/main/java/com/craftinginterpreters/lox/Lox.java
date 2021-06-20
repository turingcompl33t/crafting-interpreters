/**
 * Lox.java
 */

package com.craftinginterpreters.lox;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

import com.craftinginterpreters.lox.ast.*;

/**
 * A dummy logger class that just writes to standard output.
 */
class Logger {
  /**
   * Log an error message.
   * @param message
   */
  static void error(final String message) {
    System.out.println(message);
  }
};

/**
 * The Lox class is the top-level entry point to the Lox interpreter.
 */
public class Lox {
  /**
   * The Lox REPL prompt.
   */
  private static final String PROMPT = "> ";

  /**
   * Denotes that an error occurred during program loading (scanning / parsing).
   */
  private static boolean hadError = false;

  /**
   * Denotes than an error occurred during program execution.
   */
  private static boolean hadRuntimeError = false;

  /**
   * The interpreter instance on which programs are executed.
   */
  private static final Interpreter interpreter = new Interpreter();

  /**
   * Static class cannot be instantiated.
   */
  private Lox() {

  }

  // --------------------------------------------------------------------------
  // Interpreter Entry Point
  // --------------------------------------------------------------------------

  /**
   * Lox interpreter entry point.
   * @param args
   * @throws IOException
   */
  public static void main(String[] args) throws IOException {
    if (args.length > 1) {
      Logger.error("Usage: jlox [script]");
      System.exit(ExitCodes.EX_USAGE);
    }
    else if (args.length == 1) {
      runFile(args[0]);
    } else {
      runPrompt();
    }
  }

  // --------------------------------------------------------------------------
  // Top-Level Interpreter Execution
  // --------------------------------------------------------------------------

  /**
   * Run the Lox source file at the specified path on the interpreter.
   * @param path The path to the Lox source file
   * @throws IOException
   */
  public static void runFile(final String path) throws IOException {
    byte[] bytes = Files.readAllBytes(Paths.get(path));
    run(new String(bytes, Charset.defaultCharset()));
    if (hadError) {
      System.exit(ExitCodes.EX_DATAERR);
    }
    if (hadRuntimeError) {
      System.exit(ExitCodes.EX_SOFTWARE);
    }
  }

  /**
   * Run the Lox interpreter interactively.
   * @throws IOException
   */
  public static void runPrompt() throws IOException {
    InputStreamReader input = new InputStreamReader(System.in);
    BufferedReader reader = new BufferedReader(input);
    for (;;) {
      System.out.print(PROMPT);
      final String program = reader.readLine();
      if (program == null) {
        break;
      }
      run(program);
      hadError = false;
    }
  }

  /**
   * Run the input Lox program on the interpreter.
   * @param source The input program source
   */
  private static void run(final String source) {
    // Scan the input source
    Scanner scanner = new Scanner(source);
    List<Token> tokens = scanner.scanTokens();

    // Parse the scanned tokens to the Lox AST
    Parser parser = new Parser(tokens);
    final Expr expression = parser.parse();

    if (hadError) {
      // Syntax error during parsing
      return;
    }

    // Interpret the program
    interpreter.interpret(expression);
  }

  // --------------------------------------------------------------------------
  // Error Reporting
  // --------------------------------------------------------------------------

  /**
   * Report an static error.
   * @param line The line number at which the error occurred
   * @param message The error message
   */
  public static void error(final int line, final String message) {
    report(line, "", message);
  }

  /**
   * Report a (static) parse error.
   * @param token The token at which the error occurred
   * @param message The error message to display
   */
  public static void error(final Token token, final String message) {
    if (token.getType() == TokenType.EOF) {
      report(token.getLine(), " at end", message);
    } else {
      report(token.getLine(), " at '" + token.getLexeme() + "'", message);
    }
  }

  /**
   * Report a runtime error.
   * @param error The error
   */
  public static void runtimeError(final RuntimeError error) {
    StringBuilder builder = new StringBuilder();
    builder.append(error.getMessage());
    builder.append("\n[line ");
    builder.append(error.token.getLine());
    builder.append("]");
    System.err.println(builder.toString());
    hadRuntimeError = true;
  }

  /**
   * Report an error.
   * @param line The line number at which the error occurred
   * @param where The error location
   * @param message The error message
   */
  private static void report(final int line, final String where, final String message) {
    StringBuilder builder = new StringBuilder();
    builder.append("[line ");
    builder.append(line);
    builder.append("] Error");
    builder.append(where);
    builder.append(": ");
    builder.append(message);
    System.err.println(builder.toString());
    hadError = true;
  }
}
