#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <iterator>
#include "array_ptr.h"

class ReserveProxyObj {
public:
	ReserveProxyObj(size_t new_capacity)
		: capacity_(new_capacity)
	{
	}
	
	size_t GetCapacity() const {
		return capacity_;
	}
	
private:
	size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
	return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
	using Iterator = Type*;
	using ConstIterator = const Type*;
	
	SimpleVector() noexcept = default;
	
	// Создаёт вектор из size элементов, инициализированных значением по умолчанию
	explicit SimpleVector(size_t size) 
		: array_(size)
		, size_(size)
		, capacity_(size)
	{
		std::fill(begin(), end(), 0);
	}
	
	// Создаёт вектор из size элементов, инициализированных значением value
	SimpleVector(size_t size, const Type& value) 
		: array_(size)
		, size_(size)
		, capacity_(size)
	{
		std::fill(begin(), end(), value);
	}
	
	// Создаёт вектор из std::initializer_list
	SimpleVector(std::initializer_list<Type> init) 
		: array_(init.size())
		, size_(init.size())
		, capacity_(init.size())
	{
		std::copy(init.begin(), init.end(), begin());
	}
	
	SimpleVector(const ReserveProxyObj other) {
		Reserve(other.GetCapacity());
	}
	
	SimpleVector(const SimpleVector& other) {
		assert(size_ == 0 && capacity_ == 0);
		
		SimpleVector<Type> temp(other.GetSize());
		std::move(other.begin(), other.end(), temp.begin());
		swap(temp);
	}
	
	SimpleVector& operator=(const SimpleVector& rhs) {
		if(this != &rhs) {
			SimpleVector<Type> temp(rhs);
			swap(temp);
		}
		
		return *this;
	}
	
	// Создаёт вектор из size элементов, инициализированных значением value
	SimpleVector(size_t size, Type&& value) 
		: array_(size)
		, size_(size)
		, capacity_(size)
	{
		std::fill(begin(), end(), std::move(value));
	}
	
	SimpleVector(SimpleVector&& other) {
		assert(size_ == 0 && capacity_ == 0);
		
		SimpleVector<Type> temp(other.GetSize());
		std::move(other.begin(), other.end(), temp.begin());
		other.Clear();
		swap(temp);
	}
	
	SimpleVector& operator=(SimpleVector&& rhs) {
		if(this != &rhs) {
			SimpleVector<Type> temp(std::move(rhs));
			swap(temp);
		}
		
		return *this;
	}
	
	void Reserve(size_t new_capacity) {
		if(new_capacity > GetCapacity()) {
			size_t old_size_ = GetSize();
			Resize(new_capacity);
			size_ = old_size_;
		}
	}
	
	// Изменяет размер массива.
	// При увеличении размера новые элементы получают значение по умолчанию для типа Type
	void Resize(size_t new_size) {
		if(new_size <= size_) {
			size_ = new_size;
		} else if(new_size <= capacity_) {
			std::fill(end(), end() + new_size, 0);
			size_ = new_size;
		} else {
			SimpleVector<Type> other(new_size);
			std::move(begin(), end(), other.begin());
			swap(other);
		}
	}
	
	// Добавляет элемент в конец вектора
	// При нехватке места увеличивает вдвое вместимость вектора
	void PushBack(const Type& item) {
		if(IsEmpty() && GetCapacity() == 0) {
			Resize(1);
			size_ = 0;
		} else if(GetSize() >= GetCapacity()) {
			size_t old_size_ = GetSize();
			Resize(GetCapacity() * 2);
			size_ = old_size_;
		}
		
		array_[size_++] = std::move(item);
	}
	
	// Добавляет элемент в конец вектора
	// При нехватке места увеличивает вдвое вместимость вектора
	void PushBack(Type&& item) {
		if(IsEmpty() && GetCapacity() == 0) {
			Resize(1);
			size_ = 0;
		} else if(GetSize() >= GetCapacity()) {
			size_t old_size_ = GetSize();
			Resize(GetCapacity() * 2);
			size_ = old_size_;
		}
		
		array_[size_++] = std::move(item);
	}
	
	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, const Type& value) {
		int distance_ = std::distance(cbegin(), pos);
		if(IsEmpty() && GetCapacity() == 0) {
			Resize(1);
			size_ = 0;
		} else if(GetSize() >= GetCapacity()) {
			size_t old_size_ = GetSize();
			Resize(GetCapacity() * 2);
			size_ = old_size_;
			std::move_backward(begin() + distance_, end(), end() + 1);
		} else {
			std::move_backward(begin() + distance_, cend(), end() + 1);
		}
		*(begin() + distance_) = std::move(value);
		++size_;
		
		return (begin() + distance_);
	}
	
	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, Type&& value) {
		int distance_ = std::distance(cbegin(), pos);
		if(IsEmpty() && GetCapacity() == 0) {
			Resize(1);
			size_ = 0;
		} else if(GetSize() >= GetCapacity()) {
			size_t old_size_ = GetSize();
			Resize(GetCapacity() * 2);
			size_ = old_size_;
			std::move_backward(begin() + distance_, end(), end() + 1);
		} else {
			std::move_backward(begin() + distance_, end(), end() + 1);
		}
		*(begin() + distance_) = std::move(value);
		++size_;
		
		return (begin() + distance_);
	}
	
	// "Удаляет" последний элемент вектора. Вектор не должен быть пустым
	void PopBack() noexcept {
		assert(!IsEmpty());
		
		--size_;
	}
	
	// Удаляет элемент вектора в указанной позиции
	Iterator Erase(ConstIterator pos) {
		int distance_ = std::distance(cbegin(), pos);
		std::move(begin() + distance_ + 1, end(), begin() + distance_);
		--size_;
		
		return (begin() + distance_);
	}
	
	// Обменивает значение с другим вектором
	void swap(SimpleVector& other) noexcept {
		std::swap(capacity_, other.capacity_);
		std::swap(size_, other.size_);
		array_.swap(other.array_);
	}
	
	// Возвращает количество элементов в массиве
	size_t GetSize() const noexcept {
		return size_;
	}
	
	// Возвращает вместимость массива
	size_t GetCapacity() const noexcept {
		return capacity_;
	}
	
	// Сообщает, пустой ли массив
	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	
	// Возвращает ссылку на элемент с индексом index
	Type& operator[](size_t index) noexcept {
		return array_[index];
	}
	
	// Возвращает константную ссылку на элемент с индексом index
	const Type& operator[](size_t index) const noexcept {
		return array_[index];
	}
	
	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	Type& At(size_t index) {
		if(index >= size_) {
			throw std::out_of_range("Invalid index");
		}
		
		return array_[index];
	}
	
	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	const Type& At(size_t index) const {
		if(index >= size_) {
			throw std::out_of_range("Invalid index");
		}
		
		return array_[index];
	}
	
	// Обнуляет размер массива, не изменяя его вместимость
	void Clear() noexcept {
		size_ = 0;
	}
	
	// Возвращает итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	Iterator begin() noexcept {
		return array_.Get();
	}
	
	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	Iterator end() noexcept {
		return array_.Get() + GetSize();
	}
	
	// Возвращает константный итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator begin() const noexcept {
		return cbegin();
	}
	
	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator end() const noexcept {
		return cend();
	}
	
	// Возвращает константный итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator cbegin() const noexcept {
		return array_.Get();
	}
	
	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator cend() const noexcept {
		return array_.Get() + GetSize();
	}
	
	private:
	ArrayPtr<Type> array_;
	size_t size_ = 0;
	size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return (&lhs == &rhs)
		|| (lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(rhs > lhs);
}