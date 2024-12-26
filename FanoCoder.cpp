#include "FanoCoder.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdint>


using namespace std;

bool FanoCoder::compareNodes(const Node &a, const Node &b) {
    return a.frequency > b.frequency;
}

void FanoCoder::buildFano(vector<Node> &nodes, int start, int end) {
    if (start >= end) return;

    int total = 0;
    for (int i = start; i <= end; i++) total += nodes[i].frequency;

    int half = 0, split = start;
    for (int i = start; i <= end; i++) {
        if (half + nodes[i].frequency > total / 2) break;
        half += nodes[i].frequency;
        split = i;
    }

    for (int i = start; i <= split; i++) nodes[i].code += "0";
    for (int i = split + 1; i <= end; i++) nodes[i].code += "1";

    buildFano(nodes, start, split);
    buildFano(nodes, split + 1, end);
}

map<char, string> FanoCoder::buildEncodingTable(const string &input) {
    map<char, int> frequency;
    for (char c : input) frequency[c]++;

    vector<Node> nodes;
    for (const auto &[symbol, freq] : frequency) nodes.push_back({symbol, freq, ""});

    sort(nodes.begin(), nodes.end(), compareNodes);
    buildFano(nodes, 0, nodes.size() - 1);

    map<char, string> encodingTable;
    for (const auto &node : nodes) encodingTable[node.symbol] = node.code;
    return encodingTable;
}

string FanoCoder::encode(const string &input, const map<char, string> &encodingTable) {
    string encoded;
    for (char c : input) encoded += encodingTable.at(c);
    return encoded;
}

string FanoCoder::decode(const string &encoded, const map<char, string> &encodingTable) {
    map<string, char> reverseTable;
    for (const auto &[symbol, code] : encodingTable) reverseTable[code] = symbol;

    string decoded, buffer;
    for (char c : encoded) {
        buffer += c;
        if (reverseTable.count(buffer)) {
            decoded += reverseTable[buffer];
            buffer.clear();
        }
    }
    return decoded;
}

void FanoCoder::saveToFile(const string &filename, const string &data) {
    ofstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Ошибка: не удалось открыть файл для записи: " + filename);
    }
    file.write(data.c_str(), data.size());
    file.close();
    cout << "Файл успешно сохранён: " << filename << endl;
}

string FanoCoder::loadFromFile(const string &filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Ошибка: не удалось открыть файл для чтения: " + filename);
    }
    string data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    cout << "Файл успешно загружен: " << filename << endl;
    return data;
}

void FanoCoder::compress(const std::string &inputFilename, const std::string &outputFilename) {
    std::string input = loadFromFile(inputFilename);
    auto encodingTable = buildEncodingTable(input);
    std::string encoded = encode(input, encodingTable);
    std::string hash = calculateMD5(input);

    std::ofstream file(outputFilename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Ошибка: не удалось открыть файл для записи: " + outputFilename);
    }

    size_t tableSize = encodingTable.size();
    file.write(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));
    for (const auto &[symbol, code] : encodingTable) {
        file.write(&symbol, sizeof(symbol));
        size_t codeLength = code.length();
        file.write(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));
        file.write(code.c_str(), codeLength);
    }
    file.write(encoded.c_str(), encoded.size());
    file.write(hash.c_str(), hash.size());

    file.close();
    std::cout << "Файл успешно заархивирован в " << outputFilename << std::endl;
}


void FanoCoder::decompress(const std::string &originalFileName, const std::string &inputFilename, const std::string &outputFilename) {
    std::ifstream file(inputFilename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Ошибка: не удалось открыть файл для чтения: " + inputFilename);
    }

    size_t tableSize;
    file.read(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));
    std::map<char, std::string> encodingTable;
    for (size_t i = 0; i < tableSize; i++) {
        char symbol;
        file.read(&symbol, sizeof(symbol));
        size_t codeLength;
        file.read(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));
        std::string code(codeLength, ' ');
        file.read(&code[0], codeLength);
        encodingTable[symbol] = code;
    }

    std::string encoded((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string hash = encoded.substr(encoded.size() - 32);
    encoded = encoded.substr(0, encoded.size() - 32);

    file.close();

    std::string decoded = decode(encoded, encodingTable);
    std::string original = loadFromFile(originalFileName);
    
    verifyIntegrity(original, decoded);

    saveToFile(outputFilename, decoded);
    std::cout << "Файл успешно разархивирован в " << outputFilename << std::endl;
}

std::string FanoCoder::calculateMD5(const std::string &data) {
    uint32_t s[] = {7, 12, 17, 22, 5, 9, 14, 20, 4, 11, 16, 23, 6, 10, 15, 21};
    uint32_t K[64];
    for (int i = 0; i < 64; i++) K[i] = (uint32_t)(abs(sin(i + 1)) * pow(2, 32));

    uint32_t a0 = 0x67452301, b0 = 0xEFCDAB89, c0 = 0x98BADCFE, d0 = 0x10325476;
    uint64_t bitlen = data.size() * 8;

    std::vector<uint8_t> msg(data.begin(), data.end());
    msg.push_back(0x80);

    while ((msg.size() * 8) % 512 != 448) msg.push_back(0);
    for (int i = 0; i < 8; i++) msg.push_back((bitlen >> (i * 8)) & 0xFF);

    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t M[16];
        for (int i = 0; i < 16; i++) {
            M[i] = 0;
            for (int j = 0; j < 4; j++) M[i] |= msg[offset + i * 4 + j] << (j * 8);
        }

        uint32_t A = a0, B = b0, C = c0, D = d0;
        for (int i = 0; i < 64; i++) {
            uint32_t F, g;
            if (i < 16) {
                F = (B & C) | (~B & D);
                g = i;
            } else if (i < 32) {
                F = (D & B) | (~D & C);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                F = B ^ C ^ D;
                g = (3 * i + 5) % 16;
            } else {
                F = C ^ (B | ~D);
                g = (7 * i) % 16;
            }
            uint32_t temp = D;
            D = C;
            C = B;
            B = B + ((A + F + K[i] + M[g]) << s[i % 4 + (i / 16) * 4]) | ((A + F + K[i] + M[g]) >> (32 - s[i % 4 + (i / 16) * 4]));
            A = temp;
        }
        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }

    std::ostringstream result;
    for (uint32_t v : {a0, b0, c0, d0}) result << std::hex << std::setw(8) << std::setfill('0') << v;
    return result.str();
}

void FanoCoder::verifyIntegrity(const std::string &original, const std::string &decompressed) {
    std::string originalHash = calculateMD5(original);
    std::string decompressedHash = calculateMD5(decompressed);

    if (originalHash != decompressedHash) {
        throw std::runtime_error("Ошибка: контрольные суммы не совпадают. Файл поврежден.");
    }

    std::cout << "Контрольные суммы совпадают. Целостность данных подтверждена." << std::endl;
}
