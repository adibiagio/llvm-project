//===- unittest/Tooling/RecursiveASTVisitorTests/LambdaExpr.cpp -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TestVisitor.h"
#include "llvm/TargetParser/Host.h"
#include <stack>

using namespace clang;

namespace {

class LambdaExprVisitor : public ExpectedLocationVisitor {
public:
  LambdaExprVisitor() { ShouldVisitImplicitCode = false; }

  bool VisitLambdaExpr(LambdaExpr *Lambda) override {
    PendingBodies.push(Lambda->getBody());
    PendingClasses.push(Lambda->getLambdaClass());
    Match("", Lambda->getIntroducerRange().getBegin());
    return true;
  }
  /// For each call to VisitLambdaExpr, we expect a subsequent call to visit
  /// the body (and maybe the lambda class, which is implicit).
  bool VisitStmt(Stmt *S) override {
    if (!PendingBodies.empty() && S == PendingBodies.top())
      PendingBodies.pop();
    return true;
  }
  bool VisitDecl(Decl *D) override {
    if (!PendingClasses.empty() && D == PendingClasses.top())
      PendingClasses.pop();
    return true;
  }
  /// Determine whether parts of lambdas (VisitLambdaExpr) were later traversed.
  bool allBodiesHaveBeenTraversed() const { return PendingBodies.empty(); }
  bool allClassesHaveBeenTraversed() const { return PendingClasses.empty(); }

private:
  std::stack<Stmt *> PendingBodies;
  std::stack<Decl *> PendingClasses;
};

TEST(RecursiveASTVisitor, VisitsLambdaExpr) {
  LambdaExprVisitor Visitor;
  Visitor.ExpectMatch("", 1, 12);
  EXPECT_TRUE(Visitor.runOver("void f() { []{ return; }(); }",
                              LambdaExprVisitor::Lang_CXX11));
  EXPECT_TRUE(Visitor.allBodiesHaveBeenTraversed());
  EXPECT_FALSE(Visitor.allClassesHaveBeenTraversed());
}

TEST(RecursiveASTVisitor, LambdaInLambda) {
  LambdaExprVisitor Visitor;
  Visitor.ExpectMatch("", 1, 12);
  Visitor.ExpectMatch("", 1, 16);
  EXPECT_TRUE(Visitor.runOver("void f() { []{ []{ return; }; }(); }",
                              LambdaExprVisitor::Lang_CXX11));
  EXPECT_TRUE(Visitor.allBodiesHaveBeenTraversed());
  EXPECT_FALSE(Visitor.allClassesHaveBeenTraversed());
}

TEST(RecursiveASTVisitor, TopLevelLambda) {
  LambdaExprVisitor Visitor;
  Visitor.ShouldVisitImplicitCode = true;
  Visitor.ExpectMatch("", 1, 10);
  Visitor.ExpectMatch("", 1, 14);
  EXPECT_TRUE(Visitor.runOver("auto x = []{ [] {}; };",
                              LambdaExprVisitor::Lang_CXX11));
  EXPECT_TRUE(Visitor.allBodiesHaveBeenTraversed());
  EXPECT_TRUE(Visitor.allClassesHaveBeenTraversed());
}

TEST(RecursiveASTVisitor, VisitsLambdaExprAndImplicitClass) {
  LambdaExprVisitor Visitor;
  Visitor.ShouldVisitImplicitCode = true;
  Visitor.ExpectMatch("", 1, 12);
  EXPECT_TRUE(Visitor.runOver("void f() { []{ return; }(); }",
                              LambdaExprVisitor::Lang_CXX11));
  EXPECT_TRUE(Visitor.allBodiesHaveBeenTraversed());
  EXPECT_TRUE(Visitor.allClassesHaveBeenTraversed());
}

TEST(RecursiveASTVisitor, VisitsAttributedLambdaExpr) {
  if (llvm::Triple(llvm::sys::getDefaultTargetTriple()).isPS())
    GTEST_SKIP(); // PS4/PS5 do not support fastcall.
  LambdaExprVisitor Visitor;
  Visitor.ExpectMatch("", 1, 12);
  EXPECT_TRUE(Visitor.runOver(
      "void f() { [] () __attribute__ (( fastcall )) { return; }(); }",
      LambdaExprVisitor::Lang_CXX14));
}

} // end anonymous namespace
