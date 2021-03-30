#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <algorithm>
#include "array_ptr.h"
#include <iostream>
struct ReserveProxyObj {
	size_t capacity;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
	return ReserveProxyObj{capacity_to_reserve};
}

template <typename Type>
class SimpleVector {
public:
	using Iterator = Type*;
	using ConstIterator = const Type*;

	SimpleVector() noexcept = default;

	// Создаёт вектор из size элементов, инициализированных значением по умолчанию
	explicit SimpleVector(size_t size) 
		: SimpleVector(size, Type())
	{
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

	SimpleVector(const ReserveProxyObj other) 
		: array_(other.capacity)
		, size_(0)
		, capacity_(other.capacity)
	{
	}

	SimpleVector(const SimpleVector& other)
		: array_(other.GetSize())
		, size_(other.GetSize())
		, capacity_(other.GetSize())
	{
		std::copy(other.begin(), other.end(), begin());
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
		std::fill(begin(), end(), value);
	}

	SimpleVector(SimpleVector&& other)
		: array_(std::move(other.array_))
		, size_(std::move(other.size_))
		, capacity_(std::move(other.capacity_))
	{
		other.Clear();
        other.capacity_ = 0;
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
			IncreaseCapacity(new_capacity);
		}
	}

	// Изменяет размер массива.
	// При увеличении размера новые элементы получают значение по умолчанию для типа Type
	void Resize(size_t new_size) {
		if(new_size <= size_) {
			size_ = new_size;
		} else if(new_size <= capacity_) {
			std::fill(end(), end() + new_size, Type());
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
			IncreaseCapacity(1);
		} else if(GetSize() >= GetCapacity()) {
			IncreaseCapacity(GetCapacity() * 2);
		}

		array_[size_++] = item;
	}

	// Добавляет элемент в конец вектора
	// При нехватке места увеличивает вдвое вместимость вектора
	void PushBack(Type&& item) {
		if(IsEmpty() && GetCapacity() == 0) {
			IncreaseCapacity(1);
		} else if(GetSize() >= GetCapacity()) {
			IncreaseCapacity(GetCapacity() * 2);
		}
        
		array_[size_++] = std::move(item);
	}

	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, const Type& value) {
		const size_t distance_ = std::distance(cbegin(), pos);
		if(IsEmpty() && GetCapacity() == 0) {
			IncreaseCapacity(1);
		} else if(GetSize() >= GetCapacity()) {
			IncreaseCapacity(GetCapacity() * 2);
			std::copy_backward(begin() + distance_, end(), end() + 1);
		} else {
			std::copy_backward(begin() + distance_, end(), end() + 1);
		}
		*(begin() + distance_) = value;
		++size_;

		return (begin() + distance_);
	}

	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, Type&& value) {
		const size_t distance_ = std::distance(cbegin(), pos);
		if(IsEmpty() && GetCapacity() == 0) {
			IncreaseCapacity(1);
		} else if(GetSize() >= GetCapacity()) {
			IncreaseCapacity(GetCapacity() * 2);
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
		const size_t distance_ = std::distance(cbegin(), pos);
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
		assert(index < size_);

		return array_[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	const Type& operator[](size_t index) const noexcept {
		assert(index < size_);

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

	void IncreaseCapacity(const size_t new_capacity) {
		ArrayPtr<Type> new_array_(new_capacity);
		for(size_t index = 0; index < GetSize(); ++index) {
			new_array_[index] =  std::move(array_[index]);
		}
		array_.swap(new_array_);
		capacity_ = new_capacity;
	}
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