/**
 * Resolver.java
 */

package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import com.craftinginterpreters.lox.ast.*;

/**
 * The Resolver class implements a variable resolution pass.
 */
public class Resolver implements ExprVisitor<Void>, StmtVisitor<Void> {
  /**
   * The FunctionType enumeration defines function types.
   */
  private enum FunctionType {
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD
  }

  /**
   * The ClassType enumeration defines class types.
   */
  private enum ClassType {
    NONE,
    CLASS,
    SUBCLASS
  }

  /**
   * The associated interpreter instance.
   */
  private final Interpreter interpreter;

  /**
   * The stack of scopes.
   */
  private final Stack<Map<String, Boolean>> scopes = new Stack<>();

  /**
   * Denotes the current function, if any. 
   */
  private FunctionType currentFunction = FunctionType.NONE;

  /**
   * Denotes the current class, if any.
   */
  private ClassType currentClass = ClassType.NONE;

  /**
   * Construct a new Resolver instance.
   * @param interpreter The associated interpreter
   */
  public Resolver(final Interpreter interpreter) {
    this.interpreter = interpreter;
  }

  // --------------------------------------------------------------------------
  // Interesting Resolution Visitors
  // --------------------------------------------------------------------------

  /**
   * Visit a block statement.
   * @param stmt The statement
   */
  @Override
  public Void visitBlockStmt(final BlockStmt stmt) {
    beginScope();
    resolve(stmt.statements);
    endScope();
    return null;
  }

  /**
   * Visit a variable statement
   * @param stmt The statement
   */
  @Override
  public Void visitVarStmt(final VarStmt stmt) {
    declare(stmt.name);
    if (stmt.initializer != null) {
      resolve(stmt.initializer);
    }
    define(stmt.name);
    return null;
  }

  /**
   * Visit a variable expression.
   * @param expr The expression
   */
  @Override
  public Void visitVariableExpr(final VariableExpr expr) {
    if (!scopes.isEmpty()
      && scopes.peek().get(expr.name.getLexeme()) == Boolean.FALSE) {
      Lox.error(expr.name, "Can't read local variable in its own initializer.");
    }

    resolveLocal(expr, expr.name);
    return null;
  }

  /**
   * Visit an assignment expression.
   * @param expr The expression
   */
  @Override
  public Void visitAssignExpr(final AssignExpr expr) {
    resolve(expr.value);
    resolveLocal(expr, expr.name);
    return null;
  }

  /**
   * Visit a function statement.
   * @param stmt The statement
   */
  @Override
  public Void visitFunctionStmt(final FunctionStmt stmt) {
    declare(stmt.name);
    define(stmt.name);

    resolveFunction(stmt, FunctionType.FUNCTION);
    return null;
  }

  /**
   * Visit a return statement.
   * @param stmt The statement
   */
  @Override
  public Void visitReturnStmt(final ReturnStmt stmt) {
    if (currentFunction == FunctionType.NONE) {
      Lox.error(stmt.keyword, "Can't return from top-level code.");
    }
    
    if (stmt.value != null) {
      if (currentFunction == FunctionType.INITIALIZER) {
        Lox.error(stmt.keyword, "Can't return a value from a class initializer.");
      }
      resolve(stmt.value);
    }
    return null;
  }

  /**
   * Visit a this expression.
   * @param expr The expression
   */
  @Override
  public Void visitThisExpr(final ThisExpr expr) {
    if (currentClass == ClassType.NONE) {
      Lox.error(expr.keyword, "Can't use 'this' outside of a class.");
    }

    resolveLocal(expr, expr.keyword);
    return null;
  }

  // --------------------------------------------------------------------------
  // Uninteresting Resolution Visitors
  // --------------------------------------------------------------------------

  /**
   * Visit an expression statement.
   * @param stmt The statement
   */
  @Override
  public Void visitExpressionStmt(final ExpressionStmt stmt) {
    resolve(stmt.expression);
    return null;
  }

  /**
   * Visit an if statement.
   * @param stmt The statement
   */
  @Override
  public Void visitIfStmt(final IfStmt stmt) {
    resolve(stmt.condition);
    resolve(stmt.thenBranch);
    if (stmt.elseBranch != null) resolve(stmt.elseBranch);
    return null;
  }

  /**
   * Visit a print statement.
   * @param stmt The statement
   */
  @Override
  public Void visitPrintStmt(final PrintStmt stmt) {
    resolve(stmt.expression);
    return null;
  }

  /**
   * Visit a while statement.
   * @param stmt The statement
   */
  @Override
  public Void visitWhileStmt(final WhileStmt stmt) {
    resolve(stmt.condition);
    resolve(stmt.body);
    return null;
  }

  /**
   * Visit a binary expression.
   * @param expr The expression
   */
  @Override
  public Void visitBinaryExpr(final BinaryExpr expr) {
    resolve(expr.left);
    resolve(expr.right);
    return null;
  }

  /**
   * Visit a call expression.
   * @param expr The expression
   */
  @Override
  public Void visitCallExpr(final CallExpr expr) {
    resolve(expr.callee);
    for (final Expr argument : expr.arguments) {
      resolve(argument);
    }
    return null;
  }

  /**
   * Visit a get expression.
   * @param expr The expression
   */
  @Override
  public Void visitGetExpr(final GetExpr expr) {
    resolve(expr.object);
    return null;
  }

