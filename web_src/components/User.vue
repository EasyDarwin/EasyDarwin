<template>
    <div class="box box-success">
        <div class="box-header">
            <h4 class="text-success text-center">用户列表</h4>
            <div class="form-inline">
                <div class="form-group">
                    <button type="button" class="btn btn-sm btn-success" @click.prevent="$refs['userDlg'].show()"><i class="fa fa-plus"></i> 新增用户</button>
                </div>
                <div class="form-group pull-right">
					<div class="input-group">
						<input type="text" class="form-control" spellcheck="false" placeholder="搜索" v-model.trim="query" ref="search" @keyup.enter="doSearch">
						<div class="input-group-btn">
							<button type="button" class="btn btn-default" @click.prevent="doSearch">
								<i class="fa fa-search"></i>
							</button>
						</div>
					</div>                        
                </div>
            </div>
        </div>
        <div class="box-body">
            <el-table :data="users" stripe>
                <el-table-column min-width="120" label="用户名" prop="name"></el-table-column>
				<el-table-column width="230" label="操作" fixed="right">
                    <template slot-scope="scope">
                        <div class="btn-group">
                            <a role="button" class="btn btn-success btn-xs" @click.prevent="$refs['userDlg'].show(scope.row)">
                                <i class='fa fa-edit'></i> 编辑
                            </a>
                            <a role="button" class="btn btn-warning btn-xs" @click.prevent="resetPwd(scope.row)">
                                <i class='fa fa-key'></i> 重置密码
                            </a>
                            <a role="button" class="btn btn-danger btn-xs" @click.prevent="remove(scope.row)">
                                <i class='fa fa-remove'></i> 删除
                            </a>
                        </div>
                    </template>
				</el-table-column>                
            </el-table>
        </div>
        <div class="box-footer clearfix">
            <el-pagination layout="prev,pager,next" class="pull-right" :total="total" :page-size.sync="pageSize" :current-page="page"></el-pagination>
        </div>

        <UserFormDlg ref="userDlg" @submit="load" :defaultPwd="defaultPwd"></UserFormDlg>
    </div>
</template>

<script>
import UserFormDlg from 'components/UserFormDlg.vue'

import $ from 'jquery'

export default {
    props: {
        page: {
            type: Number,
            default: 1
        },
        q: {
            type: String,
            default: ''
        }
    },
    data() {
        return {
            query: '',
            defaultPwd: '',
            pageSize: 10,
            total: 0,
            users: []
        }
    },
    activated() {
        this.$refs['search'].focus();
    },
    components: {
        UserFormDlg
    },
    mounted() {
        this.query = this.q;
        this.$refs['search'].focus();
        this.load();
        $.get('/user/defaultPwd').then(data => {
            this.defaultPwd = data;
        })
    },
    computed: {
    },
    methods: {
        load() {
            $.get("/user/list", {
                q: this.q,
                start: (this.currentPage - 1) * this.pageSize,
                limit: this.pageSize
            }).then(data => {
                this.total = data.total;
                this.users = data.rows;
            })
        },
        remove(row) {
            this.$confirm(`确认删除用户 ${row.name} ?`, '提示').then(() => {
                $.get('/user/remove', {
                    id: row.id
                }).always(() => {
                    this.load();
                })
            }).catch(() => {})
        },
        resetPwd(row) {
            this.$confirm(`确认要重置用户 ${row.name} 登录密码为默认密码 ${this.defaultPwd} 吗?`, '提示').then(() => {
                $.get('/user/resetPwd', {
                    id: row.id
                }).then(data => {
                    this.$message({
                        type: 'success',
                        message: '密码重置成功'
                    });
                })
            }).catch(() => {})
        },
        doSearch() {
            this.$router.push(`/users/${this.page}${this.query ? '?q=' + this.query : ''}`);
        },       
    },
    beforeRouteEnter(to, from, next) {
        if(!to.params.page) {
            return next({
                path: `/users/1`,
                replace: true
            });
        }
        to.params.page = parseInt(to.params.page) || 1;
        to.params.q = to.query.q;
        return next();
    },
    beforeRouteUpdate(to, from, next) {
        if(!to.params.page) {
            return next({
                path: `/users/1`,
                replace: true
            });
        }
        to.params.page = parseInt(to.params.page) || 1;
        to.params.q = to.query.q;
        next();
        this.$nextTick(() => {
            this.query = this.q;
            this.load();
        })
    }
}
</script>
