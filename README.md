# OopAgent_Project

# C++ AI Agent Framework & Evaluation Harness

Dự án AI Agent bằng C++17 kết nối với Local LLM Server (Ollama API), áp dụng các mô hình thiết kế hướng đối tượng (OOP Design Patterns) như **Template Method**, **Observer Pattern**, và **Strategy Pattern**.

## 🏗️ Cấu trúc dự án
- `src/agent/`: Chứa core Agent Loop (ReAct) và Loop Detector.
- `src/client/`: Ollama Client xử lý giao tiếp REST API qua `libcurl`.
- `src/tools/`: Tool Registry và các công cụ thực thi (Exec, File, Calc, Memory, Web).
- `src/harness/`: Harness Runner, Evaluators (Keyword, Functional), và Trajectory Logger.
- `skills/`: Các file chỉ dẫn Prompt Engineering dạng Markdown (`.md`).
- `benchmark/`: Tập 10 bài test (`tasks.json`) và chương trình chạy đánh giá tự động (`run_eval.cpp`).
- `docs/`: Sơ đồ thiết kế hệ thống (UML Diagrams).

## 🚀 Hướng dẫn biên dịch & chạy chương trình

### 1. Khởi động Ollama Server
Đảm bảo dịch vụ Ollama đã được bật và đã pull model:
```bash
ollama serve &
ollama pull qwen2.5:0.5b