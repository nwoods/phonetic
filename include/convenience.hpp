#pragma once

#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include <unordered_set>


// trim white space from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim white space from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// strip punctuation from a text, returning a vector of words in order
inline std::vector<std::string> strip_punctuation(const std::string& text){
   std::istringstream iss(text);
   std::vector<std::string> output{};
   std::string word{};
   while (iss >> word) {
      std::regex word_regex("[a-zA-Z]+([-']?[a-zA-Z]+)*");

      for (std::sregex_iterator it{word.begin(), word.end(), word_regex}, end{}; it != end; ++it) {
         std::smatch match = *it;
         std::string match_str = match.str();
         // put it in lower case
         output.emplace_back(match_str);
      }
   }
   return output;
}

inline void tokenize_inplace(const std::string& s, std::vector<std::string>& out, char delim=' ')
{
    std::stringstream ss(s);
    std::string word;
    while(!ss.eof())
    {
        getline(ss, word, delim);
        out.push_back(word);
    }
}

inline std::vector<std::string> tokenize(const std::string& s, char delim=' ')
{
    std::vector<std::string> out;
    tokenize_inplace(s, out, delim);
    return out;
}

namespace
{
static const std::unordered_set<std::string> valid_phonemes = {
    "AA",
    "AE",
    "AH",
    "AO",
    "AW",
    "AY",
    "AA0",
    "AE0",
    "AH0",
    "AO0",
    "AW0",
    "AY0",
    "AA1",
    "AE1",
    "AH1",
    "AO1",
    "AW1",
    "AY1",
    "AA2",
    "AE2",
    "AH2",
    "AO2",
    "AW2",
    "AY2",
    "B",
    "CH",
    "D",
    "DH",
    "EH",
    "ER",
    "EY",
    "EH0",
    "ER0",
    "EY0",
    "EH1",
    "ER1",
    "EY1",
    "EH2",
    "ER2",
    "EY2",
    "F",
    "G",
    "HH",
    "IH",
    "IY",
    "IH0",
    "IY0",
    "IH1",
    "IY1",
    "IH2",
    "IY2",
    "JH",
    "K",
    "L",
    "M",
    "N",
    "NG",
    "OW",
    "OY",
    "OW0",
    "OY0",
    "OW1",
    "OY1",
    "OW2",
    "OY2",
    "P",
    "R",
    "S",
    "SH",
    "T",
    "TH",
    "UH",
    "UW",
    "UH0",
    "UW0",
    "UH1",
    "UW1",
    "UH2",
    "UW2",
    "V",
    "W",
    "Y",
    "Z",
    "ZH"
};
} // anonymous namespace

inline bool is_phoneme(const std::string& s)
{
    if(s.size() == 0 || s.size() > 3) return false;

    return bool(::valid_phonemes.count(s));
}

inline std::string phoneme_pattern()
{
    std::stringstream ss;

    ss << "(?:(?:";
    bool first = true;
    for(const auto& ph : ::valid_phonemes)
    {
        if(first)
        {
            first = false;
        }
        else
        {
            ss << ")|(?:";
        }
        ss << ph;
    }
    ss << "))";

    return ss.str();
}
