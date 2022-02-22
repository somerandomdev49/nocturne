#include "types.hpp"

namespace noct
{
	NumericType platformPointerType = NumericType::u64;

	std::size_t TypeNumeric::size() const noexcept
	{
		return getNumericTypeWidth(numeric);
	}

	bool TypeNumeric::assignable(Ptr<Type> out) const noexcept
	{
		if(auto t = dynamic_cast<TypeNumeric*>(out.get()); t != nullptr)
			return size() >= t->size();

		return false;
	}

	void TypeNumeric::print(std::ostream &out) const noexcept
	{
		out << getNumericTypeName(numeric);
	}

	std::size_t TypePointer::size() const noexcept
	{
		return getNumericTypeWidth(platformPointerType);
	}

	bool TypePointer::assignable(Ptr<Type> out) const noexcept
	{
		if(auto t = dynamic_cast<TypeNumeric*>(out.get()); t != nullptr)
			return size() >= t->size();

		return false;
	}

	void TypePointer::print(std::ostream &out) const noexcept
	{
		base->print(out);
		out << "*";
	}
}
