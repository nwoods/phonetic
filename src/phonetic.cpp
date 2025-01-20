#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#endif

#include "phonetic.hpp"
#include "convenience.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <functional>

Phonetic::Phonetic() : m_dictionary(import_dictionary()), m_phoneme_idx(generate_idx(m_dictionary)) {
}

Phonetic::Dict_t Phonetic::import_dictionary() {

    std::string file_path = CMU_DICT_PATH;

    #ifdef __EMSCRIPTEN__
    file_path = "/data/cmudict-0.7b";
    #endif

    Dict_t dictionary;

    std::ifstream cmudict{file_path};
    if (!cmudict.is_open()) {
        std::cerr << "Failed to open the dictionary." << '\n';
        return {};
    }
    std::string line;
    while (std::getline(cmudict, line)) {
        // ignore leading ;
        if(line.empty() || line[0] == ';') {
            continue;
        }

        std::istringstream iss(line);

        // extract word up to white space
        std::string word;
        iss >> word;

        // strip variation "(n)", used in CMU DICT for multiple entries of same word
        // instead each pronunciation is added to the vector dictionary[word]
        if (word.back() == ')'){
            word.pop_back();
            word.pop_back();
            word.pop_back();
        }

        // pronunciation is ARPABET symbols, separated by spaces
        // vowels end with a number indicating stress, 0 no stress, 1 primary stress, 2 secondary stress
        std::string pronunciation;
        std::getline(iss, pronunciation);

        // trim white space
        ltrim(pronunciation);
        rtrim(pronunciation);

        dictionary[word].push_back(pronunciation);
    }

    cmudict.close();
    return dictionary;
}

Phonetic::Idx_t Phonetic::generate_idx(const Phonetic::Dict_t& dictionary)
{
    Idx_t phoneme_idx;
    std::vector<std::string> tokens;
    for(const auto& dict_entry : dictionary)
    {
        const auto& word = dict_entry.first;
        const auto& pronunciations = dict_entry.second;
        for(const auto& pronunciation : pronunciations)
        {
            tokenize_inplace(pronunciation, tokens);

            for(size_t i = 0; i < tokens.size(); ++i)
            {
                phoneme_idx[tokens[i]].insert(&dict_entry);
                if(i == 0)
                {
                    phoneme_idx["^" + tokens[i]].insert(&dict_entry);
                    if(tokens.size() > 1)
                    {
                        phoneme_idx["^" + tokens[i] + " " + tokens[i + 1]].insert(&dict_entry);
                    }
                }
                if(i == (tokens.size() - 1))
                {
                    phoneme_idx[tokens[i] + "$"].insert(&dict_entry);
                }
                else
                {
                    phoneme_idx[tokens[i] + " " + tokens[i + 1]].insert(&dict_entry);

                    if(tokens.size() > 1)
                    {
                        if(i == (tokens.size() - 2))
                        {
                            phoneme_idx[tokens[i] + " " + tokens[i + 1] + "$"].insert(&dict_entry);
                        }
                        else
                        {
                            phoneme_idx[tokens[i] + " " + tokens[i + 1] + tokens[i + 2]].insert(&dict_entry);
                        }
                    }
                }
            }

            tokens.clear();
        }
    }

    return phoneme_idx;
}

std::vector<std::string> Phonetic::word_to_phones(std::string word) const {
    // capitalize all queries
    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
    auto it = m_dictionary.find(word);
    if (it != m_dictionary.end()) {
        return it->second;
    }
    else {
        throw std::runtime_error(word + " not found in dictionary.");
    }
}

std::vector<std::pair<std::vector<std::string>, bool>> Phonetic::text_to_phones(const std::string & text) const {

    std::vector<std::pair<std::vector<std::string>, bool>> results{};
    std::vector<std::string> words {strip_punctuation(text)};

    for (const auto & w : words) {
        try {
            std::vector<std::string> phones{word_to_phones(w)};
            results.emplace_back(phones, true);
        } catch (const std::exception &) {
            std::vector<std::string> word_searched_for{};
            word_searched_for.emplace_back(w);
            results.emplace_back(word_searched_for, false);
        }
    }

    return results;
}


std::string Phonetic::phone_to_stress(const std::string& phones) const {
    std::string stresses{};
    for (const auto & c : phones){
        if (c == '0' || c == '1' || c == '2') {
            stresses.push_back(c);
        }
    }
    return stresses;
}

std::vector<std::string> Phonetic::word_to_stresses(const std::string& word) const {
    std::vector<std::string> stresses{};

    std::vector<std::string> phones{word_to_phones(word)};
    for (const auto & p : phones) {
        stresses.emplace_back(phone_to_stress(p));
    }
    return stresses;
}

