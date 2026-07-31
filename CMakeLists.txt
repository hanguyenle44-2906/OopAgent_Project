cmake_minimum_required(VERSION 3.20)
project(AgentTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_executable(
	ai_agent
	main.cpp
	src/client/ollama_client.cpp
	src/tools/tool_registry.cpp
	src/tools/calculator_tool.cpp
	src/tools/file_tool.cpp
	src/tools/exec_tool.cpp
	src/tools/web_tool.cpp
	src/agent/agent.cpp
)
target_link_libraries(
	ai_agent PRIVATE
	CURL::libcurl
	nlohmann_json::nlohmann_json
	unofficial::sqlite3::sqlite3
)