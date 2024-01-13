cmake_minimum_required(VERSION 3.20)


add_executable(SpiderApp
	main.cpp
	http_utils.h
	http_utils.cpp
	link.h
	client.h
	client.cpp
	../config/config.h
	../config/config.cpp
	../config/config.ini
	../database/database.h
	../database/database.cpp
	../indexer/indexer.h
	../indexer/indexer.cpp)

target_compile_features(SpiderApp PRIVATE cxx_std_17) 

target_link_libraries(SpiderApp pqxx)

target_link_libraries(SpiderApp OpenSSL::SSL)

target_include_directories(SpiderApp PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(SpiderApp ${Boost_LIBRARIES})