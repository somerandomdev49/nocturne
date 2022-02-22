#pragma once
#include <concepts>
#include <memory>
#include <vector>

namespace noct
{
	template<typename I, typename T>
	concept Iterator = requires(I i, I j)
	{
		{ ++i } -> std::convertible_to<I>;
		{  *i } -> std::convertible_to<T>;
		{ i == j } -> std::convertible_to<bool>;
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
			if(buffer_.size() != 0)
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
			if(buffer_.size() == 0)
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

	template<std::integral T>
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
