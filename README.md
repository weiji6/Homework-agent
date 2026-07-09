# 智能学习任务管理系统

基于 Qt / C++ / MySQL 的学习任务管理系统，支持用户登录注册、课程管理、任务管理、学习笔记、数据统计和 AI 学习助手。

## 功能

- 用户注册、登录、退出登录、密码哈希存储
- 课程增删改查，支持星期、教室、教师和上课时间
- 学习任务增删改查，支持课程筛选、状态筛选、截止时间排序和标记完成
- 学习笔记增删改查，支持课程筛选和关键词搜索
- 数据统计：任务状态、课程任务数量、优先级数量
- AI 助手：基于 MySQL 任务数据回答自然语言问题

## 运行准备

1. 安装 Qt，并确保启用 `widgets`、`sql`、`network` 模块。
2. 安装 MySQL，并创建数据库：

```sql
CREATE DATABASE smart_study DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
```

3. 复制 `.env.example` 为 `.env`，再修改项目根目录 `.env` 中的配置。程序会从 `.env` 读取数据库连接和 Agent 配置，登录页不再输入数据库信息。

```env
DB_DRIVER=auto
DB_HOST=127.0.0.1
DB_PORT=3306
DB_NAME=smart_study
DB_USER=root
DB_PASSWORD=
DB_ODBC_DRIVER=MySQL ODBC 8.0 Unicode Driver

LLM_API_KEY=
LLM_API_URL=https://api.openai.com/v1/chat/completions
LLM_MODEL=gpt-4o-mini
LLM_TEMPERATURE=0.3
```

程序会自动创建表，也可以手动执行 `sql/init.sql`。

如果启动时报 `can not load requested driver 'QMYSQL'`，说明当前 Qt 没有 MySQL 插件。保持 `DB_DRIVER=auto` 时，程序会优先用 `QMYSQL`，没有就尝试 `QODBC`。使用 `QODBC` 时需要系统安装 MySQL ODBC Driver，并确保 `DB_ODBC_DRIVER` 名称和 Windows ODBC 驱动名称一致。

开发环境使用 shadow build 时，程序会依次查找：

- 程序目录下的 `.env`
- 当前工作目录下的 `.env`
- 当前工作目录上两级的 `.env`
- 程序目录上三级的 `.env`

## 大模型 API 配置

AI 助手默认会在本地根据任务数据生成回答。需要接入大模型时，在 `.env` 里填写：

```env
LLM_API_KEY=你的 API Key
LLM_API_URL=https://api.openai.com/v1/chat/completions
LLM_MODEL=gpt-4o-mini
LLM_TEMPERATURE=0.3
```

`LLM_API_URL` 使用 OpenAI 兼容的 Chat Completions 接口即可。
