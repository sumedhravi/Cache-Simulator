// #include <stdio.h>
// #include <inttypes.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include "cachesim.hpp"
// #include <fstream>
// #include <vector>
// #include <string>
// #include <iostream>
// #include <sstream>
// #include <math.h>

// static void print_help(void);
// // static std::string parse_insert_policy(const char *arg, insert_policy_t *policy_out);
// static int validate_config(sim_config_t *config);
// static void print_cache_config(cache_config_t *cache_config, const char *cache_name);
// static void print_statistics(sim_stats_t* stats);
// static const char *insert_policy_str(insert_policy_t policy);

// void writeCSVFile(const sim_config_t *config, const double time, const std::string filename)
// {
//     std::ofstream outputFile;
//     outputFile.open(filename, std::ofstream::out | std::ofstream::app);
//     outputFile << std::to_string(config->l1_config.c) << ","<< std::to_string(config->l1_config.b) << "," << std::to_string(config->l1_config.s) << ",";
//     outputFile << std::to_string(config->l2_config.c)  << "," << std::to_string(config->l2_config.s) << ",";
//     outputFile << std::to_string(config->victim_cache_entries) <<",";
//     outputFile << insert_policy_str(config->l2_config.insert_policy) <<",";
//     outputFile << std::to_string(time) << std::endl;
 
//     outputFile.close();
// }

// int main(int argc, char **argv) {
//     sim_config_t config = DEFAULT_SIM_CONFIG;
//     int opt;
//     std::ifstream inputFile;
//     inputFile.open(argv[1]);
//     std::vector<std::vector<std::string>> fileData;

//     if(!inputFile)
//     {
//         printf("Cannot open the File");
//         return 1;
//     }

//     std::string line;
    
//     std::vector<std::string>  lineData;
//     // std::vector<std::string> line;
//     std::string value;
    
//     while (std::getline(inputFile, line))
//     {
//         std::stringstream  lineStream(line);
//         lineData.clear();
//         // Read an integer at a time from the line
//         while(lineStream >> value)
//         {
//             // Add the integers from a line to a 1D array (vector)
//             // std::cout<<value<<std::endl;
//             lineData.push_back(value);
//         }
//         // When all the integers have been read, add the 1D array
//         // into a 2D array (as one line in the 2D array)
//         fileData.push_back(lineData);
//     }

//     inputFile.close();

//     std::ofstream outputFile;
//     std::string fileName = std::string(argv[1]);
//     int r_idx = fileName.rfind('.');
//     int l_idx = fileName.find('/');
//     fileName = std::string(&fileName[l_idx+1], &fileName[r_idx]);
//     fileName += ".csv";

//     outputFile.open(fileName, std::ofstream::out | std::ofstream::trunc);
//     outputFile << "L1_C" << ","<< "B" << "," << "L1_S" << "," << "L2_C" << "," << "L2_S" << ","<< "num_victim_cache"<< ","<<"insertion_policy"<<","<<"access_time"<<std::endl;
//     outputFile.close();
//     sim_stats_t stats;

//     for (int b=7; b<=7; b++)
//     {
//         for (int l2c = 17; l2c<=17; l2c++)
//         {
//             for (int l2s= 0; l2s<=l2c-b-1 and l2s<=7; l2s++)
//             { 
//                 for (int l1c =7; l1c<=15 and l1c<l2c; l1c++)
//                 {
//                     for (int l1s= 0; l1s<=l1c-b-1 and l1s<l2s and l1s <=5; l1s++)
//                     {
//                         for (int i =0; i<2; i++)
//                         {
//                         // for (int v = 0; v<=2 and pow(2, b)*v <= 128; v++)
//                         // {
//                             config.l1_config.c = l1c;
//                             config.l2_config.c = l2c;
//                             config.l1_config.b = b;
//                             config.l2_config.b = b;
//                             config.l1_config.s = l1s;
//                             config.l2_config.s = l2s;
//                             config.victim_cache_entries = b == 7 ? 1: 2;
//                             config.l2_config.insert_policy = i==0 ? INSERT_POLICY_LIP: INSERT_POLICY_MIP; 
//                             // printf("Cache Settings\n");
//                             // printf("--------------\n");
//                             // print_cache_config(&config.l1_config, "L1");
//                             // printf("Victim cache entries: %" PRIu64 "\n", config.victim_cache_entries);
//                             // print_cache_config(&config.l2_config, "L2");
//                             // printf("\n");

//                             if (validate_config(&config)) {
//                                 // return 1;
//                                 std::cout << "Validation failed" << std::endl;
//                                 return 1;
//                             }

//                             sim_setup(&config);

