#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "./json11/json11.hpp"

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
            for(const auto& ass : n->associations){
                results.push_back({ass.probability, ass.next_node->word});
                // results.push_back({ass.count, ass.next_node->word});
            }
        }
        return results;
    }
};

}
int main(int argc, char* argv[]){
    // std::ifstream in("pg.txt");
    // std::stringstream buffer;
    // buffer << in.rdbuf();
    // std::string contents(buffer.str());
    auto corpus = std::string("the cat is red the cat is green the cat is blue the dog is brown");
    
    std::stringstream ss(corpus); // Insert the string into a stream
    std::vector<std::string> tokens; // Create vector to hold our words
    std::string buf;
    while (ss >> buf)
        tokens.push_back(buf);    

    Mangrove::Mangrove m;
    for(const auto& w : tokens){
        m.Add(w);
    }
    m.ComputeProbabilities();

    std::cout << "\n\n\n\n";
    auto search_token = std::string("the");
    for(const auto& t: m.GetNext(search_token)){
        std::cout << search_token << "->" << t.second << " " << t.first << std::endl;
    }
}
