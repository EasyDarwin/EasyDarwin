// Author: xiexu
// Date: 2024-05-01

package fn

import "slices"

// DeduplicationFunc 去重
func DeduplicationFunc[T comparable](vs []T, fn func(T) string) []T {
	uniqueMap := make(map[string]struct{}, len(vs))
	uniqueSlice := make([]T, 0, len(vs))
	for _, v := range vs {
		key := fn(v)
		if _, ok := uniqueMap[key]; !ok {
			uniqueMap[key] = struct{}{}
			uniqueSlice = append(uniqueSlice, v)
		}
	}
	return uniqueSlice
}

// Deduplication 去重
func Deduplication[T comparable](vs ...T) []T {
	uniqueMap := make(map[T]struct{}, len(vs))
	uniqueSlice := make([]T, 0, len(vs))
	for _, v := range vs {
		if _, ok := uniqueMap[v]; !ok {
			uniqueMap[v] = struct{}{}
			uniqueSlice = append(uniqueSlice, v)
		}
	}
	return uniqueSlice
}

// Any 存在指定的值即返回 true
func Any[T comparable](items []T, callback func(T) bool) bool {
	for _, item := range items {
		if callback(item) {
			return true
		}
	}
	return false
}

// Unique 切片中所有值都是唯一，返回 true
func Unique[T comparable](values []T) bool {
	uniqueValues := make(map[T]struct{}, len(values))
	for _, v := range values {
		uniqueValues[v] = struct{}{}
	}
	return len(values) == len(uniqueValues)
}

func Filter[T comparable](values []T, filter func(T) bool) []T {
	out := make([]T, 0, len(values))
	for _, v := range values {
		v := v
		if filter(v) {
			out = append(out, v)
		}
	}
	return out
}

// Reverse 反转数组
func Reverse[T any](s []T) []T {
	arr := slices.Clone(s)
	i, j := 0, len(s)-1
	for i < j {
		arr[i], arr[j] = arr[j], arr[i]
		i++
		j--
	}
	return arr
}
