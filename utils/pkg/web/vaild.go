package web

// Validator 验证对象是否合法
type Validator struct {
	Errors map[string]string
}

// NewValidator ...
func NewValidator() *Validator {
	return &Validator{Errors: make(map[string]string)}
}

// Valid 如果没有错误，返回 true
func (v *Validator) Valid() bool {
	return len(v.Errors) == 0
}

// AddError 添加错误
func (v *Validator) AddError(key, message string) *Validator {
	if _, exist := v.Errors[key]; !exist {
		v.Errors[key] = message
	}
	return v
}

// Check 如果 ok==false，则将 message 写入 errors
func (v *Validator) Check(ok bool, key, message string) *Validator {
	if !ok {
		v.AddError(key, message)
	}
	return v
}

// Result true 表示没有错误
func (v *Validator) Result() (bool, []string) {
	return v.Valid(), v.List()
}

// List 验证 !Valid() 后，可获取错误列表
func (v *Validator) List() []string {
	tmp := make([]string, 0, len(v.Errors))
	for k, v := range v.Errors {
		tmp = append(tmp, k+" "+v)
	}
	return tmp
}
