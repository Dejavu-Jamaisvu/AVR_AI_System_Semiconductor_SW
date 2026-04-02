cmake_minimum_required(VERSION 3.10)
project(SerialMonitor C)

# MSVC 컴파일러 인코딩 경고(C4819) 해결
if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# 외부 libserialport 소스 경로를 변수로 지정
# set(LSP_DIR "D:/libserialport")
set(LSP_DIR "D:/AVR/AVR_AI_System_Semiconductor_SW/libserialport")

# 빈 config.h 파일을 빌드 디렉토리에 자동 생성 (Autotools 에러 우회)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.h "")

# 1. libserialport 소스 파일 지정 (절대 경로 변수 활용, 윈도우 기준)
set(LSP_SOURCES
    ${LSP_DIR}/serialport.c
    ${LSP_DIR}/windows.c
    ${LSP_DIR}/timing.c
)

# 2. 실행 파일 생성 (내 소스 + 외부 경로의 라이브러리 소스 동시 컴파일)
add_executable(SerialMonitor main.c ${LSP_SOURCES})

# 3. 헤더 파일 탐색 경로 지정 (외부 폴더 + 생성된 config.h 위치 포함)
target_include_directories(SerialMonitor PRIVATE 
    ${LSP_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# 4. 컴파일 매크로 정의 (동적 링크용 매크로 무효화)
target_compile_definitions(SerialMonitor PRIVATE 
    HAVE_CONFIG_H 
    SP_PRIV=
)

# 5. 윈도우 전용 시스템 라이브러리 링크 (장치 인식용 필수)
if(WIN32)
    target_link_libraries(SerialMonitor PRIVATE setupapi advapi32)
endif()