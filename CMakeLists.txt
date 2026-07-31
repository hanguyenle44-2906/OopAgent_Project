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
	src/tools/memory_tool.cpp
	src/agent/agent.cpp
	src/agent/skill_loader.cpp
)

target_link_libraries(
	ai_agent PRIVATE
	CURL::libcurl
	nlohmann_json::nlohmann_json
	unofficial::sqlite3::sqlite3
)

add_executable(manual_test
	manual_test.cpp
	src/tools/tool_registry.cpp
	src/tools/memory_tool.cpp
	src/agent/skill_loader.cpp
)
target_link_libraries(manual_test PRIVATE
	nlohmann_json::nlohmann_json
	unofficial::sqlite3::sqlite3
)

# Tu dong copy thu muc skills/ vao cung cho voi file .exe moi lan build,
# de chuong trinh luon tim thay "skills" bang duong dan tuong doi, bat ke
# working directory duoc IDE cau hinh la gi.
add_custom_command(TARGET ai_agent POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/skills"
		"$<TARGET_FILE_DIR:ai_agent>/skills"
)
add_custom_command(TARGET manual_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/skills"
		"$<TARGET_FILE_DIR:manual_test>/skills"
)