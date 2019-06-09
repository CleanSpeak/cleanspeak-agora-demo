cmake_minimum_required(VERSION 3.14)
project(agora_cleanspeak)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Boost 1.36.0 REQUIRED COMPONENTS system filesystem)

add_executable(AgoraAutoModerator src/main.cpp src/ModeratorHandler.cpp src/AudioContainer.cpp src/CleanSpeakAudioModerationUtil.cpp)

target_link_directories(AgoraAutoModerator PUBLIC libs)
target_link_libraries(AgoraAutoModerator librecorder.a pthread curl ${Boost_LIBRARIES})
target_include_directories(AgoraAutoModerator PUBLIC include)
