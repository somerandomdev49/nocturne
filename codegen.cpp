#include "codegen.hpp"
#include "util.hpp"
#include "fmt.hpp"
#include "log.hpp"

#include <utility>
#include <stack>
#include <memory>
#include <initializer_list>

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
#include <llvm/Support/Host.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

namespace
{
	// template<typename... Args>
	// void error(const std::string &fmt, Args &&...args);

	// void error(const std::string &fmt)
	// {
	// 	std::cerr << "\033[0;31mError: " << fmt << "\033[0;0m" << std::endl;
	// }

	// void panic(const std::string &fmt)
	// {
	// 	std::cerr << "\033[0;31mPanic!: " << fmt << "\033[0;0m" << std::endl;
	// 	std::exit(1);
	// }

} // namespace

#define PANIC(...) panic(__func__ __VA_OPT__(, ) __VA_ARGS__);

namespace noct
{
	struct Variable
	{
		Ptr<Type>   type;
		llvm::Type *llvmType;
		std::string name;
	};

	struct Global : Variable
	{
		llvm::GlobalVariable *llvmVar;

		Global(llvm::GlobalVariable *v) : llvmVar(v) {}
	};

	struct Local : Variable
	{
		llvm::Value *llvmVar;
	};

	class Env
	{
	  public:
		auto has(const std::string &name) -> bool { return !it_(name).error; }

		auto get(const std::string &name) -> Variable *
		{
			if(auto i = it_(name); !i.error)
				return i.value->second.get();
			return nullptr;
		}

		template<typename T, typename... Args>
		void set(const std::string &name, Args &&...args)
		{
			if(auto i = it_(name); i.error)
				i.value->second = std::make_unique<T>(std::forward<Args...>(args...));
		}

		bool isLoop, isFunc;

	  private:
		using MapType = std::unordered_map<std::string, std::unique_ptr<Variable>>;
		auto it_(const std::string &name) -> Result<MapType::iterator>
		{
			auto f = values.find(name);
			return {f == values.end(), f};
		}

		MapType values;
	};

	struct GeneratorImpl
	{
		std::string                   moduleName;
		llvm::LLVMContext             context;
		llvm::IRBuilder<>             builder;
		std::unique_ptr<llvm::Module> codeModule;

		std::vector<Env> baseEnv;

		bool        shouldOutputAssembly;
		std::string outputAssemblyFile;

		bool        shouldOutputObject;
		std::string outputObjectFile;

		bool        shouldOutputIR;
		std::string outputIRFile;

		GeneratorImpl(const std::string &moduleName);
		void generateFunction(ASTFunc *func);
		void generateGlobal(ASTVar *var);

		void output();

		void provideImpls(AST *ast);
	};

	struct ASTNodeImpl
	{
		virtual llvm::Value *gen(GeneratorImpl &env) const noexcept = 0;
		virtual void         provideImpls(GeneratorImpl &env) const noexcept = 0;
	};

	llvm::Type *convertNumericTypeToLLVMType(llvm::LLVMContext &ctx, NumericType t)
	{
		switch(t)
		{
		case NumericType::u8: return llvm::Type::getInt8Ty(ctx);
		case NumericType::u16: return llvm::Type::getInt16Ty(ctx);
		case NumericType::u32: return llvm::Type::getInt32Ty(ctx);
		case NumericType::u64: return llvm::Type::getInt64Ty(ctx);
		case NumericType::i8: return llvm::Type::getInt8Ty(ctx);
		case NumericType::i16: return llvm::Type::getInt16Ty(ctx);
		case NumericType::i32: return llvm::Type::getInt32Ty(ctx);
		case NumericType::i64: return llvm::Type::getInt64Ty(ctx);
		case NumericType::f32: return llvm::Type::getFloatTy(ctx);
		case NumericType::f64: return llvm::Type::getFloatTy(ctx);
		default: return nullptr;
		}
	}

	llvm::Type *convertTypeToLLVMType(GeneratorImpl &env, const Ptr<Type> &p)
	{
		if(auto t = dynamic_cast<TypeNumeric *>(p.get()); t != nullptr)
			return convertNumericTypeToLLVMType(env.context, t->numeric);

		return nullptr;
	}

	llvm::Function *generateFunctionProto(GeneratorImpl &env, FuncDeclaration &decl)
	{
		llvm::FunctionType *ft = llvm::FunctionType::get(
			convertTypeToLLVMType(env, decl.signature.returnType),
			std::vector<llvm::Type *>(), false);

		llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
		                                           decl.name, env.codeModule.get());

