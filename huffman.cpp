#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <cstring>
struct Node
{
    char ch;
    int freq;
    struct Node *left, *right;
};

class MinHeapCompare
{
public:
    bool operator()(struct Node *left, struct Node *right)
    {
        return left->freq > right->freq;
    }
};

std::ostream &operator<<(std::ostream &ofs, std::unordered_map<char, std::string> const &umap)
{
    for (auto i : umap)
    {
        ofs << i.first;
        ofs << i.second;
        ofs << '\v';
    }
    ofs << '\r';
    return ofs;
}

std::istream &operator>>(std::istream &ifs, std::unordered_map<char, std::string> &umap)
{
    char c, ch;
    std::string code = "";
    bool isString = false;
    while (!ifs.eof())
    {
        ifs >> c;
        if (c == '\v')
        {
            isString = !isString;
            if (code != "")
            {
                umap[ch] = code;
            }
            code = "";
        }
        if (isString)
        {
            code += c;
        }
        else
        {
            ch = c;
        }
    }
    return ifs;
}

void encode(struct Node *root, std::unordered_map<char, std::string> &huffmanTable, std::string code)
{
    if (root == nullptr)
    {
        return;
    }
    if (root->ch != '\r')
    {
        huffmanTable[root->ch] = code;
    }
    encode(root->left, huffmanTable, code + '0');
    encode(root->right, huffmanTable, code + '1');
}

std::unordered_map<char, std::string> createHuffmanTree(std::priority_queue<struct Node *, std::vector<struct Node *>, MinHeapCompare> pq)
{
    std::string code = "";
    std::unordered_map<char, std::string> huffmanTable;

    while (pq.size() != 1)
    {
        struct Node *left = pq.top();
        pq.pop();
        struct Node *right = pq.top();
        pq.pop();
        int sum = left->freq + right->freq;
        struct Node *ptr = (struct Node *)malloc(sizeof(struct Node));
        ptr->ch = '\r';
        ptr->freq = sum;
        ptr->left = left;
        ptr->right = right;
        pq.push(ptr);
    }

    struct Node *root = pq.top();
    encode(root, huffmanTable, code);

    return huffmanTable;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: huffman <file_name> <operation>\n";
        std::cout << "Operation:\n\tencode: Encodes the file and attaches the Huffman Table at the end of the file.";
        std::cout << "\n\tdecode: Decodes the file.";
        return 1;
    }

    std::string fileName = argv[1];

    if (strcmp(argv[2], "encode") == 0)
    {
        std::ifstream ifs(fileName, std::ios::binary);

        if (!ifs)
        {
            std::cerr << "Error! File not found.";
            return 1;
        }

        ifs.seekg(0, std::ios::end);
        std::ifstream::pos_type filesize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        std::vector<char> bytes(filesize);

        ifs.read(&bytes[0], filesize);

        std::unordered_map<char, int> dictionary;
        for (char c : bytes)
        {
            if (c != '\r')
            {
                dictionary[c]++;
            }
        }

        // Creating min heap
        std::priority_queue<struct Node *, std::vector<struct Node *>, MinHeapCompare> pq;

        for (auto i : dictionary)
        {
            struct Node *node = (struct Node *)malloc(sizeof(struct Node));
            node->left = NULL;
            node->right = NULL;
            node->ch = i.first;
            node->freq = i.second;
            pq.push(node);
        }

        std::unordered_map<char, std::string> huffmanTable = createHuffmanTree(pq);

        std::unordered_map<char, std::vector<char>> trueHuffmanTable; // Checking if vector<char> takes less space than string...
                                                                      // It does not!

        for (auto i : huffmanTable)
        {
            std::vector<char> temp;
            for (char c : i.second)
            {
                temp.push_back(c);
            }
            trueHuffmanTable[i.first] = temp;
        }

        std::ofstream ofs(fileName, std::ios::binary);

        for (auto i : bytes)
        {
            if (i != '\r')
            {
                std::vector<char> code = trueHuffmanTable[i];

                for (char c : code)
                {
                    ofs << c;
                }
            }
        }

        ofs << '\v';
        ofs << huffmanTable;
        //     std::cout << huffmanTableSize;

        ifs.close();
        ofs.close();

        std::cout << "Encoding successful!\n";
    }
    else if (strcmp(argv[2], "decode") == 0)
    {
        // Decode

        //  Inverted Huffman Table
        /////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
        std::unordered_map<std::string, char> invHuffmanTable;

        std::ifstream iifs(fileName, std::ios::binary);

        if (!iifs)
        {
            std::cerr << "\nError loading encoded file!";
        }

        iifs.seekg(0, std::ios::end);
        std::ifstream::pos_type filesizeEncode = iifs.tellg();
        iifs.seekg(0, std::ios::beg);

        std::vector<char> fullEncodedBytes(filesizeEncode);

        iifs.read(&fullEncodedBytes[0], filesizeEncode);

        std::vector<char> encodedBytes;

        // Extract just the encoded bits part
        int counter = 0;
        while (fullEncodedBytes[counter] != '\v')
        {
            encodedBytes.push_back(fullEncodedBytes[counter]);
            counter++;
        }

        // Extract Huffman Table
        counter++;
        char key;
        std::string value = "";
        std::unordered_map<char, std::string> extractedHuffmanTable;
        while (1)
        {
            if (fullEncodedBytes[counter] == '\r')
            {
                break;
            }
            key = fullEncodedBytes[counter];
            counter++;
            while (fullEncodedBytes[counter] != '\v')
            {
                value += fullEncodedBytes[counter];
                counter++;
            }
            counter++;
            extractedHuffmanTable[key] = value;
            value = "";
        }

        for (auto i : extractedHuffmanTable)
        {
            invHuffmanTable[i.second] = i.first;
        }

        std::ofstream oofs(fileName, std::ios::binary);

        std::string decode = "";
        std::string decodedText = "";
        for (auto i : encodedBytes)
        {
            decode += i;
            if (invHuffmanTable[decode] != '\0')
            {
                decodedText += invHuffmanTable[decode];
                oofs << invHuffmanTable[decode];
                decode = "";
            }
        }

        std::cout << "Decoding successful!\n";
        iifs.close();
        oofs.close();
    }
    return 0;
}