int Phonetic::phone_to_syllable_count(const std::string& phones) const {
    return static_cast<int>(phone_to_stress(phones).length());
}

std::vector<int> Phonetic::word_to_syllable_counts(const std::string& word) const {
    std::vector<int> syllables;
    std::vector<std::string> phones{word_to_phones(word)};
    for(const auto & p : phones) {
        syllables.emplace_back(phone_to_syllable_count(p));
    }
    return syllables;
}

std::string Phonetic::get_rhyming_part(const std::string& phones) const {
    std::string result{};
    // if we were using C++23 we could use std::ranges::find_last_if, but we're not

    // get a reverse iterator pointing at the number in the last stressed vowel
    auto r_it {std::find_if(phones.rbegin(), phones.rend(), [](char c) {
        return std::isdigit(c) && c != '0';
    })};
    // if we found it
    if (r_it != phones.rend()) {
        // move it up to the first char of our vowel
        r_it += 2;
        // turn it around
        // base moves us one to the right, so we got to step it back
        auto f_it = r_it.base() - 1;
        std::copy(f_it, phones.end(), std::back_inserter(result));
        return result;
    }
    // but if the word has no stresses, we'll return from just the last vowel onward
    auto no_stress_r_it {std::find_if(phones.rbegin(), phones.rend(), [](char c) {
        return std::isdigit(c);
    })};

    if (no_stress_r_it != phones.rend()) {
        // move it up to the first char of our vowel
        no_stress_r_it += 2;
        // turn it around
        // base moves us one to the right, so we got to step it back
        auto f_it = no_stress_r_it.base() - 1;
        std::copy(f_it, phones.end(), std::back_inserter(result));
        return result;
    }

    // else, there are no vowels at all, so we return an empty string
    return result;
}


std::unordered_set<std::string> Phonetic::search(const std::string& pattern, const std::vector<std::string>& contains) const
{
    if(pattern.empty()) return {};

    std::vector<std::string> phoneme_runs(contains.begin(), contains.end());
    if(contains.empty())
    {
        // regex could be improved but should give enough index entries to hit
        static const std::regex run_pattern = std::regex("((?:(?:^\\^)|(?:\\b" + phoneme_pattern() + " ))?" + phoneme_pattern() + "(?:(?:\\$$)|(?: " + phoneme_pattern() + "\\b))?)");

        std::sregex_iterator it(pattern.begin(), pattern.end(), run_pattern);
        auto the_end = std::sregex_iterator();
        size_t n_runs = 0;
        for(; it != the_end && n_runs < 5; ++it)
        {
            phoneme_runs.push_back(it->str());
            ++n_runs;
        }

        if(phoneme_runs.empty())
        {
            throw std::runtime_error("Could not extract phoneme runs from pattern \"" + pattern + "\". Please provide one or more manually.");
        }
    }

    auto it_idx = m_phoneme_idx.find(phoneme_runs[0]);
    if(it_idx == m_phoneme_idx.end())
    {
        return {};
    }
    auto candidates = it_idx->second;

    for(size_t i = 1; i < phoneme_runs.size(); ++i)
    {
        it_idx = m_phoneme_idx.find(phoneme_runs[i]);
        if(it_idx == m_phoneme_idx.end())
        {
            return {};
        }

        // TODO when we move to C++20, replace the following with std::erase_if
        auto it = candidates.begin();
        while(it != candidates.end())
        {
            if(!it_idx->second.count(*it))
            {
                it = candidates.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if(candidates.empty()) return {};
    }

    std::unordered_set<std::string> out;
    std::regex re(pattern);
    for(const auto& cand : candidates)
    {
        if(std::any_of(cand->second.begin(), cand->second.end(), [&](const std::string& x){return std::regex_search(x, re);}))
        {
            out.insert(cand->first);
        }
    }

    return out;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(my_module) {

    emscripten::register_vector<std::string>("StringVector");


    emscripten::class_<Phonetic>("Phonetic")
        .constructor<>()
        .function("word_to_phones", &Phonetic::word_to_phones)
        .function("text_to_phones", &Phonetic::text_to_phones)
        .function("phone_to_stress", &Phonetic::phone_to_stress)
        .function("word_to_stresses", &Phonetic::word_to_stresses)
        .function("phone_to_syllable_count", &Phonetic::phone_to_syllable_count)
        .function("word_to_syllable_counts", &Phonetic::word_to_syllable_counts)
        .function("get_rhyming_part", &Phonetic::get_rhyming_part)
        ;
}
#endif
