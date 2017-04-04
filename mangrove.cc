#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <random>

// #include "./json11/json11.hpp"

namespace Mangrove{

struct Node;

struct Association final{
    int count;
    std::shared_ptr<Node> next_node;
    float probability{0.0};
};

struct Node {
    std::string word;
    std::vector<Association> associations;
    int total{0}; //total forward association freq.
    void Associate(std::shared_ptr<Node> node){
        auto w = node->word;
        auto it = std::find_if( associations.begin(),
                                associations.end(),
                                [&w](const Association& ass){
            return ass.next_node->word == w;
        });

        total += 1;
        if(it==associations.end()){//not found
            auto ass = Association();
            ass.count = 1;
            ass.next_node = node;
            // ass.probability = (double)ass.count/(double)total;
            associations.push_back(ass);
            std::cout << word << "->" << node->word <<"\t" << ass.count<< "\n";
        }else{
            it->count += 1;
            // it->probability = (double)(it->count)/(double)total;
            std::cout << word << "->" << node->word <<"\t" << it->count<< "\n";
        }
    }
};

class Mangrove final{
    std::map<std::string,std::shared_ptr<Node>> node_map;
    std::shared_ptr<Node> current_node{nullptr};
public:
    void Add(const std::string& word,bool new_sentence=false){
        current_node = new_sentence ? nullptr: current_node;
        // std::cout << " Adding " << word << "\n";
        std::shared_ptr<Node> n;
        if(node_map.find(word)==node_map.end()){//not found
            n = std::make_shared<Node>();
            n->word = word;
            node_map[word] = n;
        }else{
            n = node_map[word];
        }

        if(current_node!=nullptr){
            current_node->Associate(n);
        }
        current_node = n;
    }
    
    void ComputeProbabilities(){
        for(const auto& el : node_map){
            auto n = el.second;

            for(int i=0;i<n->associations.size();++i){
                n->associations[i].probability = (double)n->associations[i].count/(double)n->total;
            }

            std::cout <<"-------------------------\n";
            for(int i=0;i<n->associations.size();++i){
                std::cout << n->word << "->" 
                    << n->associations[i].next_node->word 
                    << "|" << n->associations[i].probability
                    << "\n";
            }

            std::sort(n->associations.begin(),n->associations.end(), 
                        [](const Association& a1, const Association& a2){
                            return a1.probability > a2.probability;
                            // return a1.count > a2.count;
                        }
                    );
        }
    }        
    
    std::vector<std::pair<float,std::string>> GetNext(const std::string& word){
        std::vector<std::pair<float,std::string>> results;
        std::shared_ptr<Node> n;
        if(node_map.find(word) != node_map.end()){//not found
            n = node_map[word];
            int counter = 0; int max_results = 3;
            for(const auto& ass : n->associations){
                results.push_back({ass.probability, ass.next_node->word});
                // results.push_back({ass.count, ass.next_node->word});
                if(counter >= max_results){
                    break;
                }
                ++counter;
            }
        }
        return results;
    }
};

}
//------------------------------------------------------------------------------------------
// Trims whitespace from begining and end of a string.
// Does not modify the original string
static inline std::string string_trim(const std::string& s){
    const std::string whitespace = " \t\f\v\n\r";
    std::string ss(s);
    if(ss.empty())
        return ss;
    int start = ss.find_first_not_of(whitespace);
    int end = ss.find_last_not_of(whitespace);
    if(start<0){//whole string is whitespace
      ss.erase(0);
      return ss;
    }
    ss.erase(0,start);
    ss.erase((end - start) + 1);
    return ss;
}
//------------------------------------------------------------------------------------------------------
static inline std::vector<std::string> string_split(const std::string& s, const std::string& delim){
    std::vector<std::string> result;
    if(delim.empty() || s.empty())
        return result;

    std::string::size_type k0 = 0;
    std::string::size_type k1 = s.find(delim);
    if(k1==std::string::npos)
      return std::vector<std::string>{s};
    while(k1!=std::string::npos){
        auto item = s.substr(k0,k1-k0);
        if(!item.empty())
          result.push_back(item);
        k0 = k1+delim.size();
        k1 = s.find(delim,k0);
    }
    
    auto item = s.substr(k0);
    if(!item.empty())
      result.push_back(item);
      
    return result;
}
//------------------------------------------------------------------------------------------------------
static inline std::string string_remove(const std::string& s, const std::string& chars_to_remove){
    auto str = s;
    for(const auto c: chars_to_remove){
        str.erase(std::remove(str.begin(), str.end(), c), str.end());
    }
    return str;
}
//------------------------------------------------------------------------------------------------------
static int GenerateRandom(const int low,const int high){
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> dist(low, high);
    return dist(g);
}
//------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]){
    std::ifstream in("pg.txt");
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string content(buffer.str());

    Mangrove::Mangrove m;
    
    auto sentences = string_split(content,"\n.;-()");
    for(const auto& s: sentences){
        std::stringstream ss(s); // Insert the string into a stream
        std::vector<std::string> tokens; // Create vector to hold our words
        std::string buf;
        while (ss >> buf)
            tokens.push_back(string_trim(string_remove(buf,".=;/{}*&^%$£!,()[]?")));

        
        for(const auto& w : tokens){
            m.Add(w);
        }
    }

    m.ComputeProbabilities();

    auto reader_t = std::thread([&](){
        while(true){
            std::string n;
            std::cin >> n;

            auto search_token = std::string(n);
            auto candidates = m.GetNext(search_token);
            while(candidates.size()>0){
                int i = GenerateRandom(0,candidates.size()-1);
                search_token = candidates.at(i).second;
                std::cout << search_token << " ";
                candidates = m.GetNext(search_token);
            }
            std::cout << "---\n";

            // for(const auto& t: m.GetNext(search_token)){
            //     std::cout << search_token << "->" << t.second << " " << t.first << std::endl;
            // }
        }
    });

    reader_t.join();

}
