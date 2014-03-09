//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// author:  Vassil Vassilev <vvasilev@cern.ch>
//
// This file is dual-licensed: you can choose to license it under the University
// of Illinois Open Source License or the GNU Lesser General Public License. See
// LICENSE.TXT for details.
//------------------------------------------------------------------------------

#include "MetaSema.h"

#include "Display.h"

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Value.h"
#include "cling/MetaProcessor/MetaProcessor.h"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
#include "clang/Serialization/ASTReader.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"


#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/SourceManager.h"

#include <cstdlib>
#include <iostream>

namespace cling {

  MetaSema::MetaSema(Interpreter& interp, MetaProcessor& meta)
    : m_Interpreter(interp), m_MetaProcessor(meta), m_IsQuitRequested(false) { }

  MetaSema::ActionResult MetaSema::actOnLCommand(llvm::StringRef file) {

    // Lookup the file
    clang::SourceManager& SM = m_Interpreter.getSema().getSourceManager();
    clang::FileManager& FM = SM.getFileManager();
    bool needsRegistration = false;
    const clang::FileEntry* Entry
      = FM.getFile(file, /*OpenFile*/false, /*CacheFailure*/false);
    if (Entry) {
      Watermarks::iterator Pos = m_Watermarks.find(Entry);
      if (Pos != m_Watermarks.end()) {
        const Transaction* unloadPoint = Pos->second;
        while(m_Interpreter.getLastTransaction() != unloadPoint)
          m_Interpreter.unload(/*numberOfTransactions*/1);
        // Unload the last one also.
        m_Interpreter.unload(/*numberOfTransactions*/1);
        m_Watermarks.erase(Pos);
      }
      else
        needsRegistration = true;
    }

    // TODO: extra checks. Eg if the path is readable, if the file exists...
    if (m_Interpreter.loadFile(file.str()) == Interpreter::kSuccess) {
      if (needsRegistration)
        m_Watermarks[Entry] = m_Interpreter.getLastTransaction();
      return AR_Success;
    }
    return AR_Failure;
  }

  MetaSema::ActionResult MetaSema::actOnRedirectCommand(llvm::StringRef file,
                         MetaProcessor::RedirectionScope stream,
                         bool append) {

    m_MetaProcessor.setStdStream(file, stream, append);
    return AR_Success;
  }

  void MetaSema::actOnComment(llvm::StringRef comment) const {
    // Some of the comments are meaningful for the cling::Interpreter
    m_Interpreter.declare(comment);
  }

  MetaSema::ActionResult MetaSema::actOnxCommand(llvm::StringRef file,
                                                 llvm::StringRef args,
                                                 Value* result) {
    // Fall back to the meta processor for now.
    Interpreter::CompilationResult compRes = Interpreter::kFailure;
    m_MetaProcessor.executeFile(file.str(), args.str(), compRes, result);
    ActionResult actionResult = AR_Failure;
    if (compRes == Interpreter::kSuccess)
       actionResult = AR_Success;
    return actionResult;

    //m_Interpreter.loadFile(path.str());
    // TODO: extra checks. Eg if the path is readable, if the file exists...
  }

  void MetaSema::actOnqCommand() {
    m_IsQuitRequested = true;
  }

  MetaSema::ActionResult MetaSema::actOnUCommand(unsigned value /*=1*/) const {
     // FIXME: unload, once implemented, must return success / failure
     m_Interpreter.unload(value);
     return AR_Success;
  }

  void MetaSema::actOnICommand(llvm::StringRef path) const {
    if (path.empty())
      m_Interpreter.DumpIncludePath();
    else
      m_Interpreter.AddIncludePath(path.str());
  }

  void MetaSema::actOnrawInputCommand(SwitchMode mode/* = kToggle*/) const {
    if (mode == kToggle) {
      bool flag = !m_Interpreter.isRawInputEnabled();
      m_Interpreter.enableRawInput(flag);
      // FIXME:
      m_MetaProcessor.getOuts() << (flag ? "U" :"Not u") << "sing raw input\n";
    }
    else
      m_Interpreter.enableRawInput(mode);
  }

  void MetaSema::actOnprintDebugCommand(SwitchMode mode/* = kToggle*/) const {
    if (mode == kToggle) {
      bool flag = !m_Interpreter.isPrintingDebug();
      m_Interpreter.enablePrintDebug(flag);
      // FIXME:
      m_MetaProcessor.getOuts() << (flag ? "P" : "Not p") << "rinting Debug\n";
    }
    else
      m_Interpreter.enablePrintDebug(mode);
  }

  void MetaSema::actOnstoreStateCommand(llvm::StringRef name) const {
    m_Interpreter.storeInterpreterState(name);
  }

  void MetaSema::actOncompareStateCommand(llvm::StringRef name) const {
    m_Interpreter.compareInterpreterState(name);
  }

  void MetaSema::actOnstatsCommand(llvm::StringRef name) const {
    if (name.equals("ast")) {
      m_Interpreter.getCI()->getSema().getASTContext().PrintStats();
    }
  }

