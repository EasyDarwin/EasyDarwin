
<h1 align="center" style="margin: 30px 0 30px; font-weight: bold;">EasyDarwin v8.3.2</h1>

## 平台简介

* 本仓库为前端技术栈 [Vue3](https://v3.cn.vuejs.org) + [Vite](https://cn.vitejs.dev) + [Ant](https://www.antdv.com/docs/vue/introduce-cn) + [Pinia](https://pinia.vuejs.org/zh/introduction.html)  搭建的后台管理系统模板。

## 前端运行

```bash
# 克隆项目
git https://github.com/EasyDarwin/EasyDarwin.git

# 进入项目目录
cd EasyDarwin/easydarwin8_web

# node环境
v18.19.0

# 安装依赖
npm install

# 启动服务
npm run dev

# 构建生产环境
npm run build

# 构建测试环境 npm run build
# 构建生产环境 npm run build
# 前端访问地址 http://localhost:3001
```


## 项目结构
<pre>
├─public # 静态资源文件
├─src # 源代码目录
│  ├─api # API 请求相关文件
│  ├─assets # 静态资源文件
│  ├─components # 可复用的 Vue 组件
│  ├─layouts # 布局组件
│  ├─plugins  # 插件配置
│  ├─router # 路由配置
│  ├─settings # 项目设置
│  ├─store # Vuex 状态管理
│  ├─styles # 样式文件
│  ├─utils # 工具函数
│  └─views # 页面视图
├─.editorconfig # 代码格式配置
├─.env.development # 开发环境变量
├─.prettierrc.json # 代码格式配置
├─index.html # 入口 HTML 文件
├─jsconfig.json # 项目配置
├─package.json # 项目依赖
├─README.md # 项目说明
├─vite.config.ts # Vite 配置文件
├─unocss.config.js # 样式配置文件
</pre>

## 内置功能

1.  登录：用户登录。
2.  直播服务：用户通过创建添加拉流地址进行直播功能、创建推流地址，被配置到推流工具中进行直播。
13.  接口文档：查看服务中接口的调用。
14.  版本信息：记录服务当前运行情况，以及历史版本。

