#pragma once
#include <concepts>
#include <type_traits>
#include <memory>
#include <vector>

namespace noct
{
	// For some reason, brew clang-11 lacks most concept implementations :(
	
	template<typename T>
	concept Integral = requires {
		requires std::is_integral_v<T>;
	};

	template<typename T, typename U>
	concept ConvertibleTo = requires {
		requires std::is_convertible_v<T, U>;
	};

	template<typename I, typename T>
	concept Iterator = requires(I i, I j)
	{
		{ ++i } -> ConvertibleTo<I>;
		{  *i } -> ConvertibleTo<T>;
		{ i == j } -> ConvertibleTo<bool>;
	};
	
	template<typename I, typename T>
	concept Iterable = requires(I i)
	{
		requires std::is_same_v<decltype(i.begin()), decltype(i.end())>;
		requires Iterator<decltype(i.begin()), T>;
	};

	template<typename T, Iterator<T> U>
	class BufferedIterator
	{
	public:
		BufferedIterator(U &&orig) : iter_(std::move(orig)) {}

		auto get() -> T
		{
			if(not buffer_.empty())
			{
				auto tmp = buffer_.back();
				buffer_.pop_back();
				return tmp;
			}

			++iter_;
			auto v = *iter_;
			return v;
		}
		
		auto peek() -> T&
		{
			if(buffer_.empty())
			{
				++iter_;
				auto v = *iter_;
				buffer_.push_back(v);
			}

			return buffer_.back();
		}

	private:
		U iter_;
		std::vector<T> buffer_;
	};

	template<typename T, Iterable<T> U>
	class BufferedIterable
	{
	public:
		using Iterator = decltype(std::declval<U>().begin());

		BufferedIterable(U &base) : base_(base) {}

		auto begin() const { return BufferedIterator<T, Iterator>(base_.begin()); }
		auto end() const { return BufferedIterator<T, Iterator>(base_.end()); }
	private:
		U &base_;
	};
	
	template<typename T>
	using Ptr = std::shared_ptr<T>;

	template<typename T, typename ...Args>
	auto makePtr(Args &&...args) -> Ptr<T> {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<Integral T>
	constexpr auto pow(T base, T val) -> T
	{
		if(val == 1 || base == 1) return base;
		T x = 1;
		while(val--) x *= base;
		return x;
	}

	template<typename T>
	struct Result
	{
		bool error;
		T value;

		T *operator->() { return &value; }
	};
}