  /**
   * Visit a set expression.
   * @param expr The expression
   */
  @Override
  public Void visitSetExpr(final SetExpr expr) {
    resolve(expr.value);
    resolve(expr.object);
    return null;
  }

  /**
   * Visit a super expression.
   * @param expr The expression
   */
  @Override
  public Void visitSuperExpr(final SuperExpr expr) {
    if (currentClass == ClassType.NONE) {
      Lox.error(expr.keyword, "Can't use 'super' outside of a class.");
    } else if (currentClass != ClassType.SUBCLASS) {
      Lox.error(expr.keyword, "Can't use 'super' in a class with no superclass.");
    }

    resolveLocal(expr, expr.keyword);
    return null;
  }

  /**
   * Visit a grouping expression.
   * @param expr The expression
   */
  @Override
  public Void visitGroupingExpr(final GroupingExpr expr) {
    resolve(expr.expression);
    return null;
  }

  /**
   * Visit a literal expression.
   * @param expr The expression
   */
  @Override
  public Void visitLiteralExpr(final LiteralExpr expr) {
    return null;
  }

  /**
   * Visit a logical expression.
   * @param expr The expression
   */
  @Override
  public Void visitLogicalExpr(final LogicalExpr expr) {
    resolve(expr.left);
    resolve(expr.right);
    return null;
  }

  /**
   * Visit a unary expression.
   * @param expr The expression
   */
  @Override
  public Void visitUnaryExpr(final UnaryExpr expr) {
    resolve(expr.expression);
    return null;
  }

  /**
   * Visit a class statement.
   * @param stmt The statement
   */
  @Override
  public Void visitClassStmt(final ClassStmt stmt) {
    final ClassType enclosingClass = currentClass;
    currentClass = ClassType.CLASS;
    
    declare(stmt.name);
    define(stmt.name);

    if (stmt.superclass != null &&
      stmt.name.getLexeme().equals(stmt.superclass.name.getLexeme())) {
      Lox.error(stmt.superclass.name, "A class cannot inherit from itself.");
    }

    // If the class defines a superclass, resolve it
    if (stmt.superclass != null) {
      currentClass = ClassType.SUBCLASS;
      resolve(stmt.superclass);
    }

    // If the class declaration has a superclass,
    // create a new scope surrounding all of its methods
    if (stmt.superclass != null) {
      beginScope();
      scopes.peek().put("super", true);
    }

    // `this` is implicitly defined in the scope for each method
    beginScope();
    scopes.peek().put("this", true);

    for (final FunctionStmt method : stmt.body) {
      // Initializers require special attention; we don't want
      // to allow users to return arbitrary values from initializers
      final FunctionType declaration = method.name.getLexeme().equals("init") 
        ? FunctionType.INITIALIZER 
        : FunctionType.METHOD;
      resolveFunction(method, declaration);
    }

    endScope();

    // If we created a scope for the superclass methods, end it
    if (stmt.superclass != null) endScope();

    currentClass = enclosingClass;
    return null;
  }

  // --------------------------------------------------------------------------
  // Helpers
  // --------------------------------------------------------------------------

  /**
   * Resolve a local variable.
   * @param expr The variable expression
   * @param name The variable identifier
   */
  private void resolveLocal(final Expr expr, final Token name) {
    for (int i = scopes.size() - 1; i >= 0; --i) {
      if (scopes.get(i).containsKey(name.getLexeme())) {
        interpreter.resolve(expr, scopes.size() - 1 - i);
        return;
      }
    }
  }

  /**
   * Resolve a function.
   * @param function The function
   * @param type The current function type
   */
  private void resolveFunction(final FunctionStmt function, final FunctionType type) {
    final FunctionType enclosingFunction = currentFunction;
    currentFunction = type;
    
    beginScope();
    for (final Token param : function.params) {
      declare(param);
      define(param);
    }
    resolve(function.body);
    endScope();

    currentFunction = enclosingFunction;
  }

  /**
   * Declare a variable.
   * @param name The variable identifier
   */
  private void declare(final Token name) {
    if (scopes.isEmpty()) return;
    final Map<String, Boolean> scope = scopes.peek();
    if (scope.containsKey(name.getLexeme())) {
      Lox.error(name, "Already a variable with this name in this scope.");
    }
    scope.put(name.getLexeme(), false);
  }

  /**
   * Define a variable.
   * @param name The variable identifier
   */
  private void define(final Token name) {
    if (scopes.isEmpty()) return;
    scopes.peek().put(name.getLexeme(), true);
  }

  /**
   * Enter a new scope.
   */
  private void beginScope() {
    scopes.push(new HashMap<String, Boolean>());
  }

  /**
   * Leave the current scope.
   */
  private void endScope() {
    scopes.pop();
  }

  /**
   * Resolve a collection of statements.
   * @param statements The collection of statements
   */
  void resolve(final List<Stmt> statements) {
    for (final Stmt statement : statements) {
      resolve(statement);
    }
  }

  /**
   * Resolve a statement.
   * @param statement The statement
   */
  void resolve(final Stmt statement) {
    statement.accept(this);
  }

  /**
   * Resolve an expression.
   * @param expression The expression
   */
  void resolve (final Expr expression) {
    expression.accept(this);
  }
}