//                             // /* Setup statistics */
//                             memset(&stats, 0, sizeof stats);
//                             for (auto line: fileData)
//                             {
//                                 const char* address = line[1].c_str();
//                                 sim_access((line[0][0]), strtoull(address, NULL, 16), &stats);
//                             }


//                             sim_finish(&stats);

//                             // print_statistics(&stats);
//                             writeCSVFile(&config, stats.avg_access_time_l1, fileName);
//                         // }
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     return 0;
// }

// static int validate_config(sim_config_t *config) {
//     if (config->l1_config.b > 7 || config->l1_config.b < 4) {
//         printf("Invalid configuration! The block size must be reasonable: 4 <= B <= 7\n");
//         return 1;
//     }

//     if (!config->l2_config.disabled && config->l1_config.s > config->l2_config.s) {
//         printf("Invalid configuration! L1 associativity must be less than or equal to L2 associativity\n");
//         return 1;
//     }

//     if (!config->l2_config.disabled && config->l1_config.c >= config->l2_config.c) {
//         printf("Invalid configuration! L1 size must be strictly less than L2 size\n");
//         return 1;
//     }

//     if (config->victim_cache_entries > 2) {
//         printf("Invalid configuration! Victim Cache entries must be 0, 1, or 2\n");
//         return 1;
//     }

//     return 0;
// }

// static const char *insert_policy_str(insert_policy_t policy) {
//     switch (policy) {
//         case INSERT_POLICY_MIP: return "MIP";
//         case INSERT_POLICY_LIP: return "LIP";
//         default: return "Unknown policy";
//     }
// }

// static void print_cache_config(cache_config_t *cache_config, const char *cache_name) {
//     printf("%s ", cache_name);
//     if (cache_config->disabled) {
//         printf("disabled\n");
//     } else {
//         printf("(C,B,S): (%" PRIu64 ",%" PRIu64 ",%" PRIu64 "). Insertion policy: %s\n",
//            cache_config->c, cache_config->b, cache_config->s,
//            insert_policy_str(cache_config->insert_policy));
//     }
// }

// // static std::string parse_insert_policy(const char *arg, insert_policy_t *policy_out) {
// //     if (!strcmp(arg, "mip") || !strcmp(arg, "MIP")) {
// //         *policy_out = INSERT_POLICY_MIP;
// //         return "MIP";
// //     } else if (!strcmp(arg, "lip") || !strcmp(arg, "LIP")) {
// //         *policy_out = INSERT_POLICY_LIP;
// //         return "LIP";
// //     } else {
// //         printf("Unknown cache insertion/replacement policy `%s'\n", arg);
// //         return "Unknown";
// //     }
// // }

// static void print_statistics(sim_stats_t* stats) {
//     printf("Cache Statistics\n");
//     printf("----------------\n");
//     printf("Reads: %" PRIu64 "\n", stats->reads);
//     printf("Writes: %" PRIu64 "\n", stats->writes);
//     printf("\n");
//     printf("L1 accesses: %" PRIu64 "\n", stats->accesses_l1);
//     printf("L1 hits: %" PRIu64 "\n", stats->hits_l1);
//     printf("L1 misses: %" PRIu64 "\n", stats->misses_l1);
//     printf("L1 hit ratio: %.3f\n", stats->hit_ratio_l1);
//     printf("L1 miss ratio: %.3f\n", stats->miss_ratio_l1);
//     printf("L1 average access time (AAT): %.3f\n", stats->avg_access_time_l1);
//     printf("\n");
//     printf("Victim Cache hits after L1 miss: %" PRIu64 "\n", stats->hits_victim_cache);
//     printf("Victim Cache misses after L1 miss: %" PRIu64 "\n", stats->misses_victim_cache);
//     printf("Victim Cache hit ratio: %.3f\n", stats->hit_ratio_victim_cache);
//     printf("Victim Cache miss ratio: %.3f\n", stats->miss_ratio_victim_cache);
//     printf("Write-backs from L1 or Victim Cache: %" PRIu64 "\n", stats->write_backs_l1_or_victim_cache);
//     printf("\n");
//     printf("L2 reads: %" PRIu64 "\n", stats->reads_l2);
//     printf("L2 writes: %" PRIu64 "\n", stats->writes_l2);
//     printf("L2 read hits: %" PRIu64 "\n", stats->read_hits_l2);
//     printf("L2 read misses: %" PRIu64 "\n", stats->read_misses_l2);
//     printf("L2 read hit ratio: %.3f\n", stats->read_hit_ratio_l2);
//     printf("L2 read miss ratio: %.3f\n", stats->read_miss_ratio_l2);
//     printf("L2 average access time (AAT): %.3f\n", stats->avg_access_time_l2);
// }
