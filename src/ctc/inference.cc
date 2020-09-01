#include "type.h"
#include "buffer.h"
#include "ctc.h"
#include "inference.h"
#include <string>
#include <cstring>
#include "operator.h"

namespace inference
{

struct ModelHandle{
    TFModel* ctc_model;
    ~ModelHandle(){
        if (NULL!=ctc_model) { delete ctc_model; ctc_model=NULL; }
    }
};

INFStatus LoadModel(const char *conf, void* &Model_Handel) {
    string entry_conf(conf);
    int pos = entry_conf.find_last_of('/');
    string conf_dir = entry_conf.substr(0, pos+1);
    char config[1024];
    std::string path;
    int num_threads;
    FILE *fIconf = fopen(conf, "r" );
    if(NULL == fIconf) {
        std::cerr<<"Fail to load handle conf"<<std::endl;
        return STATUS_ERROR;
    }else {
        char conf_key[1024], conf_value[1024];
        while(-1 != fscanf( fIconf, "%s %s", conf_key, conf_value )){
            if(0 == strcmp("",conf_key)){
                continue;
            }
            std::string tmp_dir(conf_dir);
            if(0 == strcmp("CTC_CONF",conf_key)){
                strcpy(config, conf_value);
            }else if(0 == strcmp("CTC_MODEL", conf_key)){
                path.assign(conf_value);
            }else if(0 == strcmp("NUM_THREADS", conf_key)){
                num_threads = std::atoi(conf_value);
            }
        }
    }
    fclose(fIconf);

    ModelHandle *model_handle = new ModelHandle;

    // init atten model
    model_handle->ctc_model = init_model(path, num_threads);
    strcpy(model_handle->ctc_model->config, config);

    Model_Handel = (void*)model_handle;

    return STATUS_OK;
}

INFStatus CreateHandle(void* Model_Handel, void* &Inf_Handel) {
    ModelHandle *model_handle = (ModelHandle *)Model_Handel;
    InfHandle *inf_handle = new InfHandle;

    // Ctc
    Ctc* ctc = new Ctc();
    if (NULL == ctc){
        std::cerr << "Fail to new ctc object!" << std::endl;
        return STATUS_ERROR;
    }

    inf_handle->handle_ = ctc;
    inf_handle->handle_->tfmodel = model_handle->ctc_model;

    int ret;
    ret = ctc->init(model_handle->ctc_model->config);

    if (-1 == ret){
        std::cerr << "Fail to init ctc handle!" << std::endl;
        return STATUS_ERROR;
    }

    Inf_Handel = (void*)inf_handle;
    
    return STATUS_OK;
}

INFStatus GetEncoderOutput(void* Inf_Handle, Input* in, encoder_output* enc_output){
    InfHandle *inf_handle = (InfHandle *)Inf_Handle;
    Ctc* ctc_ = (Ctc*)inf_handle->handle_;
    ctc_->push_states(in);
    int ret = ctc_->model_run(ctc_->tfmodel);
    if (-1 == ret ){
        std::cerr << "Fail to model run!" << std::endl;
        return STATUS_ERROR;
    }
    ret = 0;
    ret = ctc_->get_new_states(enc_output);
    if (-1 == ret){
        std::cerr << "Fail to get new states!" << std::endl;
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

INFStatus FreeHandle(void* Inf_Handel) {
    InfHandle *handle_ = (InfHandle *)Inf_Handel;
    if(NULL!=handle_){
        delete handle_;
        handle_=NULL;
    } 
    return STATUS_OK;
}

INFStatus FreeModel(void* Model_Handel) {
   ModelHandle *model_handle = (ModelHandle *)Model_Handel; 
   if (NULL != model_handle){
       delete model_handle;
       model_handle = NULL;
   } 
   return STATUS_OK;
}

}