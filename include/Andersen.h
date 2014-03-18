//
// This file defines an implementation of Andersen's interprocedural alias
// analysis
//
// In pointer analysis terms, this is a subset-based, flow-insensitive,
// field-sensitive, and context-insensitive algorithm pointer algorithm.
//
// This algorithm is implemented as three stages:
//   1. Object identification.
//   2. Inclusion constraint identification.
//   3. Offline constraint graph optimization
//   4. Inclusion constraint solving.
//
// The object identification stage identifies all of the memory objects in the
// program, which includes globals, heap allocated objects, and stack allocated
// objects.
//
// The inclusion constraint identification stage finds all inclusion constraints
// in the program by scanning the program, looking for pointer assignments and
// other statements that effect the points-to graph.  For a statement like "A =
// B", this statement is processed to indicate that A can point to anything that
// B can point to.  Constraints can handle copies, loads, and stores, and
// address taking.
//
// The offline constraint graph optimization portion includes offline variable
// substitution algorithms intended to compute pointer and location
// equivalences.  Pointer equivalences are those pointers that will have the
// same points-to sets, and location equivalences are those variables that
// always appear together in points-to sets.  It also includes an offline
// cycle detection algorithm that allows cycles to be collapsed sooner
// during solving.
//
// The inclusion constraint solving phase iteratively propagates the inclusion
// constraints until a fixed point is reached.  This is an O(N^3) algorithm.
//
// Function constraints are handled as if they were structs with X fields.
// Thus, an access to argument X of function Y is an access to node index
// getNode(Y) + X.  This representation allows handling of indirect calls
// without any issues.  To wit, an indirect call Y(a,b) is equivalent to
// *(Y + 1) = a, *(Y + 2) = b.
// The return node for a function is always located at getNode(F) +
// CallReturnPos. The arguments start at getNode(F) + CallArgPos.
//

#ifndef TCFS_ANDERSEN_H
#define TCFS_ANDERSEN_H

#include "Constraint.h"
#include "NodeFactory.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetLibraryInfo.h"

#include <vector>

class Andersen: public llvm::ModulePass
{
private:
	llvm::TargetLibraryInfo* tli;

	// Constants that will become useful
	static const unsigned SelfRep = (unsigned)-1;
	static const unsigned Unvisited = (unsigned)-1;
	// Position of the function return node relative to the function node.
	static const unsigned CallReturnPos = 1;
	// Position of the function call node relative to the function node.
	static const unsigned CallFirstArgPos = 2;

	AndersNodeFactory nodeFactory;

	/// Constraints - This vector contains a list of all of the constraints
	/// identified by the program.
	std::vector<AndersConstraint> constraints;

	void identifyObjects(llvm::Module&);
	void collectConstraints(llvm::Module&);
	void solveConstraints();
public:
	static char ID;

	Andersen(): llvm::ModulePass(ID) {}
	bool runOnModule(llvm::Module& M);
	void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
	void releaseMemory();
};


#endif