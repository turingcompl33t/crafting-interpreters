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
   * Denotes that an error occurred during program execution.
   */
  private static boolean hadError = false;

  /**
   * Static class cannot be instantiated.
   */
  private Lox() {

  }

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
    Scanner scanner = new Scanner(source);
    List<Token> tokens = scanner.scanTokens();

    // For now just print tokens
    for (final Token token : tokens) {
      System.out.println(token);
    }
  }

  public static void error(final int line, final String message) {
    report(line, "", message);
  }

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
