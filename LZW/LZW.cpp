#include <iostream>
#include <math.h> 
#include <map>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

bool readfile(std::string& s, const char* filename, bool mode) //чтение из файла
{
    ifstream fp(filename, mode ? ios::binary : 1);
    if (!fp.is_open())
        return false;

    std::stringstream buffer;
    buffer << fp.rdbuf();
    s = buffer.str();
    return (s.length() > 0);
}

string get_key(const std::string& val, const std::unordered_map<std::string, std::string>& dict) {
    for (const auto& pair : dict)
    {
        const std::string& key = pair.first;
        const std::string& value = pair.second;
        if ((val.size() > value.size() && val == std::string(val.size() - value.size(), '0') + value) || (val == value))
        {
            return key;
        }
    }
    return ""; // Return empty string if key is not found
}



vector <int> int_to_bits(int value, int bits_amount)
{
    vector <int> result;
    for (int n = 0; n < bits_amount; ++n) {
        result.push_back((value & (1 << (bits_amount - 1 - n))) ? 1 : 0);
    }
    return result;
}

string int_to_strbits(int value, int bits_amount) {
    vector<int> bits = int_to_bits(value, bits_amount);
    string result;
    for (int bit : bits) {
        result += to_string(bit);
    }
    return result;
}

unordered_map <string, string> base_dict;

void Encode(string uncompressed)
{
    cout << "Encoding..." << endl;

    vector<char> array_for_dict;
    for (char c : uncompressed) {
        if (find(array_for_dict.begin(), array_for_dict.end(), c) == array_for_dict.end())
        {
            array_for_dict.push_back(c);
        }
    }
    int bits_amount = static_cast<int>(ceil(log2(array_for_dict.size())));
    int code_bits = bits_amount;

    //unordered_map<string, string> base_dict;
    unordered_map<string, string> extended_dict;

    ofstream dict_file("Dictionary.txt");
    for (size_t i = 0; i < array_for_dict.size(); ++i)
    {
        string value = int_to_strbits(i, bits_amount);
        base_dict[string(1, array_for_dict[i])] = value;
        extended_dict[string(1, array_for_dict[i])] = value;
        dict_file << array_for_dict[i] << " - " << value << "\n";
    }

    int dict_size = base_dict.size();
    string series = string(1, uncompressed[0]);
    vector<string> compressed;

    for (size_t i = 1; i < uncompressed.length(); ++i) {
        string combo = series + uncompressed[i];
        if (extended_dict.find(combo) != extended_dict.end()) {
            series = combo;
        }
        else {
            for (char c : string(code_bits - extended_dict[series].length(), '0') + extended_dict[series]) {
                compressed.push_back(string(1, c));
            }

            if (dict_size >= pow(2, bits_amount)) {
                bits_amount += 1;
            }

            extended_dict[combo] = int_to_strbits(dict_size, bits_amount);
            dict_size += 1;
            dict_file << combo << " - " << extended_dict[combo] << "\n";

            if (bits_amount > code_bits) {
                code_bits += 1;
            }

            series = std::string(1, uncompressed[i]);
        }
    }

    dict_file.close();

    if (!series.empty())
    {
        for (char c : string(code_bits - extended_dict[series].length(), '0') + extended_dict[series])
        {
            compressed.push_back(string(1, c));
        }
    }
    ofstream compressed_file("Compressed.txt", std::ios::binary);
    while (compressed.size() >= 8) {
        std::string byte_str = "";
        for (int i = 0; i < 8; ++i) {
            byte_str += compressed[0];
            compressed.erase(compressed.begin());
        }

        char byte = static_cast<char>(std::stoi(byte_str, nullptr, 2));
        compressed_file.write(&byte, 1);
    }

    if (!compressed.empty()) {
        std::string last_char = "";
        for (const std::string& s : compressed) {
            last_char += s;
        }

        last_char += std::string(8 - last_char.length(), '0');

        char byte = static_cast<char>(std::stoi(last_char, nullptr, 2));
        compressed_file.write(&byte, 1);
    }

    compressed_file.close();
}

void Decode(string compressed)
{
    cout << "Decoding..." << endl;

    unordered_map<string, string> extended_dict;
    int dict_size = 0;

    for (const auto& pair : base_dict)
    {
        extended_dict[pair.first] = pair.second;
        dict_size += 1;
    }
    int bits_amount = static_cast<int>(ceil(log2(dict_size)));
    std::vector<int> binary_code;
    for (char c : compressed)
    {
        for (int n = 0; n < 8; ++n)
        {
            binary_code.push_back((c & (1 << (8 - 1 - n))) ? 1 : 0);
        }
    }
    std::string series;
    for (int i = 0; i < bits_amount; ++i) {
        series += std::to_string(binary_code[0]);
        binary_code.erase(binary_code.begin());
    }

    std::string char_str = get_key(series, extended_dict);
    std::string uncompressed = char_str;

    std::ofstream f("Uncompressed.txt");

    while (binary_code.size() >= bits_amount) {
        std::string new_code = "";
        if (dict_size >= pow(2, bits_amount)) {
            bits_amount += 1;
        }

        for (int i = 0; i < bits_amount; ++i) {
            new_code += std::to_string(binary_code[0]);
            binary_code.erase(binary_code.begin());
        }

        if (!get_key(new_code, extended_dict).empty() && extended_dict.find(series + get_key(new_code, extended_dict)) != extended_dict.end()) {
            char_str = get_key(new_code, extended_dict);
            series += char_str;
            continue;
        }
        else {
            if (get_key(new_code, extended_dict).empty() && std::stoi(new_code, nullptr, 2) == dict_size) {
                extended_dict[get_key(series, extended_dict) + char_str[0]] = int_to_strbits(dict_size, bits_amount);
            }
            else {
                char_str = get_key(new_code, extended_dict);
                extended_dict[get_key(series, extended_dict) + char_str[0]] = int_to_strbits(dict_size, bits_amount);
            }

            dict_size += 1;
            f << get_key(series, extended_dict);
            series = new_code;
        }
    }

    f << get_key(series, extended_dict);
    f.close();

    cout << "Done" << endl;
}

void main()
{
    cout << "File text.txt:\n\n";
    string uncompressed;
    string compressed;
    if (readfile(uncompressed, "Test.txt", false))
        cout << uncompressed << "\n" << endl;
    Encode(uncompressed);
    readfile(compressed, "Compressed.txt", true);
    Decode(compressed);
}