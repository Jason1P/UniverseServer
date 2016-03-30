#include "LVLFile.h"
#include "Logger.h"
#include "UtfConverter.h"

LVLFile::LVLFile(std::string filename, std::string scenename, unsigned long version, unsigned long worldID) {
	std::vector<unsigned char> content = OpenPacket(filename);

	if (content.size() > 0) {
		RakNet::BitStream file;
		for (int i = 0; i < content.size(); i++) {
			file.Write(content.at(i));
		}

		unsigned long long u64;
		unsigned long u32;
		unsigned short u16;
		unsigned char u8;
		float f;

		while ((file.GetNumberOfUnreadBits() / 8) > 0) {
			std::string cur = "";

			for (int i = 0; i < 4; i++) {
				char buf;
				file.Read(buf);
				cur += buf;
			}

			if (cur == "CHNK") {
				unsigned long type;
				file.Read(type);

				if (type == 2001) {
					file.Read(u16);
					file.Read(u16);
					file.Read(u32);
					file.Read(u32);

					while (((file.GetReadOffset() / 8) % 16) != 0) {
						file.Read(u8);
					}

					unsigned long object_amount;
					file.Read(object_amount);

					for (int i = 0; i < object_amount; i++) {
						WorldObjectInfo info;

						info.zone = worldID;

						long LOT;

						file.Read(u64);
						file.Read(LOT);

						if (version >= 0x26)
							file.Read(u32);

						if (version >= 0x20)
							file.Read(u32);

						file.Read(info.posX);
						file.Read(info.posY);
						file.Read(info.posZ);

						file.Read(info.rotW);
						file.Read(info.rotX);
						file.Read(info.rotY);
						file.Read(info.rotZ);

						file.Read(f);

						unsigned long len;
						file.Read(len);

						std::wstring ldfData = L"";
						for (int k = 0; k < len; k++) {
							wchar_t buf;
							file.Read(buf);
							ldfData += buf;
						}

						if (version >= 7)
							file.Read(u32);

						std::istringstream stream(UtfConverter::ToUtf8(ldfData));
						std::string line;
						while (std::getline(stream, line)) {
							if (startsWith(line, "spawntemplate=1:")) {
								LOT = std::stoul(removeLeft(UtfConverter::FromUtf8(line), std::strlen("spawntemplate=1:")));
								if (LOT != -1) {
									info.LOT = LOT;
									objects.push_back(info);
									Logger::log("DEBUG", "LVLFile", "Saved object information for LOT " + std::to_string(info.LOT) + " from " + scenename);
								}
							}
						}
					}
				}
			}
		}
	}
}

LVLFile::~LVLFile() {}

std::vector<WorldObjectInfo> LVLFile::getObjects() { return objects; }