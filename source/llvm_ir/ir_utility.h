//
// Created by foxfurry on 6/1/21.
//

#ifndef LEXER_TEST_IR_UTILITY_H
#define LEXER_TEST_IR_UTILITY_H

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

#include "../ast/ast_node.h"

#define JIT_MODULE_NAME "PBLANG JIT MODULE"

inline std::unique_ptr<llvm::LLVMContext> llvm_context;
inline std::unique_ptr<llvm::Module> llvm_module;
inline std::unique_ptr<llvm::IRBuilder<>> llvm_builder;
inline std::map<std::string, llvm::AllocaInst *> named_values;
inline std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;

#endif //LEXER_TEST_IR_UTILITY_H
