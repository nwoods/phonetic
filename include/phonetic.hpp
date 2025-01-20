#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <array>
#include<functional>


class Phonetic {
public:
    using Dict_t = std::unordered_map<std::string, std::vector<std::string> >;
    using Idx_t = std::unordered_map<std::string, std::unordered_set<const Dict_t::value_type*> >;
private:
    // map of words with their CMU ARPABET_pronunciations, separated by spaces
    const Dict_t m_dictionary;

    // m_phoneme_idx[some_phonemes] is the list (as an unordered set) of pointers to the dictionary entries for words containing the 1-, 2-, or 3-phoneme run some_phonemes.
    // Multi-phoneme runs are in order (so "hello" is indexed by "HH EH" but not "HH L").
    // The start and end of words are represented by pseudo-phonemes ^ and $ to allow regex-like search, so hello is indexed by "^ HH" and "OW $".
    const Idx_t m_phoneme_idx;

    // assumes filepath "../data/CMUdict/cmudict-0.7b"
    static Dict_t import_dictionary();
    static Idx_t generate_idx(const Dict_t& dictionary);

public:

    Phonetic();

    /**
     * Get array of possible pronunciations from CMUdict.
     *
     * Throws a std::exception if word not found.
     *
     * @param (string) word: English word to look up.
     * @return Vector of strings, of the possible pronunciations recorded in the dictionary, each of which are a string of ARAPBET phones separated by spaces.
    */
    std::vector<std::string> word_to_phones(std::string word) const;

    /**
     * Get an array of possible pronuncitation of each word from a text.
     *
     * If a word is not found, vector will contain the word that was searched for, and bool is marked false.
     *
     * @param text (string): text to look up
     * @return An array of pairs, each pair has 1. array of possible pronunciations, 2. a bool flag indicating if word was found.
    */
    std::vector<std::pair<std::vector<std::string>, bool>> text_to_phones(const std::string & text) const;

     /**
     * Takes a string of space-separated CMUdict phones, returns a string of the stresses.
     *
     * 0 = no stress, 1 = primary stress, 2 = secondary stress.
     *
     * @param (string) phones: space-separated CMUdict phones
     * @return (string): String of stresses
    */
    std::string phone_to_stress(const std::string& phones) const;

    /**
     * Takes an English word and returns a vector of possible stresses.
     *
     * TODO: should do exception handling here as well.
     *
     * @param (string) word: English word
     * @return (vector<string>): vector of strings of stresses
    */
    std::vector<std::string> word_to_stresses(const std::string& word) const;

    /**
     * Take a string of space-separated CMUdict phones, returns number of syllables.
     *
     * @param (string) phones: space-separated CMUdict phones
     * @return (int): number of syllables
    */
    int phone_to_syllable_count(const std::string& phones) const;

    /**
     * Takes an English word, returns a vector of possible stresses.
     *
     * (Most words have a single syllable count even across multiple pronunciations, but this catches the outliers, e.g. "fire".)
     *
     * @param (string) word: English word
     * @return (vector<int>): possible numbers of syllables
    */
    std::vector<int> word_to_syllable_counts(const std::string& word) const;


    /**
     * Get the rhyming part from a string of space-separated phones.
     *
     * "Rhyming part" consists of: the accented vowel closest, to the end of the word.
     *
     * @param phones (string): string of space-separated phones
     * @return (string): a string of space-separated phones
    */
    std::string get_rhyming_part(const std::string& phones) const;


    /**
     * Find dictionary words matching a pattern, from candidates containing 1-3 phoneme runs in containing. Phonemes in these runs must be space-separated, and may contain ^ and $ for beginning/end of words (not space-separated; these count as one of the three phonemes).
     * No validity checking is performed on contains (TODO)
     * TODO it would be better to return the pointers stored in the index, but I don't want to figure out how that interacts with Python yet.
     * If contains is empty or not provided, we try to figure it out from pattern, and throw an exception if we can't.
     *
     * @param pattern (string): Regex to match
     * @param contains (vector<string>, optional): At least one run of 1-3 phonemes (possibly including ^ or $ for beginning/end)
     * @return (unordered_set<string>): words from the dictionary containing all phoneme runs and matching pattern
     *
    */
    std::unordered_set<std::string> search(const std::string& pattern, const std::vector<std::string>& contains={}) const;

    // TODO implement search by stress
};
