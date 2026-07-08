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

3. 运行程序后，在登录页填写 MySQL 连接信息。程序会自动创建表，也可以手动执行 `sql/init.sql`。

## 大模型 API 配置

AI 助手默认会在本地根据任务数据生成回答。需要接入大模型时，设置环境变量：

```powershell
$env:LLM_API_KEY="你的 API Key"
$env:LLM_API_URL="https://api.openai.com/v1/chat/completions"
$env:LLM_MODEL="gpt-4o-mini"
```

`LLM_API_URL` 使用 OpenAI 兼容的 Chat Completions 接口即可。
