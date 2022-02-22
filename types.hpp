#pragma once
#include "util.hpp"

#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace noct
{
	enum class TypeType
	{
		numeric,
		structural,
		unknown
	};
	
	struct Type
	{
		TypeType type;

		Type(TypeType type) : type(type) {}

		virtual std::size_t size() const noexcept = 0;
		virtual bool assignable(Ptr<Type> out) const noexcept = 0;
		virtual void print(std::ostream &out) const noexcept = 0;
	};

	enum class NumericType
	{
		u8, u16, u32, u64,
		i8, i16, i32, i64,
		f32, f64, unknown,
	};

	extern NumericType platformPointerType;

	template<Integral T>
	constexpr auto getSmallestIntegerTypeFor(T val) -> NumericType
	{
		if(val >= 0)
		{
			if(val <= pow<std::size_t>(2, 8 ) / 2) return NumericType::i8 ;
			if(val <= pow<std::size_t>(2, 8 )    ) return NumericType::u8 ;
			if(val <= pow<std::size_t>(2, 16) / 2) return NumericType::i16;
			if(val <= pow<std::size_t>(2, 16)    ) return NumericType::u16;
			if(val <= pow<std::size_t>(2, 32) / 2) return NumericType::i32;
			if(val <= pow<std::size_t>(2, 32)    ) return NumericType::u32;
			if(val <= pow<std::size_t>(2, 64) / 2) return NumericType::i64;
			if(val <= pow<std::size_t>(2, 64)    ) return NumericType::u64;
		} 
		else
		{
			val = -val;
			if(val <= pow<std::size_t>(2, 8 ) / 2) return NumericType::i8 ;
			if(val <= pow<std::size_t>(2, 16) / 2) return NumericType::i16;
			if(val <= pow<std::size_t>(2, 32) / 2) return NumericType::i32;
			if(val <= pow<std::size_t>(2, 64) / 2) return NumericType::i64;
		}
		return NumericType::unknown;
	}

	template<Integral T>
	constexpr auto getSuitableIntegerTypeFor(T val) -> NumericType
	{
		if(val >= 0)
		{
			if(val <= pow<std::size_t>(2, 32) / 2) return NumericType::i32;
			if(val <= pow<std::size_t>(2, 32)    ) return NumericType::u32;
			if(val <= pow<std::size_t>(2, 64) / 2) return NumericType::i64;
			if(val <= pow<std::size_t>(2, 64)    ) return NumericType::u64;
		} 
		else
		{
			val = -val;
			if(val <= pow<std::size_t>(2, 32) / 2) return NumericType::i32;
			if(val <= pow<std::size_t>(2, 64) / 2) return NumericType::i64;
		}
		return NumericType::unknown;
	}

	constexpr auto isSignedNumericType(NumericType t) -> bool
	{
		switch(t)
		{
		case NumericType::u8 : return false;
		case NumericType::u16: return false;
		case NumericType::u32: return false;
		case NumericType::u64: return false;
		case NumericType::i8 : return true;
		case NumericType::i16: return true;
		case NumericType::i32: return true;
		case NumericType::i64: return true;
		case NumericType::f32: return true;
		case NumericType::f64: return true;
		default: return false;
		}
	}
	
	constexpr auto getNumericTypeWidth(NumericType t) -> std::uint8_t
	{
		switch(t)
		{
		case NumericType::u8 : return 1;
		case NumericType::u16: return 2;
		case NumericType::u32: return 4;
		case NumericType::u64: return 8;
		case NumericType::i8 : return 1;
		case NumericType::i16: return 2;
		case NumericType::i32: return 4;
		case NumericType::i64: return 8;
		case NumericType::f32: return 4;
		case NumericType::f64: return 8;
		default: return 0;
		}
	}

	constexpr auto getNumericTypeName(NumericType t) -> const char * {
		switch(t)
		{
		case NumericType::u8 : return "u8" ;
		case NumericType::u16: return "u16";
		case NumericType::u32: return "u32";
		case NumericType::u64: return "u64";
		case NumericType::i8 : return "i8" ;
		case NumericType::i16: return "i16";
		case NumericType::i32: return "i32";
		case NumericType::i64: return "i64";
		case NumericType::f32: return "f32";
		case NumericType::f64: return "f64";
		default: return "<numeric unknown>";
		}
	}

	struct TypeNumeric : Type
	{
		NumericType numeric;

		TypeNumeric(NumericType numeric) : Type(TypeType::numeric), numeric(numeric) {}

		virtual std::size_t size() const noexcept override;
		virtual bool assignable(Ptr<Type> out) const noexcept override;
		virtual void print(std::ostream &out) const noexcept override;
	};
	
	struct TypePointer : Type
	{
		Ptr<Type> base;

		TypePointer(Ptr<Type> base) : Type(TypeType::numeric), base(base) {}

		virtual std::size_t size() const noexcept override;
		virtual bool assignable(Ptr<Type> out) const noexcept override;
		virtual void print(std::ostream &out) const noexcept override;
	};
	
	struct FuncSignature
	{
		Ptr<Type> returnType;
		std::vector<Ptr<Type>> argTypes;
	};
}
