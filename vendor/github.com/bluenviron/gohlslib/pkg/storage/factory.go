// Package storage contains the storage mechanism of segments and parts.
package storage

// Factory allows to allocate the storage behind init files, segments and parts.
type Factory interface {
	// NewFile allocates a file
	NewFile(fileName string) (File, error)
}
