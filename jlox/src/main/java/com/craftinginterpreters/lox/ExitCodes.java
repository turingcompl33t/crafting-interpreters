/**
 * ExitCodes.java
 */

package com.craftinginterpreters.lox;


/**
 * The ExitCodes class wraps some system exit codes from the UNIX sysexits.h header. 
 */
public class ExitCodes {
  /**
   * Static class cannot be instantiated.
   */
  private ExitCodes() {

  }

  /**
   * The command was used incorrectly.
   */
  public static final int EX_USAGE = 64;

  /**
   * The input data was incorrect in some way.
   */
  public static final int EX_DATAERR = 65;

  /**
   * An internal software error has been detected.
   */
  public static final int EX_SOFTWARE = 70;
}