#include "tagger.hpp"

bool Tagger::load(const std::string& base) {
    std::ifstream in(base + ".tag.bv", std::ios::binary);
    if (!in) return false;

    in.read(reinterpret_cast<char*>(&N_), sizeof(N_));
    if (N_ > 0) {
        tags_end_bv.load(in);
        rank = sdsl::sd_vector<>::rank_1_type(&tags_end_bv);
    }

    std::ifstream hin(base + ".tag.heads", std::ios::binary);
    if (!hin) return false;
    tag_heads.load(hin);
    return true;
}

uint16_t Tagger::get_tag(uint64_t bwt_start, uint64_t bwt_end) {
    uint64_t tag_idx = rank(bwt_end);
    return tag_heads[tag_idx];
}

std::vector<uint16_t> Tagger::get_tags(uint64_t bwt_start, uint64_t bwt_end) {
    uint64_t start_tag_idx = rank(bwt_start+1) - 1;
    uint64_t end_tag_idx = rank(bwt_end+1) - 1;
    std::vector<uint16_t> tags(end_tag_idx - start_tag_idx + 1);
    for (uint64_t i = start_tag_idx; i <= end_tag_idx; i++) {
        tags[i - start_tag_idx] = tag_heads[i];
    }
    return tags;
}