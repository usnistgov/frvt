#pragma once

#include <string>
#include <iostream>
#include <fstream>

// Read file to buffer
class BufferFile {
public:
	std::string file_path_;
	int length_;
	char* buffer_;

	explicit BufferFile(std::string file_path)
		:file_path_(file_path)
	{

		std::ifstream ifs(file_path.c_str(), std::ios::in | std::ios::binary);
		if (!ifs) {
			std::cerr << "Can't open the file. Please check " << file_path << ". \n";
			length_ = 0;
			buffer_ = NULL;
			return;
		}

		ifs.seekg(0, std::ios::end);
		length_ = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		std::cout << file_path.c_str() << " ... " << length_ << " bytes\n";

		buffer_ = new char[sizeof(char) * length_];
		ifs.read(buffer_, length_);
		ifs.close();
	}

	int GetLength() {
		return length_;
	}
	char* GetBuffer() {
		return buffer_;
	}

	~BufferFile() {
		if (buffer_) {
			delete[] buffer_;
			buffer_ = NULL;
		}
	}
};
