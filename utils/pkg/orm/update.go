package orm

import (
	"context"
	"fmt"

	"gorm.io/gorm"
	"gorm.io/gorm/clause"
)

// QueryOption ..
type QueryOption func(*gorm.DB) *gorm.DB

type Query struct {
	data []QueryOption
}

func NewQuery(l int) *Query {
	return &Query{
		data: make([]QueryOption, 0, l),
	}
}

func (q *Query) Where(query any, args ...any) *Query {
	q.data = append(q.data, Where(query, args...))
	return q
}

func (q *Query) OrderBy(value any) *Query {
	q.data = append(q.data, OrderBy(value))
	return q
}

func (q *Query) Encode() []QueryOption {
	return q.data
}

// Where 查询条件
func Where(query any, args ...any) QueryOption {
	return func(d *gorm.DB) *gorm.DB {
		return d.Where(query, args...)
	}
}

// OrderBy 排序条件
func OrderBy(value any) QueryOption {
	return func(d *gorm.DB) *gorm.DB {
		return d.Order(value)
	}
}

// Universal 通用增删改查
type Universal[T any] interface {
	Get(context.Context, *T, ...QueryOption) error
	Edit(context.Context, *T, func(*T) error, ...QueryOption) error
	Del(context.Context, *T, ...QueryOption) error
	Add(context.Context, *T) error
	Find(context.Context, *[]*T, Pager, ...QueryOption) (int64, error)
}

// UniversalSession 通用事务
type UniversalSession[T any] interface {
	Session(ctx context.Context, changeFns ...func(*gorm.DB) error) error
	EditWithSession(tx *gorm.DB, model *T, changeFn func(*T) error, opts ...QueryOption) error
}

type Type[T any] struct {
	db *gorm.DB
}

func NewType[T any](db *gorm.DB) Type[T] {
	return Type[T]{db: db}
}

// First 通用查询
func (t Type[T]) Get(ctx context.Context, out *T, opts ...QueryOption) error {
	return FirstWithContext(ctx, t.db, out, opts...)
}

func First(db *gorm.DB, out any, opts ...QueryOption) error {
	return FirstWithContext(context.TODO(), db, out, opts...)
}

func FirstWithContext(ctx context.Context, db *gorm.DB, out any, opts ...QueryOption) error {
	if len(opts) == 0 {
		panic("where is empty")
	}
	for _, opt := range opts {
		db = opt(db)
	}
	return db.WithContext(ctx).First(out).Error
}

// Update 通用更新
func (t Type[T]) Edit(ctx context.Context, model *T, changeFn func(*T) error, opts ...QueryOption) error {
	return UpdateWithContext2(ctx, t.db, model, changeFn, opts...)
}

func (t Type[T]) Add(ctx context.Context, model *T) error {
	return t.db.WithContext(ctx).Create(model).Error
}

func Update[T any](db *gorm.DB, model *T, changeFn func(*T), opts ...QueryOption) error {
	return UpdateWithContext(context.TODO(), db, model, changeFn, opts...)
}

func UpdateWithContext[T any](ctx context.Context, db *gorm.DB, model *T, changeFn func(*T), opts ...QueryOption) error {
	if len(opts) == 0 {
		panic("where is empty")
	}
	return db.WithContext(ctx).Transaction(func(tx *gorm.DB) error {
		{
			tx := tx.Clauses(clause.Locking{Strength: "UPDATE"})
			for _, opt := range opts {
				tx = opt(tx)
			}
			if err := tx.First(model).Error; err != nil {
				return err
			}
		}
		changeFn(model)
		return tx.Save(model).Error
	})
}

func UpdateWithContext2[T any](ctx context.Context, db *gorm.DB, model *T, changeFn func(*T) error, opts ...QueryOption) error {
	if len(opts) == 0 {
		panic("where is empty")
	}
	return db.WithContext(ctx).Transaction(func(tx *gorm.DB) error {
		{
			tx := tx.Clauses(clause.Locking{Strength: "UPDATE"})
			for _, opt := range opts {
				tx = opt(tx)
			}
			if err := tx.First(model).Error; err != nil {
				return err
			}
		}
		if err := changeFn(model); err != nil {
			return err
		}
		return tx.Save(model).Error
	})
}

func UpdateWithSession[T any](tx *gorm.DB, model *T, fn func(*T) error, opts ...QueryOption) error {
	if len(opts) == 0 {
		panic("where is empty")
	}
	{
		tx := tx.Clauses(clause.Locking{Strength: "UPDATE"})
		for _, opt := range opts {
			tx = opt(tx)
		}
		if err := tx.First(model).Error; err != nil {
			return err
		}
	}
	if err := fn(model); err != nil {
		return err
	}
	return tx.Save(model).Error
}

// Delete 通用删除
func (t Type[T]) Del(ctx context.Context, model *T, opts ...QueryOption) error {
	return DeleteWithContext(ctx, t.db, model, opts...)
}

func Delete(db *gorm.DB, model any, opts ...QueryOption) error {
	return DeleteWithContext(context.TODO(), db, model, opts...)
}

func DeleteWithContext(ctx context.Context, db *gorm.DB, model any, opts ...QueryOption) error {
	if len(opts) == 0 {
		return fmt.Errorf("where is empty")
	}
	db = db.Clauses(clause.Returning{})
	for _, opt := range opts {
		db = opt(db)
	}
	return db.WithContext(ctx).Delete(model).Error
}

type Pager interface {
	Limit() int
	Offset() int
}

func (t Type[T]) Find(ctx context.Context, out *[]*T, p Pager, opts ...QueryOption) (int64, error) {
	return FindWithContext(ctx, t.db, out, p, opts...)
}

func Find[T any](db *gorm.DB, out *[]*T, p Pager, opts ...QueryOption) (int64, error) {
	return FindWithContext(context.TODO(), db, out, p, opts...)
}

func FindWithContext[T any](ctx context.Context, db *gorm.DB, out *[]*T, p Pager, opts ...QueryOption) (int64, error) {
	db = db.Model(new(T)).WithContext(ctx)
	for _, opt := range opts {
		db = opt(db)
	}
	var total int64
	if err := db.Count(&total).Error; err != nil || total <= 0 {
		return total, err
	}
	return total, db.Limit(p.Limit()).Offset(p.Offset()).Find(out).Error
}
