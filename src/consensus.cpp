/**
 * Multi-threaded C++ implementation of consensus.py
 * Groups records by read_id, computes consensus tag per read (merge intervals by tag, pick tag with max evidence; tie-break randomly).
 */

 #include <algorithm>
 #include <fstream>
 #include <iostream>
 #include <mutex>
 #include <numeric>
 #include <random>
 #include <string>
 #include <thread>
 #include <unordered_map>
 #include <vector>
 #include <sstream>
 
 using Interval = std::pair<int64_t, int64_t>;
 
 static int64_t merge_intervals(std::vector<Interval>& intervals) {
     if (intervals.empty()) return 0;
     std::sort(intervals.begin(), intervals.end());
     int64_t total = 0;
     int64_t cur_start = intervals[0].first, cur_end = intervals[0].second;
     for (size_t i = 1; i < intervals.size(); ++i) {
         int64_t start = intervals[i].first, end = intervals[i].second;
         if (start <= cur_end)
             cur_end = std::max(cur_end, end);
         else {
             total += cur_end - cur_start + 1;
             cur_start = start;
             cur_end = end;
         }
     }
     total += cur_end - cur_start + 1;
     return total;
 }
 
 struct Record { int64_t start, end; int tag; };
 
 static int consensus_for_read(const std::vector<Record>& records, std::mt19937& rng) {
     std::unordered_map<int, std::vector<Interval>> by_tag;
     for (const auto& r : records) {
         by_tag[r.tag].emplace_back(r.start, r.end);
     }
     std::unordered_map<int, int64_t> evidence;
     for (auto& p : by_tag)
         evidence[p.first] = merge_intervals(p.second);
 
     int64_t max_ev = 0;
     for (const auto& p : evidence)
         if (p.second > max_ev) max_ev = p.second;
 
     std::vector<int> best_tags;
     for (const auto& p : evidence)
         if (p.second == max_ev) best_tags.push_back(p.first);
 
     if (best_tags.empty()) return 0;
     if (best_tags.size() == 1) return best_tags[0];
     std::uniform_int_distribution<size_t> dist(0, best_tags.size() - 1);
     return best_tags[dist(rng)];
 }
 
 int main(int argc, char** argv) {
     if (argc < 3) {
         std::cerr << "Usage: " << (argv[0] ? argv[0] : "tagger-consensus") << " <input_file> <output_file> [nthreads]\n";
         return 1;
     }
     const std::string input_path(argv[1]), output_path(argv[2]);
     // After: const std::string input_path(argv[1]), output_path(argv[2]);

    unsigned nthreads = std::max(1u, std::thread::hardware_concurrency());
    if (argc >= 4) {
        int j = std::stoi(argv[3]);
        if (j > 0) nthreads = static_cast<unsigned>(j);
    }
 
     // Single-threaded parse: group by read_id
     std::unordered_map<std::string, std::vector<Record>> by_read;
     std::ifstream fin(input_path);
     if (!fin) {
         std::cerr << "Cannot open input: " << input_path << '\n';
         return 1;
     }
     std::string line, read_id_full, start_s, end_s, skip, tag_s;
     while (std::getline(fin, line)) {
         if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
         std::istringstream iss(line);
         if (!(iss >> read_id_full >> start_s >> end_s >> skip >> tag_s)) continue;
         std::string read_id;
         int64_t global_start = 0;
         {
             size_t colon = read_id_full.find(':');
             read_id = read_id_full.substr(0, colon);
             if (colon != std::string::npos) {
                 std::string suffix = read_id_full.substr(colon + 1);
                 size_t dash = suffix.find('-');
                 global_start = std::stoll(suffix.substr(0, dash)) - 1;
             }
         }
         int64_t start = std::stoll(start_s) + global_start;
         int64_t end = std::stoll(end_s) + global_start;
         int tag = std::stoi(tag_s);
         by_read[read_id].push_back({start, end, tag});
     }
     fin.close();
 
     // Sorted read_ids for deterministic output order
     std::vector<std::string> read_ids;
     read_ids.reserve(by_read.size());
     for (const auto& p : by_read) read_ids.push_back(p.first);
     std::sort(read_ids.begin(), read_ids.end());
 
     // Parallel consensus over read_ids
     std::vector<int> results(read_ids.size(), 0);
     auto worker = [&](unsigned thread_id) {
         std::mt19937 rng(thread_id + 1);
         for (size_t i = thread_id; i < read_ids.size(); i += nthreads) {
             const std::string& rid = read_ids[i];
             auto it = by_read.find(rid);
             if (it == by_read.end()) continue;
             results[i] = consensus_for_read(it->second, rng);
         }
     };
     std::vector<std::thread> threads;
     for (unsigned t = 0; t < nthreads; ++t)
         threads.emplace_back(worker, t);
     for (auto& t : threads) t.join();
 
     // Output in sorted read_id order
     std::ofstream fout(output_path);
     if (!fout) {
         std::cerr << "Cannot open output: " << output_path << '\n';
         return 1;
     }
     for (size_t i = 0; i < read_ids.size(); ++i)
         fout << read_ids[i] << '\t' << results[i] << '\n';
     fout.close();
     return 0;
 }