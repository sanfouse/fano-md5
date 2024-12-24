#ifndef FANOCODER_H
#define FANOCODER_H

#include <string>
#include <map>
#include <vector>

class FanoCoder {
public:
    void compress(const std::string &inputFilename, const std::string &outputFilename);
    void decompress(const std::string &inputFilename, const std::string &outputFilename);

private:
    struct Node {
        char symbol;
        int frequency;
        std::string code;
    };

    static bool compareNodes(const Node &a, const Node &b);

    void buildFano(std::vector<Node> &nodes, int start, int end);
    std::map<char, std::string> buildEncodingTable(const std::string &input);
    std::string encode(const std::string &input, const std::map<char, std::string> &encodingTable);
    std::string decode(const std::string &encoded, const std::map<char, std::string> &encodingTable);
    void saveToFile(const std::string &filename, const std::string &data);
    std::string loadFromFile(const std::string &filename);
    
    std::string calculateMD5(const std::string &data);
    void verifyIntegrity(const std::string &original, const std::string &decompressed);
};

#endif // FANOCODER_H
