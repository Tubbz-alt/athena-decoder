#include "decoder/transformer-argmax-decoder.h"
#include <algorithm>
#include <vector>
namespace athena {

TransformerArgmaxDecoder::TransformerArgmaxDecoder(int max_seq_len, int sos,int eos){
    this->max_seq_len = max_seq_len;
    this->sos = sos;
    this->eos = eos;
    this->labels.clear();
}
void TransformerArgmaxDecoder::Decode(TransformerDecodable* decodable){

    std::vector<std::vector<float> > batch_log_scores;
    std::vector<std::vector<int> > batch_history_labels;
    std::shared_ptr<inference::PackedStates> ps(NULL); 
    this->labels.clear();
    this->labels.push_back(sos);
    batch_history_labels.push_back(this->labels);

    for(int step=0;step<max_seq_len;step++){
        decodable->InferenceOneStep(batch_history_labels, ps, batch_log_scores, ps);
        std::vector<float> log_scores = batch_log_scores[0];
        std::vector<float>::iterator biggest = std::max_element(log_scores.begin(), log_scores.end());
        int biggest_idx = std::distance(std::begin(log_scores), biggest);
        this->labels.push_back(biggest_idx);
        batch_history_labels.clear();
        batch_history_labels.push_back(this->labels);
        if(biggest_idx==eos){
            break;
        }
    }

} 

void TransformerArgmaxDecoder::GetBestPath(std::vector<int>& trans){
    trans.clear();
    trans.assign(this->labels.begin()+1, this->labels.end()-1);
}

} // end namespace athena