		return f;
	}

	struct ASTVarImpl : ASTNodeImpl
	{
		ASTVar *node;
		ASTVarImpl(ASTVar *node) : node(node) {}

		llvm::Value *gen(GeneratorImpl &env) const noexcept override
		{
			llvm::Constant *initializer = nullptr;
			if(node->value)
			{
				auto *v = node->value->impl_->gen(env);
				if(!llvm::isa<llvm::Constant>(v))
					initializer = llvm::cast<llvm::Constant>(v);
				else
				{
					error("Cannot initialize global '{0}' with a non-constant!",
					      node->decl.name);
					return nullptr;
				}
			}

			auto *g = new llvm::GlobalVariable(
				*env.codeModule, convertTypeToLLVMType(env, node->decl.type), false,
				llvm::GlobalVariable::ExternalLinkage, initializer, node->decl.name);

			env.baseEnv.front().set<Global>(node->decl.name, g);
			return g;
		}

		void provideImpls(GeneratorImpl &env) const noexcept override
		{
			if(node->value != nullptr)
				env.provideImpls(node->value.get());
		}
	};

	struct ASTFuncImpl : ASTNodeImpl
	{
		ASTFunc *node;

		ASTFuncImpl(ASTFunc *node) : node(node) {}

		llvm::Value *gen(GeneratorImpl &env) const noexcept override
		{
			llvm::Function   *f = generateFunctionProto(env, node->decl);
			llvm::BasicBlock *bb = llvm::BasicBlock::Create(env.context, "entry", f);
			env.builder.SetInsertPoint(bb);

			if(auto b = dynamic_cast<ASTBlock *>(node->body.get()); b != nullptr)
			{
				auto retValue = b->impl_->gen(env);
				env.builder.CreateRet(retValue);
				llvm::verifyFunction(*f);

				return f;
			}

			error("node->body is not an ASTBlock!");
			f->eraseFromParent();
			return nullptr;
		}

		void provideImpls(GeneratorImpl &env) const noexcept override
		{
			env.provideImpls(node->body.get());
		}
	};

	struct ASTBlockImpl : ASTNodeImpl
	{
		ASTBlock *node;

		ASTBlockImpl(ASTBlock *node) : node(node) {}

		llvm::Value *gen(GeneratorImpl &env) const noexcept override
		{
			llvm::Value *last = nullptr;
			for(const auto &n : node->nodes) { last = n->impl_->gen(env); }

			return last;
		}

		void provideImpls(GeneratorImpl &env) const noexcept override
		{
			for(const auto &n : node->nodes) { env.provideImpls(n.get()); }
		}
	};

	struct ASTIntImpl : ASTNodeImpl
	{
		ASTInt *node;

		ASTIntImpl(ASTInt *node) : node(node) {}

		llvm::Value *gen(GeneratorImpl &env) const noexcept override
		{
			auto t = getSuitableIntegerTypeFor(node->value);
			return llvm::ConstantInt::get(convertNumericTypeToLLVMType(env.context, t),
			                              node->value, isSignedNumericType(t));
		}

		void provideImpls(GeneratorImpl &env) const noexcept override {}
	};

	GeneratorImpl::GeneratorImpl(const std::string &moduleName)
		: moduleName(moduleName), builder(context)
	{
		codeModule = std::make_unique<llvm::Module>(moduleName, context);
	}

	void GeneratorImpl::generateFunction(ASTFunc *func)
	{
		provideImpls(func);
		auto f = (llvm::Function *)func->impl_->gen(*this);
	}

	void GeneratorImpl::generateGlobal(ASTVar *var)
	{
		provideImpls(var);
		auto g = (llvm::GlobalVariable *)var->impl_->gen(*this);
	}

	void GeneratorImpl::output()
	{
		// codeModule->print(llvm::errs(), nullptr);
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmParser();
		llvm::InitializeNativeTargetAsmPrinter();

		auto triple = llvm::sys::getDefaultTargetTriple();
		codeModule->setTargetTriple(triple);

		std::string errorString;
		auto        target = llvm::TargetRegistry::lookupTarget(triple, errorString);

		auto cpu = "generic";
		auto fts = "";

		llvm::TargetOptions opt;
		auto                rm = llvm::Optional<llvm::Reloc::Model>();
		auto machine = target->createTargetMachine(triple, cpu, fts, opt, rm);

		codeModule->setDataLayout(machine->createDataLayout());

		if(shouldOutputObject)
		{
			llvm::legacy::PassManager passManager;
			std::error_code           errorCode;
			llvm::raw_fd_ostream      dest(outputObjectFile, errorCode,
			                               llvm::sys::fs::FA_Write);

			if(errorCode)
			{
				error("Could not open file '{0}'.", outputObjectFile);
			}

			machine->addPassesToEmitFile(passManager, dest, nullptr,
			                             llvm::CGFT_ObjectFile);
			passManager.run(*codeModule);
		}

		if(shouldOutputAssembly)
		{
			llvm::legacy::PassManager passManager;
			std::error_code           errorCode;
			llvm::raw_fd_ostream      dest(outputAssemblyFile, errorCode,
			                               llvm::sys::fs::FA_Write);

			if(errorCode)
			{
				error("Could not open file '{0}'.", outputAssemblyFile);
			}

			machine->addPassesToEmitFile(passManager, dest, nullptr,
			                             llvm::CGFT_AssemblyFile);
			passManager.run(*codeModule);
		}

		if(shouldOutputIR)
		{
			std::error_code      errorCode;
			llvm::raw_fd_ostream dest(outputIRFile, errorCode, llvm::sys::fs::FA_Write);

			if(errorCode)
			{
				error("Could not open file '{0}'.", outputIRFile);
			}

			codeModule->print(dest, nullptr, false, true);
		}
	}

	void GeneratorImpl::provideImpls(AST *ast)
	{
		if(auto n = dynamic_cast<ASTInt *>(ast); n != nullptr)
		{
			n->impl_ = makePtr<ASTIntImpl>(n);
		}
		else if(auto n = dynamic_cast<ASTBlock *>(ast); n != nullptr)
		{
			n->impl_ = makePtr<ASTBlockImpl>(n);
		}
		else if(auto n = dynamic_cast<ASTFunc *>(ast); n != nullptr)
		{
			n->impl_ = makePtr<ASTFuncImpl>(n);
		}
		else if(auto n = dynamic_cast<ASTVar *>(ast); n != nullptr)
		{
			n->impl_ = makePtr<ASTVarImpl>(n);
		}
		else if(auto n = dynamic_cast<ASTVar *>(ast); n != nullptr)
		{
		}
		else
			PANIC("Could not provide impl node!");

		if(ast->impl_)
			ast->impl_->provideImpls(*this);
	}

	Generator::Generator(const std::string &moduleName)
	{
		impl = new GeneratorImpl(moduleName);
	}

	Generator::~Generator() { delete impl; }

	void Generator::set(GeneratorOpt opt, GeneratorBool value) const
	{
		// debug("Generator::set({0}, {1})", (int)opt, value);

		switch(opt)
		{
		case GeneratorOpt::ShouldOutputAssembly:
			impl->shouldOutputAssembly = (bool)value;
			break;
		case GeneratorOpt::ShouldOutputObject:
			impl->shouldOutputObject = (bool)value;
			break;
		case GeneratorOpt::ShouldOutputIR: impl->shouldOutputIR = (bool)value; break;
		default: assert(0 && "Bad option!");
		}
	}

	void Generator::set(GeneratorOpt opt, const std::string &value) const
	{
		// debug("Generator::set({0}, '{1}')", (int)opt, value);

		switch(opt)
		{
		case GeneratorOpt::OutputAssemblyFile: impl->outputAssemblyFile = value; break;
		case GeneratorOpt::OutputObjectFile: impl->outputObjectFile = value; break;
		case GeneratorOpt::OutputIRFile: impl->outputIRFile = value; break;
		default: assert(0 && "Bad option!");
		}
	}

	void Generator::generateFunction(ASTFunc *func) const
	{
		impl->generateFunction(func);
	}
	void Generator::generate(AST *func) const
	{
#define TRY_CAST(TYPE)                   \
	auto n = dynamic_cast<TYPE *>(func); \
	n != nullptr

		if(TRY_CAST(ASTFunc))
			impl->generateFunction(n);
		if(TRY_CAST(ASTVar))
			impl->generateGlobal(n);

#undef TRY_CAST
	}
	void Generator::output() const { impl->output(); }
} // namespace noct
