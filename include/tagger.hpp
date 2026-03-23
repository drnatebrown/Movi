#ifndef TAGGER_HPP
#define TAGGER_HPP

#include "sdsl_wrapper.hpp"
#include <fstream>
#include <string>

class Tagger {
public:
    Tagger() = default;
    ~Tagger() = default;

    /** Load .tag.bv and .tag.heads from base path. Returns true on success. */
    bool load(const std::string& base);
    uint16_t get_tag(uint64_t bwt_start, uint64_t bwt_end);
    std::vector<uint16_t> get_tags(uint64_t bwt_start, uint64_t bwt_end);

    size_t size() const { return N_; }
    size_t numberOf1() const { return rank(N_); }
    size_t tag_heads_size() const { return tag_heads.size(); }

private:
    size_t N_ = 0;
    sdsl::sd_vector<> tags_end_bv;
    sdsl::sd_vector<>::rank_1_type rank;
    sdsl::int_vector<> tag_heads;
};

#endif
