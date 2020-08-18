all: format
	$(CXX) -std=c++17 client.cpp -pthread -o client
clean:
	rm -rf client
	rm -rf release
	rm -rf release.zip
format:
	clang-format -i *.{cpp,h}
release:
	mkdir release
	cp -rf pcsx2_ipc.h release/
	mkdir -p release/example
	cp -rf Makefile windows-qt.pro client.cpp pcsx2_ipc.h release/example/
	doxygen
	mkdir -p release/docs
	cp -rf html release/docs
	cd latex && make
	cd ..
	cp -rf latex/refman.pdf release/docs
	zip -r release.zip release