  void MetaSema::actOndynamicExtensionsCommand(SwitchMode mode/* = kToggle*/)
    const {
    if (mode == kToggle) {
      bool flag = !m_Interpreter.isDynamicLookupEnabled();
      m_Interpreter.enableDynamicLookup(flag);
      // FIXME:
      m_MetaProcessor.getOuts()
        << (flag ? "U" : "Not u") << "sing dynamic extensions\n";
    }
    else
      m_Interpreter.enableDynamicLookup(mode);
  }

  void MetaSema::actOnhelpCommand() const {
    std::string& metaString = m_Interpreter.getOptions().MetaString;
    llvm::raw_ostream& outs = m_MetaProcessor.getOuts();
    outs << "Cling meta commands usage\n"
      "Syntax: .Command [arg0 arg1 ... argN]\n"
      "\n"
         << metaString << "q\t\t\t\t- Exit the program\n"
         << metaString << "L <filename>\t\t\t - Load file or library\n"
         << metaString << "(x|X) <filename>[args]\t\t- Same as .L and runs a "
      "function with signature "
      "\t\t\t\tret_type filename(args)\n"
         << metaString <<"I [path]\t\t\t- Shows the include path. If a path is "
      "given - \n\t\t\t\tadds the path to the include paths\n"
         << metaString <<"@ \t\t\t\t- Cancels and ignores the multiline input\n"
         << metaString << "rawInput [0|1]\t\t\t- Toggle wrapping and printing "
      "the execution\n\t\t\t\tresults of the input\n"
         << metaString << "dynamicExtensions [0|1]\t- Toggles the use of the "
      "dynamic scopes and the \t\t\t\tlate binding\n"
         << metaString << "printDebug [0|1]\t\t\t- Toggles the printing of "
      "input's corresponding \t\t\t\tstate changes\n"
         << metaString << "help\t\t\t\t- Shows this information\n";
  }

  void MetaSema::actOnfileExCommand() const {
    const clang::SourceManager& SM = m_Interpreter.getCI()->getSourceManager();
    SM.getFileManager().PrintStats();

    m_MetaProcessor.getOuts() << "\n***\n\n";

    for (clang::SourceManager::fileinfo_iterator I = SM.fileinfo_begin(),
           E = SM.fileinfo_end(); I != E; ++I) {
      m_MetaProcessor.getOuts() << (*I).first->getName();
      m_MetaProcessor.getOuts() << "\n";
    }
    /* Only available in clang's trunk:
    clang::ASTReader* Reader = m_Interpreter.getCI()->getModuleManager();
    const clang::serialization::ModuleManager& ModMan
      = Reader->getModuleManager();
    for (clang::serialization::ModuleManager::ModuleConstIterator I
           = ModMan.begin(), E = ModMan.end(); I != E; ++I) {
      typedef
        std::vector<llvm::PointerIntPair<const clang::FileEntry*, 1, bool> >
        InputFiles_t;
      const InputFiles_t& InputFiles = (*I)->InputFilesLoaded;
      for (InputFiles_t::const_iterator IFI = InputFiles.begin(),
             IFE = InputFiles.end(); IFI != IFE; ++IFI) {
        m_MetaProcessor.getOuts() << IFI->getPointer()->getName();
        m_MetaProcessor.getOuts() << "\n";
      }
    }
    */
  }

  void MetaSema::actOnfilesCommand() const {
    m_Interpreter.printIncludedFiles(m_MetaProcessor.getOuts());
  }

  void MetaSema::actOnclassCommand(llvm::StringRef className) const {
    if (!className.empty())
      DisplayClass(m_MetaProcessor.getOuts(),
                   &m_Interpreter, className.str().c_str(), true);
    else
      DisplayClasses(m_MetaProcessor.getOuts(), &m_Interpreter, false);
  }

  void MetaSema::actOnClassCommand() const {
    DisplayClasses(m_MetaProcessor.getOuts(), &m_Interpreter, true);
  }

  void MetaSema::actOngCommand(llvm::StringRef varName) const {
    if (varName.empty())
      DisplayGlobals(m_MetaProcessor.getOuts(), &m_Interpreter);
    else
      DisplayGlobal(m_MetaProcessor.getOuts(),
                    &m_Interpreter, varName.str().c_str());
  }

  void MetaSema::actOnTypedefCommand(llvm::StringRef typedefName) const {
    if (typedefName.empty())
      DisplayTypedefs(m_MetaProcessor.getOuts(), &m_Interpreter);
    else
      DisplayTypedef(m_MetaProcessor.getOuts(),
                     &m_Interpreter, typedefName.str().c_str());
  }

  MetaSema::ActionResult
  MetaSema::actOnShellCommand(llvm::StringRef commandLine,
                              Value* result) const {
    llvm::StringRef trimmed(commandLine.trim(" \t\n\v\f\r "));
    if (!trimmed.empty()) {
      int ret = std::system(trimmed.str().c_str());

      // Build the result
      clang::ASTContext& Ctx = m_Interpreter.getCI()->getASTContext();
      if (result) {
        *result = Value(Ctx.IntTy, 0);
        result->getAs<long long>() = ret;
      }

      return (ret == 0) ? AR_Success : AR_Failure;
    }
    if (result)
      *result = Value();
    // nothing to run - should this be success or failure?
    return AR_Failure;
  }
} // end namespace cling
